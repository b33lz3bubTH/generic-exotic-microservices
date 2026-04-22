# 📸 Quick Copy-Paste Curl Commands

## Table of Contents
1. [Create Album](#create-album)
2. [View Album Details (Admin)](#view-album-details-admin)
3. [Upload Images](#upload-images)
4. [Approve/Reject/Flag Images](#image-management)
5. [Publish Album](#publish-album)
6. [Public Views](#public-views)

---

## Create Album

### Create a new album (generates one-time token)

```bash
curl -X POST http://localhost:8080/api/admin/albums \
  -H "Content-Type: application/json" \
  -d '{
    "title": "Summer Vacation 2024",
    "description": "Beach photos from July",
    "uploader_name": "Sarah Johnson"
  }'
```

**Response will include:**
- `id` - Album ID (use in subsequent requests)
- `token` - One-time upload token (use for image upload)

---

## View Album Details (Admin)

### View all albums you created

```bash
curl -X GET "http://localhost:8080/api/admin/albums?page=1&limit=10"
```

### View specific album with all images (including pending)

```bash
curl -X GET "http://localhost:8080/api/admin/albums/507f1f77bcf86cd799439011"
```

Replace `507f1f77bcf86cd799439011` with your album ID.

---

## Upload Images

### Upload images to album using token

```bash
curl -X POST http://localhost:8080/api/admin/images/upload \
  -H "X-Album-Token: a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6" \
  -F "album_id=507f1f77bcf86cd799439011" \
  -F "token=a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6" \
  -F "images=@photo1.jpg" \
  -F "alt_text=Beach sunset" \
  -F "caption=Beautiful sunset on the beach" \
  -F "display_order=1" \
  -F "images=@photo2.png" \
  -F "alt_text=Ocean waves" \
  -F "caption=Waves rolling in" \
  -F "display_order=2"
```

**Replace:**
- `album_id` with your album ID
- `token` with your one-time token
- `a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6` with your actual token
- `@photo1.jpg` and `@photo2.png` with paths to your image files

---

## Image Management

### Approve an image

```bash
curl -X POST http://localhost:8080/api/admin/images/img_001/approve \
  -H "Content-Type: application/json" \
  -d '{
    "album_id": "507f1f77bcf86cd799439011",
    "image_id": "img_001"
  }'
```

### Reject an image

```bash
curl -X POST http://localhost:8080/api/admin/images/img_002/reject \
  -H "Content-Type: application/json" \
  -d '{
    "album_id": "507f1f77bcf86cd799439011",
    "image_id": "img_002"
  }'
```

### Flag an image as NSFW

```bash
curl -X POST http://localhost:8080/api/admin/images/img_001/nsfw \
  -H "Content-Type: application/json" \
  -d '{
    "album_id": "507f1f77bcf86cd799439011",
    "image_id": "img_001",
    "reason": "Contains explicit content"
  }'
```

---

## Publish Album

### Publish album (make it live and visible to public)

```bash
curl -X POST http://localhost:8080/api/admin/albums/507f1f77bcf86cd799439011/publish
```

### Archive album

```bash
curl -X POST http://localhost:8080/api/admin/albums/507f1f77bcf86cd799439011/archive
```

### Delete album

```bash
curl -X DELETE http://localhost:8080/api/admin/albums/507f1f77bcf86cd799439011
```

---

## Public Views

### Get all published albums (no auth required)

```bash
curl -X GET "http://localhost:8080/api/albums?page=1&limit=10"
```

### Get specific album with only approved images

```bash
curl -X GET "http://localhost:8080/api/albums/507f1f77bcf86cd799439011"
```

### Search albums

```bash
curl -X GET "http://localhost:8080/api/albums/search?q=vacation"
```

### Get album statistics

```bash
curl -X GET "http://localhost:8080/api/albums/stats/overview"
```

---

## Complete Workflow Example

### 1. Create album
```bash
curl -X POST http://localhost:8080/api/admin/albums \
  -H "Content-Type: application/json" \
  -d '{"title": "My Photos", "description": "Test album", "uploader_name": "John Doe"}'
```
Save the `id` and `token` from response.

### 2. Upload images (use token from step 1)
```bash
curl -X POST http://localhost:8080/api/admin/images/upload \
  -H "X-Album-Token: YOUR_TOKEN" \
  -F "album_id=YOUR_ID" \
  -F "token=YOUR_TOKEN" \
  -F "images=@test.jpg" \
  -F "alt_text=Test photo" \
  -F "caption=A test" \
  -F "display_order=1"
```

### 3. Check album details
```bash
curl -X GET "http://localhost:8080/api/admin/albums/YOUR_ID"
```

### 4. Approve an image (replace with actual image id)
```bash
curl -X POST http://localhost:8080/api/admin/images/img_001/approve \
  -H "Content-Type: application/json" \
  -d '{"album_id": "YOUR_ID", "image_id": "img_001"}'
```

### 5. Publish album
```bash
curl -X POST http://localhost:8080/api/admin/albums/YOUR_ID/publish
```

### 6. View as public user
```bash
curl -X GET "http://localhost:8080/api/albums/YOUR_ID"
```

---

## Helpful Tips

- Replace `YOUR_ID` with your actual album ID
- Replace `YOUR_TOKEN` with your actual token
- Use `@filename` to upload files (must be jpg, jpeg, or png)
- Max file size: 50MB
- Max images per album: 40
- Image format: JPG, JPEG, PNG only
- Use `jq` for pretty JSON output: `... | jq '.'`

---

## Test Data Creation

Create simple test images:

```bash
# Create 1x1 pixel PNG files
printf '\x89PNG\r\n\x1a\n\x00\x00\x00\rIHDR\x00\x00\x00\x01\x00\x00\x00\x01\x08\x02\x00\x00\x00\x90wS\xde\x00\x00\x00\x0cIDATx\x9cc\xf8\x0f\x00\x00\x01\x01\x00\x05\x1f\xb7o\x00\x00\x00\x00IEND\xaeB`\x82' > test1.png
printf '\x89PNG\r\n\x1a\n\x00\x00\x00\rIHDR\x00\x00\x00\x01\x00\x00\x00\x01\x08\x02\x00\x00\x00\x90wS\xde\x00\x00\x00\x0cIDATx\x9cc\xf8\x0f\x00\x00\x01\x01\x00\x05\x1f\xb7o\x00\x00\x00\x00IEND\xaeB`\x82' > test2.png
printf '\x89PNG\r\n\x1a\n\x00\x00\x00\rIHDR\x00\x00\x00\x01\x00\x00\x00\x01\x08\x02\x00\x00\x00\x90wS\xde\x00\x00\x00\x0cIDATx\x9cc\xf8\x0f\x00\x00\x01\x01\x00\x05\x1f\xb7o\x00\x00\x00\x00IEND\xaeB`\x82' > test3.png
```

Then use these in upload commands:
```bash
-F "images=@test1.png" \
-F "images=@test2.png" \
-F "images=@test3.png"
```

---

## Service Status Check

Before running tests, make sure the service is running:

```bash
# Check if service is responding
curl http://localhost:8080/api/albums

# Check if MongoDB is running
curl http://localhost:8080/api/albums/stats/overview
```

If these fail, start the service:
```bash
# Build first time
cd blogging-app && mkdir build && cd build
cmake .. && make

# Run the service
./blogging_app

# Or use Docker
docker-compose up -d
```

---

## Common Issues

### 400 Bad Request
- Check JSON syntax (`jq` can help validate)
- Ensure all required fields are present
- Check field types (strings, numbers, etc.)

### 401 Unauthorized
- Token may be expired or already used
- Create a new album to get a fresh token

### 404 Not Found
- Album ID might be wrong
- Album may have been deleted

### 500 Internal Server Error
- Check MongoDB is running
- Check service logs for details

---

For more details, see `API-DOCS-ALBUM.md`
