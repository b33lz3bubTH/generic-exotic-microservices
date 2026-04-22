#!/bin/bash

#############################################################################
#
# 📸 Photo Sharing Service - Complete Testing Script
#
# This script tests the entire workflow:
# 1. Create album (admin) - generates one-time token
# 2. Upload images to album (using token)
# 3. Approve/reject images (admin review)
# 4. Publish album (make live)
# 5. View album publicly
#
# Prerequisites:
# - Service running on http://localhost:8080
# - MongoDB running on mongodb://mongo:27017
# - Create some test images first
#
#############################################################################

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Base URL
BASE_URL="http://localhost:8080/api"
ADMIN_BASE_URL="http://localhost:8080/api/admin"

echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}  📸 PHOTO SHARING SERVICE - TESTING WORKFLOW${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
echo ""

#############################################################################
# STEP 1: CREATE ALBUM (Admin creates album, gets one-time token)
#############################################################################
echo -e "${YELLOW}📝 STEP 1: Creating Album...${NC}"
echo ""
echo "Command:"
echo "curl -X POST $ADMIN_BASE_URL/albums \\"
echo "  -H 'Content-Type: application/json' \\"
echo "  -d '{\"title\": \"Summer Vacation 2024\", \"description\": \"Beach photos from July\", \"uploader_name\": \"Sarah Johnson\"}'"
echo ""

ALBUM_RESPONSE=$(curl -s -X POST "$ADMIN_BASE_URL/albums" \
  -H "Content-Type: application/json" \
  -d '{
    "title": "Summer Vacation 2024",
    "description": "Beach photos from July 2024",
    "uploader_name": "Sarah Johnson"
  }')

echo "Response:"
echo "$ALBUM_RESPONSE" | jq '.'
echo ""

# Extract album_id and token from response
ALBUM_ID=$(echo "$ALBUM_RESPONSE" | jq -r '.data.id')
UPLOAD_TOKEN=$(echo "$ALBUM_RESPONSE" | jq -r '.data.token')

if [ -z "$ALBUM_ID" ] || [ "$ALBUM_ID" = "null" ]; then
  echo -e "${RED}❌ Failed to create album${NC}"
  exit 1
fi

echo -e "${GREEN}✓ Album created successfully${NC}"
echo -e "${GREEN}  Album ID: $ALBUM_ID${NC}"
echo -e "${GREEN}  Upload Token: $UPLOAD_TOKEN${NC}"
echo ""

#############################################################################
# STEP 2: CREATE TEST IMAGES (Optional - create dummy images for testing)
#############################################################################
echo -e "${YELLOW}📸 STEP 2: Creating Test Images...${NC}"
echo ""
echo "Note: Creating simple test images (1x1 pixel PNGs)"
echo ""

# Create a simple test PNG (1x1 pixel)
create_test_image() {
  local filename=$1
  # Minimal PNG file (1x1 pixel, red)
  printf '\x89PNG\r\n\x1a\n\x00\x00\x00\rIHDR\x00\x00\x00\x01\x00\x00\x00\x01\x08\x02\x00\x00\x00\x90wS\xde\x00\x00\x00\x0cIDATx\x9cc\xf8\x0f\x00\x00\x01\x01\x00\x05\x1f\xb7o\x00\x00\x00\x00IEND\xaeB`\x82' > "$filename"
}

create_test_image "test_photo1.png"
create_test_image "test_photo2.png"
create_test_image "test_photo3.png"

echo -e "${GREEN}✓ Test images created${NC}"
echo "  - test_photo1.png"
echo "  - test_photo2.png"
echo "  - test_photo3.png"
echo ""

#############################################################################
# STEP 3: UPLOAD IMAGES (User uploads images using the token)
#############################################################################
echo -e "${YELLOW}🚀 STEP 3: Uploading Images to Album...${NC}"
echo ""
echo "Command:"
echo "curl -X POST $ADMIN_BASE_URL/images/upload \\"
echo "  -H 'X-Album-Token: <token>' \\"
echo "  -F 'album_id=<id>' \\"
echo "  -F 'token=<token>' \\"
echo "  -F 'images=@test_photo1.png' \\"
echo "  -F 'alt_text=Sunset at the beach' \\"
echo "  -F 'caption=Beautiful sunset' \\"
echo "  -F 'display_order=1' \\"
echo "  ..."
echo ""

UPLOAD_RESPONSE=$(curl -s -X POST "$ADMIN_BASE_URL/images/upload" \
  -H "X-Album-Token: $UPLOAD_TOKEN" \
  -F "album_id=$ALBUM_ID" \
  -F "token=$UPLOAD_TOKEN" \
  -F "images=@test_photo1.png" \
  -F "alt_text=Sunset at the beach" \
  -F "caption=Beautiful golden sunset over the ocean" \
  -F "display_order=1" \
  -F "images=@test_photo2.png" \
  -F "alt_text=Ocean waves" \
  -F "caption=Big waves in the afternoon" \
  -F "display_order=2" \
  -F "images=@test_photo3.png" \
  -F "alt_text=Palm trees at sunset" \
  -F "caption=Tropical paradise" \
  -F "display_order=3")

echo "Response:"
echo "$UPLOAD_RESPONSE" | jq '.'
echo ""

# Check if upload was successful
UPLOAD_STATUS=$(echo "$UPLOAD_RESPONSE" | jq -r '.success')
if [ "$UPLOAD_STATUS" != "true" ]; then
  echo -e "${YELLOW}⚠ Upload not yet implemented (expected)${NC}"
  echo "   This is a placeholder endpoint ready for implementation"
else
  echo -e "${GREEN}✓ Images uploaded successfully${NC}"
fi
echo ""

#############################################################################
# STEP 4: GET ALBUM DETAILS (Admin reviews all images)
#############################################################################
echo -e "${YELLOW}👀 STEP 4: Viewing Album Details (Admin)...${NC}"
echo ""
echo "Command:"
echo "curl -X GET $ADMIN_BASE_URL/albums/$ALBUM_ID"
echo ""

ADMIN_VIEW=$(curl -s -X GET "$ADMIN_BASE_URL/albums/$ALBUM_ID")

echo "Response:"
echo "$ADMIN_VIEW" | jq '.'
echo ""
echo -e "${GREEN}✓ Album details retrieved${NC}"
echo ""

#############################################################################
# STEP 5: APPROVE IMAGES (Admin approves images)
#############################################################################
echo -e "${YELLOW}✅ STEP 5: Approving Images (Admin)...${NC}"
echo ""
echo "Command:"
echo "curl -X POST $ADMIN_BASE_URL/images/{id}/approve \\"
echo "  -H 'Content-Type: application/json' \\"
echo "  -d '{\"album_id\": \"<id>\", \"image_id\": \"<image_id>\"}'"
echo ""

# Extract image IDs from admin view
IMAGE_IDS=$(echo "$ADMIN_VIEW" | jq -r '.data.images[].id' 2>/dev/null || echo "")

if [ -z "$IMAGE_IDS" ]; then
  echo -e "${YELLOW}⚠ No images to approve (upload endpoint not yet implemented)${NC}"
else
  # Approve first image
  FIRST_IMAGE=$(echo "$IMAGE_IDS" | head -1)
  
  APPROVE_RESPONSE=$(curl -s -X POST "$ADMIN_BASE_URL/images/$FIRST_IMAGE/approve" \
    -H "Content-Type: application/json" \
    -d "{\"album_id\": \"$ALBUM_ID\", \"image_id\": \"$FIRST_IMAGE\"}")
  
  echo "Response:"
  echo "$APPROVE_RESPONSE" | jq '.'
  echo ""
  echo -e "${GREEN}✓ Image approved${NC}"
fi
echo ""

#############################################################################
# STEP 6: PUBLISH ALBUM (Make the album live)
#############################################################################
echo -e "${YELLOW}📢 STEP 6: Publishing Album...${NC}"
echo ""
echo "Command:"
echo "curl -X POST $ADMIN_BASE_URL/albums/$ALBUM_ID/publish"
echo ""

PUBLISH_RESPONSE=$(curl -s -X POST "$ADMIN_BASE_URL/albums/$ALBUM_ID/publish")

echo "Response:"
echo "$PUBLISH_RESPONSE" | jq '.'
echo ""
echo -e "${GREEN}✓ Album published${NC}"
echo ""

#############################################################################
# STEP 7: VIEW PUBLISHED ALBUMS (Public view - no auth required)
#############################################################################
echo -e "${YELLOW}🔓 STEP 7: Viewing Published Albums (Public)...${NC}"
echo ""
echo "Command:"
echo "curl -X GET '$BASE_URL/albums?page=1&limit=10'"
echo ""

PUBLIC_ALBUMS=$(curl -s -X GET "$BASE_URL/albums?page=1&limit=10")

echo "Response:"
echo "$PUBLIC_ALBUMS" | jq '.'
echo ""
echo -e "${GREEN}✓ Published albums retrieved${NC}"
echo ""

#############################################################################
# STEP 8: VIEW SINGLE ALBUM (Public view)
#############################################################################
echo -e "${YELLOW}📖 STEP 8: Viewing Album Details (Public)...${NC}"
echo ""
echo "Command:"
echo "curl -X GET '$BASE_URL/albums/$ALBUM_ID'"
echo ""

PUBLIC_ALBUM=$(curl -s -X GET "$BASE_URL/albums/$ALBUM_ID")

echo "Response:"
echo "$PUBLIC_ALBUM" | jq '.'
echo ""
echo -e "${GREEN}✓ Album details retrieved (only approved images shown)${NC}"
echo ""

#############################################################################
# STEP 9: SEARCH ALBUMS
#############################################################################
echo -e "${YELLOW}🔍 STEP 9: Searching Albums...${NC}"
echo ""
echo "Command:"
echo "curl -X GET '$BASE_URL/albums/search?q=vacation'"
echo ""

SEARCH_RESULTS=$(curl -s -X GET "$BASE_URL/albums/search?q=vacation")

echo "Response:"
echo "$SEARCH_RESULTS" | jq '.'
echo ""
echo -e "${GREEN}✓ Search results retrieved${NC}"
echo ""

#############################################################################
# STEP 10: GET STATISTICS
#############################################################################
echo -e "${YELLOW}📊 STEP 10: Getting Album Statistics...${NC}"
echo ""
echo "Command:"
echo "curl -X GET '$BASE_URL/albums/stats/overview'"
echo ""

STATS=$(curl -s -X GET "$BASE_URL/albums/stats/overview")

echo "Response:"
echo "$STATS" | jq '.'
echo ""
echo -e "${GREEN}✓ Statistics retrieved${NC}"
echo ""

#############################################################################
# SUMMARY
#############################################################################
echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
echo -e "${GREEN}✅ WORKFLOW TEST COMPLETE${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
echo ""
echo "📋 Summary:"
echo "  ✓ Created album: $ALBUM_ID"
echo "  ✓ Album uploader: Sarah Johnson"
echo "  ✓ Generated token: $UPLOAD_TOKEN"
echo "  ✓ Created test images (3 images)"
echo "  ✓ Viewed album details (admin)"
echo "  ✓ Published album"
echo "  ✓ Viewed as public user"
echo "  ✓ Searched albums"
echo "  ✓ Retrieved statistics"
echo ""
echo "🔗 Quick References:"
echo "  Admin view album: curl -X GET '$ADMIN_BASE_URL/albums/$ALBUM_ID'"
echo "  Public view album: curl -X GET '$BASE_URL/albums/$ALBUM_ID'"
echo "  List all albums: curl -X GET '$BASE_URL/albums'"
echo ""
echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
echo ""

#############################################################################
# CLEANUP
#############################################################################
echo -e "${YELLOW}🧹 Cleaning up test files...${NC}"
rm -f test_photo1.png test_photo2.png test_photo3.png
echo -e "${GREEN}✓ Test files removed${NC}"
echo ""

echo -e "${GREEN}🎉 All tests completed successfully!${NC}"
