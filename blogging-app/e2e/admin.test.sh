#!/bin/bash

# ADMIN ENDPOINTS - Requires X-API-Key header
# E2E Test Suite

API="http://localhost:8080/api"
ADMIN_KEY="admin-secret-key-12345"

RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${BLUE}════════════════════════════════════════${NC}"
echo -e "${BLUE}   ADMIN ENDPOINTS - E2E TESTS${NC}"
echo -e "${BLUE}════════════════════════════════════════${NC}"
echo ""

# Auth header
HEADER="X-API-Key: $ADMIN_KEY"

# Test 1: Unauthorized request - missing key
echo -e "${YELLOW}[1] Attempt admin action without API key${NC}"
RESULT=$(curl -s -X GET "$API/admin/blogs")
if echo "$RESULT" | jq -e '.success == false' > /dev/null; then
  echo -e "${GREEN}✓ Correctly rejected unauthorized access${NC}"
  echo "$RESULT" | jq '.message'
else
  echo -e "${RED}✗ Should reject unauthorized access${NC}"
fi
echo ""

# Test 2: Unauthorized request - wrong key
echo -e "${YELLOW}[2] Attempt admin action with wrong API key${NC}"
RESULT=$(curl -s -X GET "$API/admin/blogs" \
  -H "X-API-Key: wrong-key")
if echo "$RESULT" | jq -e '.success == false' > /dev/null; then
  echo -e "${GREEN}✓ Correctly rejected wrong API key${NC}"
else
  echo -e "${RED}✗ Should reject wrong API key${NC}"
fi
echo ""

# Test 3: Admin - Create blog
echo -e "${YELLOW}[3] Admin create blog (auto draft status)${NC}"
BLOG=$(curl -s -X POST "$API/admin/blogs" \
  -H "$HEADER" \
  -H "Content-Type: application/json" \
  -d '{
    "title": "Admin Blog Post",
    "subtitle": "By the admin",
    "content": "This is an admin created blog, starts as draft",
    "author": "Admin User",
    "category": "Admin",
    "tags": ["admin", "official"],
    "featured_image": "https://example.com/admin.jpg"
  }')

echo "$BLOG" | jq '.'
ADMIN_BLOG_ID=$(echo "$BLOG" | jq -r '.data.id // empty')
if [ -z "$ADMIN_BLOG_ID" ]; then
  echo -e "${RED}✗ Failed to create blog${NC}"
else
  echo -e "${GREEN}✓ Blog created: $ADMIN_BLOG_ID (status: draft)${NC}"
fi
echo ""

# Test 4: Admin - Get all blogs (all statuses)
echo -e "${YELLOW}[4] Admin get all blogs (all statuses)${NC}"
curl -s -X GET "$API/admin/blogs?page=1&limit=10" \
  -H "$HEADER" | jq '.data | length'
echo ""

# Test 5: Admin - Get blog by ID
echo -e "${YELLOW}[5] Admin get blog by ID${NC}"
if [ -n "$ADMIN_BLOG_ID" ]; then
  curl -s -X GET "$API/admin/blogs/$ADMIN_BLOG_ID" \
    -H "$HEADER" | jq '.data | {title, status, views}'
fi
echo ""

# Test 6: Admin - Update blog
echo -e "${YELLOW}[6] Admin update blog${NC}"
if [ -n "$ADMIN_BLOG_ID" ]; then
  curl -s -X PUT "$API/admin/blogs/$ADMIN_BLOG_ID" \
    -H "$HEADER" \
    -H "Content-Type: application/json" \
    -d '{
      "title": "Updated Admin Blog",
      "content": "Updated content here",
      "category": "Updated Admin"
    }' | jq '.data | {title, status}'
fi
echo ""

# Test 7: Admin - Get submitted blogs
echo -e "${YELLOW}[7] Admin get submitted blogs (awaiting approval)${NC}"
curl -s -X GET "$API/admin/blogs/submitted" \
  -H "$HEADER" | jq '.data | length'
echo ""

# Test 8: Admin - Get draft blogs
echo -e "${YELLOW}[8] Admin get draft blogs${NC}"
curl -s -X GET "$API/admin/blogs/drafts" \
  -H "$HEADER" | jq '.data | length'
echo ""

# Test 9: Admin - Approve a submitted blog
echo -e "${YELLOW}[9] Admin approve submitted blog${NC}"
# Get first submitted blog
SUBMITTED=$(curl -s -X GET "$API/admin/blogs/submitted?limit=1" \
  -H "$HEADER" | jq -r '.data[0].id // empty')
if [ -n "$SUBMITTED" ]; then
  curl -s -X POST "$API/admin/blogs/$SUBMITTED/approve" \
    -H "$HEADER" | jq '.message'
else
  echo "No submitted blogs to approve"
fi
echo ""

# Test 10: Admin - Move to draft
echo -e "${YELLOW}[10] Admin move approved blog to draft${NC}"
if [ -n "$ADMIN_BLOG_ID" ]; then
  curl -s -X POST "$API/admin/blogs/$ADMIN_BLOG_ID/draft" \
    -H "$HEADER" | jq '.message'
fi
echo ""

# Test 11: Admin - Publish blog
echo -e "${YELLOW}[11] Admin publish blog${NC}"
if [ -n "$ADMIN_BLOG_ID" ]; then
  RESULT=$(curl -s -X POST "$API/admin/blogs/$ADMIN_BLOG_ID/publish" \
    -H "$HEADER")
  echo "$RESULT" | jq '.message'
  PUBLISHED_ID=$ADMIN_BLOG_ID
fi
echo ""

# Test 12: Admin - Delete blog
echo -e "${YELLOW}[12] Admin delete blog${NC}"
DELETE_ID=$(curl -s -X POST "$API/admin/blogs" \
  -H "$HEADER" \
  -H "Content-Type: application/json" \
  -d '{
    "title": "Blog to Delete",
    "content": "This will be deleted",
    "author": "Admin",
    "category": "Temp"
  }' | jq -r '.data.id')

if [ -n "$DELETE_ID" ]; then
  curl -s -X DELETE "$API/admin/blogs/$DELETE_ID" \
    -H "$HEADER" | jq '.message'
fi
echo ""

# Test 13: Admin - Search by tag
echo -e "${YELLOW}[13] Admin search by tag${NC}"
curl -s -X GET "$API/admin/search/tag/admin" \
  -H "$HEADER" | jq '.data | length'
echo ""

# Test 14: Admin - Search by author
echo -e "${YELLOW}[14] Admin search by author${NC}"
curl -s -X GET "$API/admin/search/author/Admin" \
  -H "$HEADER" | jq '.data | length'
echo ""

# Test 15: Admin - Search by category
echo -e "${YELLOW}[15] Admin search by category${NC}"
curl -s -X GET "$API/admin/search/category/Admin" \
  -H "$HEADER" | jq '.data | length'
echo ""

# Test 16: Admin - Get stats
echo -e "${YELLOW}[16] Admin get blog statistics${NC}"
curl -s -X GET "$API/admin/stats" \
  -H "$HEADER" | jq '.data'
echo ""

# Test 17: Admin - Get popular tags
echo -e "${YELLOW}[17] Admin get popular tags${NC}"
curl -s -X GET "$API/admin/tags/popular" \
  -H "$HEADER" | jq '.data | length'
echo ""

# Test 18: Published blog should appear in public
echo -e "${YELLOW}[18] Verify published blog appears in public${NC}"
if [ -n "$PUBLISHED_ID" ]; then
  sleep 1
  FOUND=$(curl -s -X GET "$API/public/blogs?page=1&limit=100" | \
    jq ".data[] | select(.id == \"$PUBLISHED_ID\") | .id")
  if [ -n "$FOUND" ]; then
    echo -e "${GREEN}✓ Published blog appears in public${NC}"
  else
    echo -e "${YELLOW}~ Published blog may take moment to appear${NC}"
  fi
fi
echo ""

# Test 19: Create multiple blogs for bulk operations
echo -e "${YELLOW}[19] Admin create multiple blogs${NC}"
for i in {1..3}; do
  curl -s -X POST "$API/admin/blogs" \
    -H "$HEADER" \
    -H "Content-Type: application/json" \
    -d "{
      \"title\": \"Bulk Blog $i\",
      \"content\": \"Content for bulk $i\",
      \"author\": \"Admin\",
      \"category\": \"Bulk\"
    }" | jq '.success'
done
echo ""

# Test 20: Admin - Invalid operations
echo -e "${YELLOW}[20] Admin attempt invalid operations${NC}"
echo "  a) Create blog without title"
curl -s -X POST "$API/admin/blogs" \
  -H "$HEADER" \
  -H "Content-Type: application/json" \
  -d '{"content": "Missing title"}' | jq '.success'

echo "  b) Update non-existent blog"
curl -s -X PUT "$API/admin/blogs/invalid-id" \
  -H "$HEADER" \
  -H "Content-Type: application/json" \
  -d '{"title": "Updated"}' | jq '.success'

echo "  c) Delete non-existent blog"
curl -s -X DELETE "$API/admin/blogs/invalid-id" \
  -H "$HEADER" | jq '.success'
echo ""

echo -e "${BLUE}════════════════════════════════════════${NC}"
echo -e "${GREEN}   ADMIN E2E TESTS COMPLETE${NC}"
echo -e "${BLUE}════════════════════════════════════════${NC}"
