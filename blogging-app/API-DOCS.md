# 📚 Blogging Service API Documentation

## Base URL
```
http://localhost:8080/api
```

---

## Response Format

All responses follow this standard format:

### Success Response (2xx)
```json
{
  "success": true,
  "message": "Operation description",
  "data": {}
}
```

### Error Response (4xx, 5xx)
```json
{
  "success": false,
  "message": "Error description"
}
```

---

## 1. Blog Operations

### 1.1 Create Blog

**Endpoint:** `POST /blogs`

**Headers:**
```
Content-Type: application/json
```

**Request Body:**
```json
{
  "title": "string (required)",
  "subtitle": "string (optional)",
  "content": "string (optional)",
  "author": "string (optional)",
  "category": "string (optional)",
  "featured_image": "string (optional)",
  "tags": ["string"] (optional)
}
```

**Response (201 Created):**
```json
{
  "success": true,
  "message": "Blog created successfully",
  "data": {
    "id": "507f1f77bcf86cd799439011"
  }
}
```

**Example:**
```bash
curl -X POST http://localhost:8080/api/blogs \
  -H "Content-Type: application/json" \
  -d '{
    "title": "My First Blog Post",
    "subtitle": "An exciting journey begins",
    "content": "This is the content of my first blog post...",
    "author": "John Doe",
    "category": "Lifestyle",
    "featured_image": "https://example.com/image.jpg",
    "tags": ["lifestyle", "journey", "first-post"]
  }'
```

---

### 1.2 Get All Blogs

**Endpoint:** `GET /blogs`

**Query Parameters:**
- `page` (optional, default: 1) - Page number for pagination
- `limit` (optional, default: 10) - Number of blogs per page

**Response (200 OK):**
```json
{
  "success": true,
  "message": "Blogs retrieved successfully",
  "data": [
    {
      "id": "507f1f77bcf86cd799439011",
      "title": "My First Blog Post",
      "subtitle": "An exciting journey begins",
      "slug": "my-first-blog-post",
      "content": "This is the content of my first blog post...",
      "author": "John Doe",
      "category": "Lifestyle",
      "published": false,
      "views": 0,
      "featured_image": "https://example.com/image.jpg",
      "tags": ["lifestyle", "journey", "first-post"],
      "created_at": "2025-04-21 10:30:45",
      "updated_at": "2025-04-21 10:30:45"
    }
  ]
}
```

**Example:**
```bash
# Get first 10 blogs
curl "http://localhost:8080/api/blogs"

# Get second page with 20 blogs per page
curl "http://localhost:8080/api/blogs?page=2&limit=20"
```

---

### 1.3 Get Blog by ID

**Endpoint:** `GET /blogs/{id}`

**Path Parameters:**
- `id` (required) - MongoDB ObjectId of the blog

**Response (200 OK):**
```json
{
  "success": true,
  "message": "Blog retrieved successfully",
  "data": {
    "id": "507f1f77bcf86cd799439011",
    "title": "My First Blog Post",
    "subtitle": "An exciting journey begins",
    "slug": "my-first-blog-post",
    "content": "This is the content of my first blog post...",
    "author": "John Doe",
    "category": "Lifestyle",
    "published": false,
    "views": 1,
    "featured_image": "https://example.com/image.jpg",
    "tags": ["lifestyle", "journey", "first-post"],
    "created_at": "2025-04-21 10:30:45",
    "updated_at": "2025-04-21 10:30:45"
  }
}
```

**Example:**
```bash
curl "http://localhost:8080/api/blogs/507f1f77bcf86cd799439011"
```

**Note:** View count is automatically incremented when fetching a blog.

---

### 1.4 Get Blog by Slug

**Endpoint:** `GET /blogs/slug/{slug}`

**Path Parameters:**
- `slug` (required) - URL-friendly slug of the blog

**Response (200 OK):** Same as 1.3

**Example:**
```bash
curl "http://localhost:8080/api/blogs/slug/my-first-blog-post"
```

---

### 1.5 Update Blog

**Endpoint:** `PUT /blogs/{id}`

**Path Parameters:**
- `id` (required) - MongoDB ObjectId of the blog

**Headers:**
```
Content-Type: application/json
```

**Request Body:**
```json
{
  "title": "string (optional)",
  "subtitle": "string (optional)",
  "content": "string (optional)",
  "author": "string (optional)",
  "category": "string (optional)",
  "featured_image": "string (optional)"
}
```

**Response (200 OK):** Same as 1.3 (returns updated blog)

**Example:**
```bash
curl -X PUT http://localhost:8080/api/blogs/507f1f77bcf86cd799439011 \
  -H "Content-Type: application/json" \
  -d '{
    "title": "My First Blog Post - Updated",
    "category": "Technology",
    "content": "Updated content..."
  }'
```

---

### 1.6 Delete Blog

**Endpoint:** `DELETE /blogs/{id}`

**Path Parameters:**
- `id` (required) - MongoDB ObjectId of the blog

**Response (200 OK):**
```json
{
  "success": true,
  "message": "Blog deleted successfully",
  "data": {}
}
```

**Example:**
```bash
curl -X DELETE "http://localhost:8080/api/blogs/507f1f77bcf86cd799439011"
```

---

## 2. Search Operations

### 2.1 Advanced Search

**Endpoint:** `GET /search`

**Query Parameters:**
- `q` (optional) - Search query (searches in title, subtitle, content)
- `author` (optional) - Filter by author name
- `category` (optional) - Filter by category
- `tags` (optional) - Comma-separated tags
- `page` (optional, default: 1) - Page number
- `limit` (optional, default: 10) - Results per page

**Response (200 OK):**
```json
{
  "success": true,
  "message": "Search completed successfully",
  "data": [
    // Array of blog objects matching criteria
  ]
}
```

**Examples:**
```bash
# Search by keyword
curl "http://localhost:8080/api/search?q=drogon"

# Search with multiple filters
curl "http://localhost:8080/api/search?q=web&author=John%20Doe&category=Technology"

# Search with pagination
curl "http://localhost:8080/api/search?q=framework&page=2&limit=20"
```

---

### 2.2 Search by Tag

**Endpoint:** `GET /search/tag/{tag}`

**Path Parameters:**
- `tag` (required) - Tag to search for

**Query Parameters:**
- `page` (optional, default: 1)
- `limit` (optional, default: 10)

**Response (200 OK):**
```json
{
  "success": true,
  "message": "Blogs retrieved successfully",
  "data": [
    // Array of blogs with the specified tag
  ]
}
```

**Example:**
```bash
curl "http://localhost:8080/api/search/tag/C++?page=1&limit=10"
curl "http://localhost:8080/api/search/tag/tutorial"
```

---

### 2.3 Search by Author

**Endpoint:** `GET /search/author/{author}`

**Path Parameters:**
- `author` (required) - Author name

**Query Parameters:**
- `page` (optional, default: 1)
- `limit` (optional, default: 10)

**Response (200 OK):** Same as 2.2

**Example:**
```bash
curl "http://localhost:8080/api/search/author/John%20Doe"
```

---

### 2.4 Search by Category

**Endpoint:** `GET /search/category/{category}`

**Path Parameters:**
- `category` (required) - Category name

**Query Parameters:**
- `page` (optional, default: 1)
- `limit` (optional, default: 10)

**Response (200 OK):** Same as 2.2

**Example:**
```bash
curl "http://localhost:8080/api/search/category/Technology"
curl "http://localhost:8080/api/search/category/tutorial"
```

---

## 3. Statistics

### 3.1 Get Blog Statistics

**Endpoint:** `GET /stats`

**Response (200 OK):**
```json
{
  "success": true,
  "message": "Stats retrieved successfully",
  "data": {
    "total_blogs": 42
  }
}
```

**Example:**
```bash
curl "http://localhost:8080/api/stats"
```

---

### 3.2 Get Popular Tags

**Endpoint:** `GET /tags/popular`

**Response (200 OK):**
```json
{
  "success": true,
  "message": "Tags retrieved successfully",
  "data": [
    {
      "tag": "c++",
      "count": 15
    },
    {
      "tag": "web",
      "count": 12
    },
    {
      "tag": "tutorial",
      "count": 8
    }
  ]
}
```

**Example:**
```bash
curl "http://localhost:8080/api/tags/popular"
```

---

## 4. Publish Management

### 4.1 Publish Blog

**Endpoint:** `POST /blogs/{id}/publish`

**Path Parameters:**
- `id` (required) - MongoDB ObjectId of the blog

**Response (200 OK):**
```json
{
  "success": true,
  "message": "Blog published successfully",
  "data": {}
}
```

**Example:**
```bash
curl -X POST "http://localhost:8080/api/blogs/507f1f77bcf86cd799439011/publish"
```

---

### 4.2 Unpublish Blog

**Endpoint:** `POST /blogs/{id}/unpublish`

**Path Parameters:**
- `id` (required) - MongoDB ObjectId of the blog

**Response (200 OK):**
```json
{
  "success": true,
  "message": "Blog unpublished successfully",
  "data": {}
}
```

**Example:**
```bash
curl -X POST "http://localhost:8080/api/blogs/507f1f77bcf86cd799439011/unpublish"
```

---

## HTTP Status Codes

| Code | Meaning |
|------|---------|
| 200 | OK - Request successful |
| 201 | Created - Resource successfully created |
| 400 | Bad Request - Invalid input |
| 404 | Not Found - Resource not found |
| 500 | Internal Server Error - Server error |

---

## Error Examples

### 400 Bad Request
```json
{
  "success": false,
  "message": "Title is required"
}
```

### 404 Not Found
```json
{
  "success": false,
  "message": "Blog not found"
}
```

### 500 Internal Server Error
```json
{
  "success": false,
  "message": "Internal server error"
}
```

---

## Rate Limiting

Currently no rate limiting is implemented. For production:
- Implement IP-based rate limiting
- Use request throttling
- Add token-based rate limits

---

## CORS

The API accepts requests from all origins (`*`). For production:
- Specify allowed origins
- Restrict methods
- Limit headers
- Handle credentials properly

---

## Authentication

Currently, the API is open. For production:
- Implement JWT authentication
- Add API key management
- Implement role-based access control (RBAC)
- Secure sensitive endpoints

---

## Performance Tips

1. **Use pagination** to reduce payload size
2. **Filter results** using category/author/tag params
3. **Cache frequently accessed blogs**
4. **Use slug-based access** for known URLs
5. **Monitor view counts** for analytics

---

## Example Workflow

```bash
# 1. Create a blog
BLOG_ID=$(curl -s -X POST http://localhost:8080/api/blogs \
  -H "Content-Type: application/json" \
  -d '{"title":"Test Blog","author":"Test"}' \
  | jq -r '.data.id')

# 2. Publish the blog
curl -X POST "http://localhost:8080/api/blogs/$BLOG_ID/publish"

# 3. Get the published blog
curl "http://localhost:8080/api/blogs/$BLOG_ID"

# 4. Search for blogs
curl "http://localhost:8080/api/search?q=Test"

# 5. Get statistics
curl "http://localhost:8080/api/stats"
```

---

## Related Documentation

- [README.md](./README.md) - Project overview and setup
- [CMakeLists.txt](./CMakeLists.txt) - Build configuration
- [docker-compose.yml](./docker-compose.yml) - Docker setup

---

**Last Updated:** April 21, 2025
