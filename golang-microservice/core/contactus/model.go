package contactus

import (
	"github.com/google/uuid"
	"gorm.io/gorm"
	"gorm.io/datatypes"
	"time"
)

type ContactUs struct {
	ID        uuid.UUID      `gorm:"type:uuid;primaryKey" json:"id"`
	Site      string         `gorm:"index" json:"site"`
	Type      string         `gorm:"index" json:"type"`
	Message   string         `json:"message"`
	Extras    datatypes.JSON `json:"extras"`
	CreatedAt time.Time      `gorm:"autoCreateTime" json:"created_at"`
}

func (c *ContactUs) BeforeCreate(tx *gorm.DB) (err error) {
	c.ID = uuid.New()
	return
} 