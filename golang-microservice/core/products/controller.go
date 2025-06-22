package products

import (
	"context"
	"net/http"
	"strconv"
	"github.com/gin-gonic/gin"
	"github.com/go-playground/validator/v10"
)

type ProductController struct {
	service   ProductService
	validator *validator.Validate
}

func NewProductController(s ProductService) *ProductController {
	return &ProductController{
		service:   s,
		validator: validator.New(),
	}
}

type ProductRequest struct {
	ID          string              `json:"id"`
	Name        string              `json:"name" validate:"required"`
	Category    string              `json:"category"`
	Description string              `json:"description"`
	Price       int                 `json:"price"`
	Featured    bool                `json:"featured"`
	Images      []string            `json:"images"`
	Variants    []ProductVariantReq `json:"variants"`
}

type ProductVariantReq struct {
	ID      string            `json:"id"`
	SKU     string            `json:"sku"`
	Color   string            `json:"color"`
	Size    string            `json:"size"`
	Image   string            `json:"image_url"`
	Price   int               `json:"price"`
	InStock bool              `json:"in_stock"`
}

func (c *ProductController) RegisterRoutes(r *gin.Engine) {
	group := r.Group("/products")
	group.POST("", c.CreateProduct)
	group.PUT(":id", c.UpdateProduct)
	group.DELETE(":id", c.DeleteProduct)
	group.GET(":id", c.GetProduct)
	group.GET("", c.ListProducts)
}

func (c *ProductController) CreateProduct(ctx *gin.Context) {
	var req ProductRequest
	if err := ctx.ShouldBindJSON(&req); err != nil {
		ctx.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}
	if err := c.validator.Struct(req); err != nil {
		ctx.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}
	product := &Product{
		ID:          req.ID,
		Name:        req.Name,
		Category:    req.Category,
		Description: req.Description,
		Price:       req.Price,
		Featured:    req.Featured,
	}
	for _, img := range req.Images {
		product.Images = append(product.Images, ProductImage{ImageURL: img})
	}
	for _, v := range req.Variants {
		product.Variants = append(product.Variants, ProductVariant{
			ID:      v.ID,
			SKU:     v.SKU,
			Color:   v.Color,
			Size:    v.Size,
			ImageURL: v.Image,
			Price:   v.Price,
			InStock: v.InStock,
		})
	}
	id, err := c.service.CreateProduct(context.Background(), product)
	if err != nil {
		ctx.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}
	ctx.JSON(http.StatusOK, gin.H{"id": id})
}

func (c *ProductController) UpdateProduct(ctx *gin.Context) {
	id := ctx.Param("id")
	var req ProductRequest
	if err := ctx.ShouldBindJSON(&req); err != nil {
		ctx.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}
	if err := c.validator.Struct(req); err != nil {
		ctx.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}
	product := &Product{
		ID:          id,
		Name:        req.Name,
		Category:    req.Category,
		Description: req.Description,
		Price:       req.Price,
		Featured:    req.Featured,
	}
	for _, img := range req.Images {
		product.Images = append(product.Images, ProductImage{ImageURL: img, ProductID: id})
	}
	for _, v := range req.Variants {
		product.Variants = append(product.Variants, ProductVariant{
			ID:      v.ID,
			SKU:     v.SKU,
			Color:   v.Color,
			Size:    v.Size,
			ImageURL: v.Image,
			Price:   v.Price,
			InStock: v.InStock,
			ProductID: id,
		})
	}
	if err := c.service.UpdateProduct(context.Background(), product); err != nil {
		ctx.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}
	ctx.JSON(http.StatusOK, gin.H{"id": id})
}

func (c *ProductController) DeleteProduct(ctx *gin.Context) {
	id := ctx.Param("id")
	if err := c.service.DeleteProduct(context.Background(), id); err != nil {
		ctx.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}
	ctx.JSON(http.StatusOK, gin.H{"deleted": true})
}

func (c *ProductController) GetProduct(ctx *gin.Context) {
	id := ctx.Param("id")
	product, err := c.service.GetProductByID(context.Background(), id)
	if err != nil {
		ctx.JSON(http.StatusNotFound, gin.H{"error": "product not found"})
		return
	}
	ctx.JSON(http.StatusOK, product)
}

func (c *ProductController) ListProducts(ctx *gin.Context) {
	skip, _ := strconv.Atoi(ctx.DefaultQuery("skip", "0"))
	take, _ := strconv.Atoi(ctx.DefaultQuery("take", "10"))
	products, total, err := c.service.PaginatedListProducts(context.Background(), skip, take)
	if err != nil {
		ctx.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}
	ctx.JSON(http.StatusOK, gin.H{"total": total, "data": products})
}

func (c *ProductController) Name() string {
	return "products"
} 