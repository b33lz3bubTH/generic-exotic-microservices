package products

import (
	"context"
)

type ProductService interface {
	CreateProduct(ctx context.Context, product *Product) (string, error)
	UpdateProduct(ctx context.Context, product *Product) error
	DeleteProduct(ctx context.Context, id string) error
	GetProductByID(ctx context.Context, id string) (*Product, error)
	PaginatedListProducts(ctx context.Context, skip, take int) ([]Product, int64, error)
}

type productService struct {
	repo ProductRepository
}

func NewProductService(repo ProductRepository) ProductService {
	return &productService{repo: repo}
}

func (s *productService) CreateProduct(ctx context.Context, product *Product) (string, error) {
	if err := s.repo.Create(ctx, product); err != nil {
		return "", err
	}
	return product.ID, nil
}

func (s *productService) UpdateProduct(ctx context.Context, product *Product) error {
	return s.repo.Update(ctx, product)
}

func (s *productService) DeleteProduct(ctx context.Context, id string) error {
	return s.repo.Delete(ctx, id)
}

func (s *productService) GetProductByID(ctx context.Context, id string) (*Product, error) {
	return s.repo.GetByID(ctx, id)
}

func (s *productService) PaginatedListProducts(ctx context.Context, skip, take int) ([]Product, int64, error) {
	return s.repo.PaginatedList(ctx, skip, take)
} 