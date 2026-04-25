#!/bin/bash

#############################################################################
#
# 📸 Photo Sharing Service - Step-by-Step Curl Commands
#
# This file contains individual curl commands you can run step-by-step
# to test the photo sharing service.
#
# Run these commands in order in your terminal
#
#############################################################################

# Configuration
BASE_URL="http://localhost:8080/api"
ADMIN_URL="http://localhost:8080/api/admin"

echo "================================"
echo "Photo Sharing Service - Curl Tests"
echo "================================"
echo ""

#############################################################################
# STEP 1: CREATE ALBUM (generates one-time token)
#############################################################################
echo "📝 STEP 1: Create Album"
echo "========================"
echo ""
echo "This creates a new album and returns a one-time token for uploading images."
echo ""
echo "Command:"
echo "--------"

cat << 'EOF'
curl -X POST http://localhost:8080/api/admin/albums \
  -H "Content-Type: application/json" \
  -d '{
    "title": "Summer Vacation 2024",
    "description": "Beach photos from July",
    "uploader_name": "Sarah Johnson"
  }'
EOF

echo ""
echo "Expected Response:"
echo "{"
echo '  "success": true,'
echo '  "message": "Album created successfully",'
echo '  "data": {'
echo '    "id": "507f1f77bcf86cd799439011",'
echo '    "token": "a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6",'
echo '    "message": "Use this token to upload images. It can only be used once."'
echo '  }'
echo '}'
echo ""
echo "💡 Save the 'id' and 'token' for next steps!"
echo ""
echo "Press Enter to continue..."
read

#############################################################################
# STEP 2: VIEW ADMIN ALBUMS (before upload)
#############################################################################
echo ""
echo "👀 STEP 2: View All Albums (Admin View)"
echo "=========================================="
echo ""
echo "View all albums including drafts not yet published."
echo ""
echo "Command:"
echo "--------"

cat << 'EOF'
curl -X GET "http://localhost:8080/api/admin/albums?page=1&limit=10"
EOF

echo ""
echo "Expected Response:"
echo "{"
echo '  "success": true,'
echo '  "message": "All albums retrieved successfully",'
echo '  "data": ['
echo '    {'
echo '      "id": "507f1f77bcf86cd799439011",'
echo '      "title": "Summer Vacation 2024",'
echo '      "status": "draft",'
echo '      "image_count": 0,'
echo '      "public_image_count": 0,'
echo '      "uploader_name": "Sarah Johnson",'
echo '      ...'
echo '    }'
echo '  ]'
echo '}'
echo ""
echo "Press Enter to continue..."
read

#############################################################################
# STEP 3: UPLOAD IMAGES TO ALBUM
#############################################################################
echo ""
echo "📤 STEP 3: Upload Images to Album"
echo "==================================="
echo ""
echo "Upload images using the token from Step 1."
echo "Note: You need actual image files (JPG/PNG) for this."
echo ""
echo "Command:"
echo "--------"

cat << 'EOFCURL'
curl -X POST http://localhost:8080/api/admin/images/upload \
  -H "X-Album-Token: YOUR_TOKEN_HERE" \
  -F "album_id=YOUR_ALBUM_ID_HERE" \
  -F "token=YOUR_TOKEN_HERE" \
  -F "images=@/path/to/photo1.jpg" \
  -F "alt_text=Beach sunset" \
  -F "caption=Beautiful sunset on the beach" \
  -F "display_order=1" \
  -F "images=@/path/to/photo2.png" \
  -F "alt_text=Ocean waves" \
  -F "caption=Waves at sunset" \
  -F "display_order=2"
EOFCURL

echo ""
echo "Note: Replace:"
echo "  - YOUR_TOKEN_HERE with the token from Step 1"
echo "  - YOUR_ALBUM_ID_HERE with the id from Step 1"
echo "  - /path/to/photo*.jpg with actual image paths"
echo ""
echo "Press Enter to continue..."
read

#############################################################################
# STEP 4: GET ALBUM DETAILS (Admin)
#############################################################################
echo ""
echo "📖 STEP 4: View Album Details (Admin - All Images)"
echo "======================================================"
echo ""
echo "View all images in the album (including pending ones)."
echo ""
echo "Command:"
echo "--------"

cat << 'EOF'
curl -X GET "http://localhost:8080/api/admin/albums/YOUR_ALBUM_ID_HERE"
EOF

echo ""
echo "Replace YOUR_ALBUM_ID_HERE with the id from Step 1"
echo ""
echo "Expected Response:"
echo "{"
echo '  "success": true,'
echo '  "message": "Album details retrieved successfully",'
echo '  "data": {'
echo '    "id": "507f1f77bcf86cd799439011",'
echo '    "title": "Summer Vacation 2024",'
echo '    "status": "submitted",'
echo '    "image_count": 2,'
echo '    "public_image_count": 0,'
echo '    "uploader_name": "Sarah Johnson",'
echo '    "images": ['
echo '      {'
echo '        "id": "img_001",'
echo '        "url": "/uploads/images/photo_001.jpg",'
echo '        "alt_text": "Beach sunset",'
echo '        "caption": "Beautiful sunset on the beach",'
echo '        "status": "pending",'
echo '        "nsfw_flagged": false'
echo '      },'
echo '      {'
echo '        "id": "img_002",'
echo '        "url": "/uploads/images/photo_002.png",'
echo '        "alt_text": "Ocean waves",'
echo '        "caption": "Waves at sunset",'
echo '        "status": "pending",'
echo '        "nsfw_flagged": false'
echo '      }'
echo '    ]'
echo '  }'
echo '}'
echo ""
echo "Press Enter to continue..."
read

#############################################################################
# STEP 5: APPROVE IMAGE
#############################################################################
echo ""
echo "✅ STEP 5: Approve Image"
echo "========================"
echo ""
echo "Admin approves individual images to make them visible to public."
echo ""
echo "Command:"
echo "--------"

cat << 'EOF'
curl -X POST http://localhost:8080/api/admin/images/img_001/approve \
  -H "Content-Type: application/json" \
  -d '{
    "album_id": "YOUR_ALBUM_ID_HERE",
    "image_id": "img_001"
  }'
EOF

echo ""
echo "Replace:"
echo "  - YOUR_ALBUM_ID_HERE with the id from Step 1"
echo "  - img_001 with the image id from Step 4"
echo ""
echo "Expected Response:"
echo "{"
echo '  "success": true,'
echo '  "message": "Image approved"'
echo '}'
echo ""
echo "Press Enter to continue..."
read

#############################################################################
# STEP 6: REJECT IMAGE
#############################################################################
echo ""
echo "❌ STEP 6: Reject Image"
echo "======================="
echo ""
echo "Admin can reject images they don't want to show."
echo ""
echo "Command:"
echo "--------"

cat << 'EOF'
curl -X POST http://localhost:8080/api/admin/images/img_002/reject \
  -H "Content-Type: application/json" \
  -d '{
    "album_id": "YOUR_ALBUM_ID_HERE",
    "image_id": "img_002"
  }'
EOF

echo ""
echo "Replace YOUR_ALBUM_ID_HERE with the id from Step 1"
echo ""
echo "Press Enter to continue..."
read

#############################################################################
# STEP 7: FLAG IMAGE AS NSFW
#############################################################################
echo ""
echo "🚫 STEP 7: Flag Image as NSFW"
echo "=============================="
echo ""
echo "Admin can flag inappropriate content."
echo ""
echo "Command:"
echo "--------"

cat << 'EOF'
curl -X POST http://localhost:8080/api/admin/images/img_xyz/nsfw \
  -H "Content-Type: application/json" \
  -d '{
    "album_id": "YOUR_ALBUM_ID_HERE",
    "image_id": "img_xyz",
    "reason": "Contains explicit content"
  }'
EOF

echo ""
echo "Press Enter to continue..."
read

#############################################################################
# STEP 8: PUBLISH ALBUM
#############################################################################
echo ""
echo "📢 STEP 8: Publish Album"
echo "========================"
echo ""
echo "Move the album to PUBLISHED status (makes it live)."
echo ""
echo "Command:"
echo "--------"

cat << 'EOF'
curl -X POST http://localhost:8080/api/admin/albums/YOUR_ALBUM_ID_HERE/publish
EOF

echo ""
echo "Expected Response:"
echo "{"
echo '  "success": true,'
echo '  "message": "Album published"'
echo '}'
echo ""
echo "Press Enter to continue..."
read

#############################################################################
# STEP 9: VIEW PUBLISHED ALBUMS (PUBLIC)
#############################################################################
echo ""
echo "🔓 STEP 9: View Published Albums (Public - No Auth)"
echo "====================================================="
echo ""
echo "Publicly view all published albums."
echo ""
echo "Command:"
echo "--------"

cat << 'EOF'
curl -X GET "http://localhost:8080/api/albums?page=1&limit=10"
EOF

echo ""
echo "Expected Response:"
echo "{"
echo '  "success": true,'
echo '  "message": "Published albums retrieved successfully",'
echo '  "data": ['
echo '    {'
echo '      "id": "507f1f77bcf86cd799439011",'
echo '      "title": "Summer Vacation 2024",'
echo '      "status": "published",'
echo '      "image_count": 2,'
echo '      "public_image_count": 1,'
echo '      "uploader_name": "Sarah Johnson",'
echo '      "created_at": "2024-04-20T10:30:00Z",'
echo '      "published_at": "2024-04-22T14:00:00Z"'
echo '    }'
echo '  ]'
echo '}'
echo ""
echo "Press Enter to continue..."
read

#############################################################################
# STEP 10: VIEW SINGLE ALBUM (PUBLIC)
#############################################################################
echo ""
echo "📖 STEP 10: View Album (Public - Approved Images Only)"
echo "========================================================"
echo ""
echo "View a specific album with only approved images visible."
echo ""
echo "Command:"
echo "--------"

cat << 'EOF'
curl -X GET "http://localhost:8080/api/albums/YOUR_ALBUM_ID_HERE"
EOF

echo ""
echo "Expected Response:"
echo "{"
echo '  "success": true,'
echo '  "message": "Album retrieved successfully",'
echo '  "data": {'
echo '    "id": "507f1f77bcf86cd799439011",'
echo '    "title": "Summer Vacation 2024",'
echo '    "uploader_name": "Sarah Johnson",'
echo '    "status": "published",'
echo '    "image_count": 2,'
echo '    "public_image_count": 1,'
echo '    "images": ['
echo '      {'
echo '        "id": "img_001",'
echo '        "url": "/uploads/images/photo_001.jpg",'
echo '        "alt_text": "Beach sunset",'
echo '        "caption": "Beautiful sunset on the beach",'
echo '        "status": "approved",'
echo '        "nsfw_flagged": false'
echo '      }'
echo '    ]'
echo '  }'
echo '}'
echo ""
echo "Note: Only 1 image shown (img_002 was rejected)"
echo ""
echo "Press Enter to continue..."
read

#############################################################################
# STEP 11: SEARCH ALBUMS
#############################################################################
echo ""
echo "🔍 STEP 11: Search Albums"
echo "=========================="
echo ""
echo "Search for albums by keyword."
echo ""
echo "Command:"
echo "--------"

cat << 'EOF'
curl -X GET "http://localhost:8080/api/albums/search?q=vacation"
EOF

echo ""
echo "Expected Response:"
echo "{"
echo '  "success": true,'
echo '  "message": "Search results",'
echo '  "data": ['
echo '    {'
echo '      "id": "507f1f77bcf86cd799439011",'
echo '      "title": "Summer Vacation 2024",'
echo '      "uploader_name": "Sarah Johnson",'
echo '      "status": "published",'
echo '      "image_count": 2,'
echo '      "public_image_count": 1'
echo '    }'
echo '  ]'
echo '}'
echo ""
echo "Press Enter to continue..."
read

#############################################################################
# STEP 12: GET STATISTICS
#############################################################################
echo ""
echo "📊 STEP 12: Get Album Statistics"
echo "=================================="
echo ""
echo "View overall statistics for all albums."
echo ""
echo "Command:"
echo "--------"

cat << 'EOF'
curl -X GET "http://localhost:8080/api/albums/stats/overview"
EOF

echo ""
echo "Expected Response:"
echo "{"
echo '  "success": true,'
echo '  "message": "Album statistics",'
echo '  "data": {'
echo '    "total_albums": 5,'
echo '    "published_albums": 3,'
echo '    "total_images": 25,'
echo '    "public_images": 18'
echo '  }'
echo '}'
echo ""
echo "Press Enter to continue..."
read

#############################################################################
# STEP 13: ARCHIVE ALBUM
#############################################################################
echo ""
echo "📦 STEP 13: Archive Album"
echo "=========================="
echo ""
echo "Archive the album (remove from active view)."
echo ""
echo "Command:"
echo "--------"

cat << 'EOF'
curl -X POST http://localhost:8080/api/admin/albums/YOUR_ALBUM_ID_HERE/archive
EOF

echo ""
echo "Expected Response:"
echo "{"
echo '  "success": true,'
echo '  "message": "Album archived"'
echo '}'
echo ""
echo "Press Enter to continue..."
read

#############################################################################
# STEP 14: DELETE ALBUM
#############################################################################
echo ""
echo "🗑️  STEP 14: Delete Album"
echo "============================"
echo ""
echo "Permanently delete the album."
echo ""
echo "Command:"
echo "--------"

cat << 'EOF'
curl -X DELETE http://localhost:8080/api/admin/albums/YOUR_ALBUM_ID_HERE
EOF

echo ""
echo "Expected Response:"
echo "{"
echo '  "success": true,'
echo '  "message": "Album deleted"'
echo '}'
echo ""

#############################################################################
# SUMMARY
#############################################################################
echo ""
echo "================================"
echo "✅ All Tests Complete!"
echo "================================"
echo ""
echo "Summary of API Endpoints:"
echo ""
echo "Admin Endpoints:"
echo "  POST   /admin/albums - Create album"
echo "  GET    /admin/albums - View all albums"
echo "  GET    /admin/albums/{id} - View album details"
echo "  POST   /admin/albums/{id}/publish - Publish"
echo "  POST   /admin/albums/{id}/archive - Archive"
echo "  DELETE /admin/albums/{id} - Delete"
echo "  POST   /admin/images/upload - Upload images"
echo "  POST   /admin/images/{id}/approve - Approve image"
echo "  POST   /admin/images/{id}/reject - Reject image"
echo "  POST   /admin/images/{id}/nsfw - Flag NSFW"
echo ""
echo "Public Endpoints:"
echo "  GET /albums - List published albums"
echo "  GET /albums/{id} - View album (approved images only)"
echo "  GET /albums/search?q=query - Search albums"
echo "  GET /albums/stats/overview - Get statistics"
echo ""
echo "For more details, see API-DOCS-ALBUM.md"
echo ""
