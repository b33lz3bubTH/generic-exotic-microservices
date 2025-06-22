package main

import (
	"github.com/gin-gonic/gin"
	"github.com/sirupsen/logrus"
	"github.com/yourorg/contactus-microservice/internal/db"
	"github.com/yourorg/contactus-microservice/core/contactus"
	"github.com/yourorg/contactus-microservice/core/products"
	"github.com/gin-contrib/cors"
	"github.com/unrolled/secure"
)

func main() {
	// Initialize logger
	logrus.SetFormatter(&logrus.TextFormatter{FullTimestamp: true})
	logrus.Info("Starting ContactUs microservice...")

	// Initialize DB
	db.InitDB()

	// Dependency injection
	repo := contactus.NewContactUsRepository(db.DB)
	svc := contactus.NewContactUsService(repo)
	ctrl := contactus.NewContactUsController(svc)

	productRepo := products.NewProductRepository(db.DB)
	productSvc := products.NewProductService(productRepo)
	productCtrl := products.NewProductController(productSvc)

	// Setup Gin
	r := gin.Default()

	// CORS middleware (allow all origins, customize as needed)
	r.Use(cors.Default())

	// Security headers (helmet-like)
	secureMiddleware := secure.New(secure.Options{
		FrameDeny:             true,
		ContentTypeNosniff:    true,
		BrowserXssFilter:      true,
		ContentSecurityPolicy: "default-src 'self'",
		ReferrerPolicy:        "strict-origin-when-cross-origin",
	})
	r.Use(func(c *gin.Context) {
		err := secureMiddleware.Process(c.Writer, c.Request)
		if err != nil {
			c.AbortWithStatus(400)
			return
		}
		c.Next()
	})

	// Rate limiting middleware for contactus
	r.Use(contactus.RateLimitMiddleware(ctrl.Name()))

	// Rate limiting middleware for products
	r.Use(contactus.RateLimitMiddleware(productCtrl.Name()))

	ctrl.RegisterRoutes(r)
	productCtrl.RegisterRoutes(r)

	// Start server
	logrus.Info("Server running on :8080")
	r.Run(":8080")
}
