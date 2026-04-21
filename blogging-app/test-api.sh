#!/bin/bash

# 🧪 Blogging Service - Curl Testing Script
# This file contains all the curl commands to test the blogging service API

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Base URL
API_BASE="http://localhost:8080/api"

echo -e "${BLUE}╔════════════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║   🌐 BLOGGING SERVICE API - TESTING SCRIPT                 ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════════════════════════╝${NC}"
echo ""

# ============================================================
# 1. CREATE BLOGS
# ============================================================

echo -e "${YELLOW}📝 Creating sample blogs...${NC}"
echo ""

echo -e "${GREEN}[1/3] Creating 'Understanding Drogon Framework'${NC}"
BLOG1=$(curl -s -X POST $API_BASE/blogs \
  -H "Content-Type: application/json" \
  -d '{
    "title": "Understanding Drogon Framework",
    "subtitle": "A modern C++ web framework",
    "content": "Drogon is a lightweight and fast C++ web application framework. It provides excellent performance and ease of use. This blog explores its key features and how to get started.",
    "author": "Alice Smith",
    "category": "Technology",
    "featured_image": "https://example.com/drogon.jpg",
    "tags": ["C++", "Drogon", "Web Framework", "Performance"]
  }')

BLOG1_ID=$(echo $BLOG1 | grep -o '"id":"[^"]*"' | cut -d'"' -f4)
echo -e "Created blog with ID: ${GREEN}$BLOG1_ID${NC}"
echo ""

echo -e "${GREEN}[2/3] Creating 'MongoDB Performance Optimization'${NC}"
BLOG2=$(curl -s -X POST $API_BASE/blogs \
  -H "Content-Type: application/json" \
  -d '{
    "title": "MongoDB Performance Optimization",
    "subtitle": "Techniques for faster queries",
    "content": "Learn advanced MongoDB optimization techniques including indexing, query optimization, and connection pooling. Improve your database performance significantly.",
    "author": "Bob Johnson",
    "category": "Database",
    "featured_image": "https://example.com/mongo.jpg",
    "tags": ["MongoDB", "Performance", "Database", "DevOps"]
  }')

BLOG2_ID=$(echo $BLOG2 | grep -o '"id":"[^"]*"' | cut -d'"' -f4)
echo -e "Created blog with ID: ${GREEN}$BLOG2_ID${NC}"
echo ""

echo -e "${GREEN}[3/3] Creating 'Docker Best Practices'${NC}"
BLOG3=$(curl -s -X POST $API_BASE/blogs \
  -H "Content-Type: application/json" \
  -d '{
    "title": "Docker Best Practices",
    "subtitle": "Container deployment guide",
    "content": "Master Docker containerization with these industry best practices. From image optimization to security, learn everything you need to deploy containers effectively.",
    "author": "Carol White",
    "category": "DevOps",
    "featured_image": "https://example.com/docker.jpg",
    "tags": ["Docker", "DevOps", "Containers", "Deployment"]
  }')

BLOG3_ID=$(echo $BLOG3 | grep -o '"id":"[^"]*"' | cut -d'"' -f4)
echo -e "Created blog with ID: ${GREEN}$BLOG3_ID${NC}"
echo ""

# ============================================================
# 2. GET ALL BLOGS
# ============================================================

echo -e "${YELLOW}📚 Getting all blogs${NC}"
echo ""
echo -e "${GREEN}GET /blogs${NC}"
curl -s -X GET "$API_BASE/blogs?page=1&limit=10" | jq '.'
echo ""

# ============================================================
# 3. GET SPECIFIC BLOG
# ============================================================

echo -e "${YELLOW}📖 Getting specific blog${NC}"
echo ""
echo -e "${GREEN}GET /blogs/$BLOG1_ID${NC}"
curl -s -X GET "$API_BASE/blogs/$BLOG1_ID" | jq '.'
echo ""

# ============================================================
# 4. GET BY SLUG
# ============================================================

echo -e "${YELLOW}🔗 Getting blog by slug${NC}"
echo ""
echo -e "${GREEN}GET /blogs/slug/understanding-drogon-framework${NC}"
curl -s -X GET "$API_BASE/blogs/slug/understanding-drogon-framework" | jq '.'
echo ""

# ============================================================
# 5. UPDATE BLOG
# ============================================================

echo -e "${YELLOW}✏️  Updating blog${NC}"
echo ""
echo -e "${GREEN}PUT /blogs/$BLOG1_ID${NC}"
curl -s -X PUT "$API_BASE/blogs/$BLOG1_ID" \
  -H "Content-Type: application/json" \
  -d '{
    "title": "Understanding Drogon Framework - Advanced Guide",
    "subtitle": "A modern C++ web framework - In Depth",
    "content": "Updated content with advanced topics...",
    "category": "Advanced"
  }' | jq '.'
echo ""

# ============================================================
# 6. PUBLISH BLOG
# ============================================================

echo -e "${YELLOW}📢 Publishing blog${NC}"
echo ""
echo -e "${GREEN}POST /blogs/$BLOG1_ID/publish${NC}"
curl -s -X POST "$API_BASE/blogs/$BLOG1_ID/publish" | jq '.'
echo ""

echo -e "${GREEN}POST /blogs/$BLOG2_ID/publish${NC}"
curl -s -X POST "$API_BASE/blogs/$BLOG2_ID/publish" | jq '.'
echo ""

echo -e "${GREEN}POST /blogs/$BLOG3_ID/publish${NC}"
curl -s -X POST "$API_BASE/blogs/$BLOG3_ID/publish" | jq '.'
echo ""

# ============================================================
# 7. SEARCH OPERATIONS
# ============================================================

echo -e "${YELLOW}🔍 Search Operations${NC}"
echo ""

echo -e "${GREEN}Search by keyword 'Drogon'${NC}"
curl -s -X GET "$API_BASE/search?q=Drogon" | jq '.'
echo ""

echo -e "${GREEN}Search by tag 'C++'${NC}"
curl -s -X GET "$API_BASE/search/tag/C++" | jq '.'
echo ""

echo -e "${GREEN}Search by author 'Alice Smith'${NC}"
curl -s -X GET "$API_BASE/search/author/Alice%20Smith" | jq '.'
echo ""

echo -e "${GREEN}Search by category 'Technology'${NC}"
curl -s -X GET "$API_BASE/search/category/Technology" | jq '.'
echo ""

echo -e "${GREEN}Advanced search with filters${NC}"
curl -s -X GET "$API_BASE/search?q=framework&author=Alice%20Smith&category=Technology" | jq '.'
echo ""

# ============================================================
# 8. STATISTICS
# ============================================================

echo -e "${YELLOW}📊 Statistics${NC}"
echo ""

echo -e "${GREEN}GET /stats${NC}"
curl -s -X GET "$API_BASE/stats" | jq '.'
echo ""

echo -e "${GREEN}GET /tags/popular${NC}"
curl -s -X GET "$API_BASE/tags/popular" | jq '.'
echo ""

# ============================================================
# 9. DELETE BLOG
# ============================================================

echo -e "${YELLOW}🗑️  Deleting blog (optional - comment out to keep data)${NC}"
echo ""
echo -e "${GREEN}DELETE /blogs/$BLOG3_ID${NC}"
curl -s -X DELETE "$API_BASE/blogs/$BLOG3_ID" | jq '.'
echo ""

# ============================================================
# 10. FINAL STATUS
# ============================================================

echo -e "${YELLOW}📊 Final blog count${NC}"
echo ""
echo -e "${GREEN}GET /stats${NC}"
curl -s -X GET "$API_BASE/stats" | jq '.'
echo ""

echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}✅ Testing completed!${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
echo ""

# ============================================================
# INDIVIDUAL CURL COMMANDS FOR REFERENCE
# ============================================================

echo -e "${YELLOW}📋 Individual curl commands for reference:${NC}"
echo ""

cat << 'EOF'
# CREATE BLOG
curl -X POST http://localhost:8080/api/blogs \
  -H "Content-Type: application/json" \
  -d '{
    "title": "My Blog Title",
    "subtitle": "My subtitle",
    "content": "Blog content here",
    "author": "John Doe",
    "category": "Technology",
    "featured_image": "https://example.com/image.jpg",
    "tags": ["tag1", "tag2"]
  }'

# GET ALL BLOGS (paginated)
curl "http://localhost:8080/api/blogs?page=1&limit=10"

# GET BLOG BY ID
curl "http://localhost:8080/api/blogs/BLOG_ID"

# GET BLOG BY SLUG
curl "http://localhost:8080/api/blogs/slug/my-blog-title"

# UPDATE BLOG
curl -X PUT "http://localhost:8080/api/blogs/BLOG_ID" \
  -H "Content-Type: application/json" \
  -d '{
    "title": "Updated Title",
    "content": "Updated content"
  }'

# PUBLISH BLOG
curl -X POST "http://localhost:8080/api/blogs/BLOG_ID/publish"

# UNPUBLISH BLOG
curl -X POST "http://localhost:8080/api/blogs/BLOG_ID/unpublish"

# SEARCH BY QUERY
curl "http://localhost:8080/api/search?q=drogon"

# SEARCH BY TAG
curl "http://localhost:8080/api/search/tag/C++"

# SEARCH BY AUTHOR
curl "http://localhost:8080/api/search/author/John%20Doe"

# SEARCH BY CATEGORY
curl "http://localhost:8080/api/search/category/Technology"

# GET STATISTICS
curl "http://localhost:8080/api/stats"

# GET POPULAR TAGS
curl "http://localhost:8080/api/tags/popular"

# DELETE BLOG
curl -X DELETE "http://localhost:8080/api/blogs/BLOG_ID"
EOF

echo ""
