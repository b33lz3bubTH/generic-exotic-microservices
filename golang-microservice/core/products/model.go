package products

import (
	"github.com/google/uuid"
	"gorm.io/gorm"
	"time"
)

type Product struct {
	ID          string    `gorm:"primaryKey" json:"id"`
	Name        string    `json:"name"`
	Category    string    `json:"category"`
	Description string    `json:"description"`
	Price       int       `json:"price"` // price in smallest currency unit
	Featured    bool      `json:"featured"`
	CreatedAt   time.Time `gorm:"autoCreateTime" json:"created_at"`
	Images      []ProductImage   `gorm:"foreignKey:ProductID" json:"images"`
	Variants    []ProductVariant `gorm:"foreignKey:ProductID" json:"variants"`
}

type ProductImage struct {
	ID        uint   `gorm:"primaryKey" json:"id"`
	ProductID string `gorm:"index" json:"product_id"`
	ImageURL  string `json:"url"`
}

type ProductVariant struct {
	ID        string `gorm:"primaryKey" json:"id"`
	ProductID string `gorm:"index" json:"product_id"`
	SKU       string `json:"sku"`
	Color     string `json:"color"`
	Size      string `json:"size"`
	ImageURL  string `json:"url"`
	Price     int    `json:"price"`
	InStock   bool   `json:"in_stock"`
}

func (p *Product) BeforeCreate(tx *gorm.DB) (err error) {
	if p.ID == "" {
		p.ID = uuid.New().String()
	}
	return
}

func (v *ProductVariant) BeforeCreate(tx *gorm.DB) (err error) {
	if v.ID == "" {
		v.ID = uuid.New().String()
	}
	return
} 