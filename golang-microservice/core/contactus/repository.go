package contactus

import (
	"context"
	"gorm.io/gorm"
)

type ContactUsRepository interface {
	Create(ctx context.Context, contact *ContactUs) error
	FuzzySearch(ctx context.Context, query map[string]interface{}, skip, limit int) ([]ContactUs, error)
	Count(ctx context.Context, query map[string]interface{}) (int64, error)
}

type contactUsRepository struct {
	db *gorm.DB
}

func NewContactUsRepository(db *gorm.DB) ContactUsRepository {
	return &contactUsRepository{db: db}
}

func (r *contactUsRepository) Create(ctx context.Context, contact *ContactUs) error {
	return r.db.WithContext(ctx).Create(contact).Error
}

func (r *contactUsRepository) FuzzySearch(ctx context.Context, query map[string]interface{}, skip, limit int) ([]ContactUs, error) {
	var results []ContactUs
	db := r.db.WithContext(ctx).Model(&ContactUs{})
	for k, v := range query {
		db = db.Where(k+" LIKE ?", "%"+v.(string)+"%")
	}
	db = db.Order("created_at DESC").Offset(skip).Limit(limit)
	if err := db.Find(&results).Error; err != nil {
		return nil, err
	}
	return results, nil
}

func (r *contactUsRepository) Count(ctx context.Context, query map[string]interface{}) (int64, error) {
	var count int64
	db := r.db.WithContext(ctx).Model(&ContactUs{})
	for k, v := range query {
		db = db.Where(k+" LIKE ?", "%"+v.(string)+"%")
	}
	if err := db.Count(&count).Error; err != nil {
		return 0, err
	}
	return count, nil
} 