package contactus

import (
	"context"
	"net/http"
	"strconv"
	"github.com/gin-gonic/gin"
	"github.com/go-playground/validator/v10"
	"encoding/json"
)

type ContactUsController struct {
	service   ContactUsService
	validator *validator.Validate
}

func NewContactUsController(s ContactUsService) *ContactUsController {
	return &ContactUsController{
		service:   s,
		validator: validator.New(),
	}
}

type ContactUsRequest struct {
	Site    string      `json:"site" validate:"required"`
	Type    string      `json:"type" validate:"required"`
	Message string      `json:"message" validate:"required"`
	Extras  interface{} `json:"extras"`
}

func (c *ContactUsController) RegisterRoutes(r *gin.Engine) {
	r.POST("/contactus", c.CreateContactUs)
	r.GET("/contactus", c.GetContactUs)
}

func (c *ContactUsController) CreateContactUs(ctx *gin.Context) {
	var req ContactUsRequest
	if err := ctx.ShouldBindJSON(&req); err != nil {
		ctx.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}
	if err := c.validator.Struct(req); err != nil {
		ctx.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}
	extraBytes, err := json.Marshal(req.Extras)
	if err != nil {
		ctx.JSON(http.StatusBadRequest, gin.H{"error": "invalid extras"})
		return
	}
	contact := &ContactUs{
		Site:    req.Site,
		Type:    req.Type,
		Message: req.Message,
		Extras:  extraBytes,
	}
	id, err := c.service.CreateContactUs(context.Background(), contact)
	if err != nil {
		ctx.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}
	ctx.JSON(http.StatusOK, gin.H{"id": id})
}

func (c *ContactUsController) GetContactUs(ctx *gin.Context) {
	query := make(map[string]interface{})
	if site := ctx.Query("site"); site != "" {
		query["site"] = site
	}
	if typ := ctx.Query("type"); typ != "" {
		query["type"] = typ
	}
	if msg := ctx.Query("message"); msg != "" {
		query["message"] = msg
	}
	// extras fuzzy search is not supported directly, but can be extended
	skip, _ := strconv.Atoi(ctx.DefaultQuery("skip", "0"))
	limit, _ := strconv.Atoi(ctx.DefaultQuery("limit", "10"))
	results, total, err := c.service.FuzzySearchContactUs(context.Background(), query, skip, limit)
	if err != nil {
		ctx.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}
	ctx.JSON(http.StatusOK, gin.H{"total": total, "data": results})
}

func (c *ContactUsController) Name() string {
	return "contactus"
} 