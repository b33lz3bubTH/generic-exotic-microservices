package products

import (
	"context"
	"gorm.io/gorm"
)

type ProductRepository interface {
	Create(ctx context.Context, product *Product) error
	Update(ctx context.Context, product *Product) error
	Delete(ctx context.Context, id string) error
	GetByID(ctx context.Context, id string) (*Product, error)
	PaginatedList(ctx context.Context, skip, take int) ([]Product, int64, error)
}

type productRepository struct {
	db *gorm.DB
}

func NewProductRepository(db *gorm.DB) ProductRepository {
	return &productRepository{db: db}
}

func (r *productRepository) Create(ctx context.Context, product *Product) error {
	return r.db.WithContext(ctx).Create(product).Error
}

func (r *productRepository) Update(ctx context.Context, product *Product) error {
	return r.db.WithContext(ctx).Session(&gorm.Session{FullSaveAssociations: true}).Updates(product).Error
}

func (r *productRepository) Delete(ctx context.Context, id string) error {
	return r.db.WithContext(ctx).Where("id = ?", id).Delete(&Product{}).Error
}

func (r *productRepository) GetByID(ctx context.Context, id string) (*Product, error) {
	var product Product
	err := r.db.WithContext(ctx).Preload("Images").Preload("Variants").First(&product, "id = ?", id).Error
	if err != nil {
		return nil, err
	}
	return &product, nil
}

func (r *productRepository) PaginatedList(ctx context.Context, skip, take int) ([]Product, int64, error) {
	var products []Product
	var count int64
	db := r.db.WithContext(ctx).Model(&Product{})
	db.Count(&count)
	err := db.Preload("Images").Preload("Variants").Offset(skip).Limit(take).Find(&products).Error
	if err != nil {
		return nil, 0, err
	}
	return products, count, nil
} 