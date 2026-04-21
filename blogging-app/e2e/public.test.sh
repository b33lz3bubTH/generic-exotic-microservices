#!/bin/bash

# PUBLIC ENDPOINTS - No authentication required
# E2E Test Suite

API="http://localhost:8080/api"

RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${BLUE}════════════════════════════════════════${NC}"
echo -e "${BLUE}   PUBLIC ENDPOINTS - E2E TESTS${NC}"
echo -e "${BLUE}════════════════════════════════════════${NC}"
echo ""

# Test 1: Submit blog publicly
echo -e "${YELLOW}[1] Submit blog publicly (submitted status)${NC}"
SUBMIT=$(curl -s -X POST "$API/public/submit" \
  -H "Content-Type: application/json" \
  -d '{
    "title": "Public Blog Submission",
    "subtitle": "From public user",
    "content": "This is my submitted blog waiting for approval",
    "author": "John User",
    "category": "General",
    "tags": ["public", "submission"],
    "featured_image": "https://example.com/img.jpg"
  }')

echo "$SUBMIT" | jq '.'
BLOG_ID=$(echo "$SUBMIT" | jq -r '.data.id // empty')
if [ -z "$BLOG_ID" ]; then
  echo -e "${RED}✗ Failed to submit blog${NC}"
else
  echo -e "${GREEN}✓ Blog submitted with ID: $BLOG_ID${NC}"
fi
echo ""

# Test 2: Get published blogs
echo -e "${YELLOW}[2] Get published blogs (paginated)${NC}"
curl -s -X GET "$API/public/blogs?page=1&limit=10" | jq '.'
echo ""

# Test 3: Get published blogs with limit
echo -e "${YELLOW}[3] Get published blogs with limit=5${NC}"
curl -s -X GET "$API/public/blogs?page=1&limit=5" | jq '.data | length'
echo ""

# Test 4: Search published blogs by keyword
echo -e "${YELLOW}[4] Search published blogs by keyword${NC}"
curl -s -X GET "$API/public/search?q=blog" | jq '.'
echo ""

# Test 5: Search by tag
echo -e "${YELLOW}[5] Search published blogs by tag${NC}"
curl -s -X GET "$API/public/search/tag/tutorial" | jq '.data | length'
echo ""

# Test 6: Search by author
echo -e "${YELLOW}[6] Search published blogs by author${NC}"
curl -s -X GET "$API/public/search/author/admin" | jq '.data | length'
echo ""

# Test 7: Search by category
echo -e "${YELLOW}[7] Search published blogs by category${NC}"
curl -s -X GET "$API/public/search/category/Technology" | jq '.data | length'
echo ""

# Test 8: Get blog by slug
echo -e "${YELLOW}[8] Get blog by slug${NC}"
curl -s -X GET "$API/public/blogs/slug/how-to-blog" | jq '.data | {title, slug, status, views}'
echo ""

# Test 9: Get blog by slug - non-existent
echo -e "${YELLOW}[9] Get non-existent blog by slug${NC}"
RESULT=$(curl -s -X GET "$API/public/blogs/non-existent-slug")
if echo "$RESULT" | jq -e '.success == false' > /dev/null; then
  echo -e "${GREEN}✓ Correctly returned error${NC}"
  echo "$RESULT" | jq '.'
else
  echo -e "${RED}✗ Should return error for non-existent slug${NC}"
fi
echo ""

# Test 10: Multiple submissions
echo -e "${YELLOW}[10] Submit multiple blogs${NC}"
for i in {1..3}; do
  curl -s -X POST "$API/public/submit" \
    -H "Content-Type: application/json" \
    -d "{
      \"title\": \"Public Submission $i\",
      \"subtitle\": \"Submission number $i\",
      \"content\": \"Content for submission $i\",
      \"author\": \"User $i\",
      \"category\": \"Category $i\",
      \"tags\": [\"submission\", \"test$i\"]
    }" | jq '.success'
done
echo ""

echo -e "${BLUE}════════════════════════════════════════${NC}"
echo -e "${GREEN}   PUBLIC E2E TESTS COMPLETE${NC}"
echo -e "${BLUE}════════════════════════════════════════${NC}"
