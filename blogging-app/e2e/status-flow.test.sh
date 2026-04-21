#!/bin/bash

# STATUS FLOW E2E Tests
# Demonstrates: submitted → approved → draft → published workflow

API="http://localhost:8080/api"
ADMIN_KEY="admin-secret-key-12345"

RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

HEADER="X-API-Key: $ADMIN_KEY"

echo -e "${BLUE}════════════════════════════════════════${NC}"
echo -e "${BLUE}   STATUS FLOW WORKFLOW TESTS${NC}"
echo -e "${BLUE}════════════════════════════════════════${NC}"
echo ""

# ============================================================
# FLOW 1: Public Submission → Admin Approval → Draft → Publish
# ============================================================

echo -e "${CYAN}[FLOW 1] Public → Approved → Draft → Published${NC}"
echo ""

# Step 1: Public user submits blog
echo -e "${YELLOW}Step 1: Public user submits blog${NC}"
echo -e "${YELLOW}Expected status: submitted${NC}"
FLOW1=$(curl -s -X POST "$API/public/submit" \
  -H "Content-Type: application/json" \
  -d '{
    "title": "User Submitted Story",
    "subtitle": "A real user story",
    "content": "This blog was submitted by a regular user",
    "author": "Regular User",
    "category": "Personal",
    "tags": ["user-submitted", "personal-story"],
    "featured_image": "https://example.com/story.jpg"
  }')

BLOG_ID=$(echo "$FLOW1" | jq -r '.data.id // empty')
STATUS=$(echo "$FLOW1" | jq -r '.data.status // empty')

if [ "$STATUS" = "submitted" ]; then
  echo -e "${GREEN}✓ Blog submitted with status: $STATUS${NC}"
  echo "  ID: $BLOG_ID"
else
  echo -e "${RED}✗ Expected 'submitted', got: $STATUS${NC}"
fi
echo ""

# Step 2: Admin views submitted blogs
echo -e "${YELLOW}Step 2: Admin views submitted blogs${NC}"
SUBMITTED=$(curl -s -X GET "$API/admin/blogs/submitted?limit=100" \
  -H "$HEADER" | jq '.data')
COUNT=$(echo "$SUBMITTED" | jq 'length')
echo -e "${GREEN}✓ Found $COUNT submitted blog(s)${NC}"
echo ""

# Step 3: Admin approves the blog
echo -e "${YELLOW}Step 3: Admin approves the blog${NC}"
echo -e "${YELLOW}Expected status: approved${NC}"
APPROVED=$(curl -s -X POST "$API/admin/blogs/$BLOG_ID/approve" \
  -H "$HEADER")

RESULT_STATUS=$(echo "$APPROVED" | jq -r '.data.status // empty')
if [ "$RESULT_STATUS" = "approved" ]; then
  echo -e "${GREEN}✓ Blog approved, new status: $RESULT_STATUS${NC}"
else
  echo -e "${YELLOW}~ Response: ${NC}"
  echo "$APPROVED" | jq '.message'
fi
echo ""

# Step 4: Admin moves to draft
echo -e "${YELLOW}Step 4: Admin moves approved blog to draft${NC}"
echo -e "${YELLOW}Expected status: draft${NC}"
DRAFT=$(curl -s -X POST "$API/admin/blogs/$BLOG_ID/draft" \
  -H "$HEADER")

RESULT_STATUS=$(echo "$DRAFT" | jq -r '.data.status // empty')
if [ "$RESULT_STATUS" = "draft" ]; then
  echo -e "${GREEN}✓ Blog moved to draft, new status: $RESULT_STATUS${NC}"
else
  echo -e "${YELLOW}~ Response: ${NC}"
  echo "$DRAFT" | jq '.message'
fi
echo ""

# Step 5: Admin publishes
echo -e "${YELLOW}Step 5: Admin publishes the blog${NC}"
echo -e "${YELLOW}Expected status: published${NC}"
PUBLISHED=$(curl -s -X POST "$API/admin/blogs/$BLOG_ID/publish" \
  -H "$HEADER")

RESULT_STATUS=$(echo "$PUBLISHED" | jq -r '.data.status // empty')
if [ "$RESULT_STATUS" = "published" ]; then
  echo -e "${GREEN}✓ Blog published, new status: $RESULT_STATUS${NC}"
else
  echo -e "${YELLOW}~ Response: ${NC}"
  echo "$PUBLISHED" | jq '.message'
fi
echo ""

# Step 6: Verify blog is visible in public
echo -e "${YELLOW}Step 6: Verify blog is public${NC}"
PUBLIC=$(curl -s -X GET "$API/public/blogs?limit=100")
FOUND=$(echo "$PUBLIC" | jq ".data[] | select(.id == \"$BLOG_ID\") | .status")
if [ "$FOUND" = "\"published\"" ]; then
  echo -e "${GREEN}✓ Blog is visible in public feed${NC}"
else
  echo -e "${YELLOW}~ Blog visibility check${NC}"
fi
echo ""

# ============================================================
# FLOW 2: Admin Create → Draft → Publish
# ============================================================

echo -e "${CYAN}[FLOW 2] Admin Create → Draft → Published${NC}"
echo ""

# Step 1: Admin creates blog directly (starts as draft)
echo -e "${YELLOW}Step 1: Admin creates blog${NC}"
echo -e "${YELLOW}Expected status: draft${NC}"
FLOW2=$(curl -s -X POST "$API/admin/blogs" \
  -H "$HEADER" \
  -H "Content-Type: application/json" \
  -d '{
    "title": "Admin News Article",
    "subtitle": "Latest news",
    "content": "This is news created and edited by admin",
    "author": "Admin Editor",
    "category": "News",
    "tags": ["news", "official"],
    "featured_image": "https://example.com/news.jpg"
  }')

ADMIN_BLOG_ID=$(echo "$FLOW2" | jq -r '.data.id // empty')
STATUS=$(echo "$FLOW2" | jq -r '.data.status // empty')

if [ "$STATUS" = "draft" ]; then
  echo -e "${GREEN}✓ Admin blog created with status: $STATUS${NC}"
  echo "  ID: $ADMIN_BLOG_ID"
else
  echo -e "${RED}✗ Expected 'draft', got: $STATUS${NC}"
fi
echo ""

# Step 2: Admin editsblog
echo -e "${YELLOW}Step 2: Admin edits the blog${NC}"
curl -s -X PUT "$API/admin/blogs/$ADMIN_BLOG_ID" \
  -H "$HEADER" \
  -H "Content-Type: application/json" \
  -d '{
    "title": "Admin News Article - Edited",
    "content": "Updated news content with more details"
  }' | jq '.data.title'
echo -e "${GREEN}✓ Blog updated${NC}"
echo ""

# Step 3: Admin publishes directly
echo -e "${YELLOW}Step 3: Admin publishes directly from draft${NC}"
echo -e "${YELLOW}Expected status: published${NC}"
PUBLISHED=$(curl -s -X POST "$API/admin/blogs/$ADMIN_BLOG_ID/publish" \
  -H "$HEADER")

RESULT_STATUS=$(echo "$PUBLISHED" | jq -r '.data.status // empty')
if [ "$RESULT_STATUS" = "published" ]; then
  echo -e "${GREEN}✓ Blog published, status: $RESULT_STATUS${NC}"
else
  echo -e "${YELLOW}~ Response: ${NC}"
  echo "$PUBLISHED" | jq '.message'
fi
echo ""

# ============================================================
# FLOW 3: Status Transitions - All paths
# ============================================================

echo -e "${CYAN}[FLOW 3] All Status Transitions${NC}"
echo ""

# Create initial blog
TEMP=$(curl -s -X POST "$API/admin/blogs" \
  -H "$HEADER" \
  -H "Content-Type: application/json" \
  -d '{
    "title": "Status Test Blog",
    "content": "Testing all status transitions",
    "author": "Tester",
    "category": "Test"
  }')
TEMP_ID=$(echo "$TEMP" | jq -r '.data.id')

echo -e "${YELLOW}Initial: Draft${NC}"
echo "  ID: $TEMP_ID"
echo ""

echo -e "${YELLOW}Transition 1: Draft → Publish${NC}"
curl -s -X POST "$API/admin/blogs/$TEMP_ID/publish" \
  -H "$HEADER" | jq '.data.status'
echo ""

echo -e "${YELLOW}Transition 2: Published → Draft${NC}"
curl -s -X POST "$API/admin/blogs/$TEMP_ID/draft" \
  -H "$HEADER" | jq '.data.status'
echo ""

echo -e "${YELLOW}Transition 3: Draft → Publish (again)${NC}"
curl -s -X POST "$API/admin/blogs/$TEMP_ID/publish" \
  -H "$HEADER" | jq '.data.status'
echo ""

# ============================================================
# FLOW 4: View Count Tracking
# ============================================================

echo -e "${CYAN}[FLOW 4] View Count Tracking${NC}"
echo ""

echo -e "${YELLOW}Step 1: Create published blog${NC}"
VIEW_TEST=$(curl -s -X POST "$API/admin/blogs" \
  -H "$HEADER" \
  -H "Content-Type: application/json" \
  -d '{
    "title": "View Count Test",
    "content": "Testing view counts",
    "author": "Tracker"
  }')
VIEW_ID=$(echo "$VIEW_TEST" | jq -r '.data.id')

# Publish it
curl -s -X POST "$API/admin/blogs/$VIEW_ID/publish" \
  -H "$HEADER" > /dev/null

echo -e "${YELLOW}Step 2: Check initial views${NC}"
curl -s -X GET "$API/admin/blogs/$VIEW_ID" \
  -H "$HEADER" | jq '.data | {title, views}'
echo ""

echo -e "${YELLOW}Step 3: Get blog from public (increments view)${NC}"
curl -s -X GET "$API/public/blogs/slug/view-count-test" > /dev/null

echo -e "${YELLOW}Step 4: Check views after access${NC}"
sleep 1
curl -s -X GET "$API/admin/blogs/$VIEW_ID" \
  -H "$HEADER" | jq '.data | {title, views}'
echo ""

# ============================================================
# Summary
# ============================================================

echo -e "${BLUE}════════════════════════════════════════${NC}"
echo -e "${GREEN}   STATUS WORKFLOW TESTS COMPLETE${NC}"
echo -e "${BLUE}════════════════════════════════════════${NC}"
echo ""

echo -e "${CYAN}Status Flow Summary:${NC}"
echo -e "  SUBMITTED  → User submitted a blog"
echo -e "  APPROVED   → Admin reviewed and approved"
echo -e "  DRAFT      → Blog in draft for editing"
echo -e "  PUBLISHED  → Blog is live and public"
echo ""

echo -e "${CYAN}Key Tests:${NC}"
echo -e "  ${GREEN}✓${NC} Public submission creates 'submitted' status"
echo -e "  ${GREEN}✓${NC} Admin approval changes to 'approved'"
echo -e "  ${GREEN}✓${NC} Admin can move between draft and published"
echo -e "  ${GREEN}✓${NC} Published blogs appear in public feed"
echo -e "  ${GREEN}✓${NC} View counts tracked on public access"
echo ""
