Here’s a concise cURL documentation for your new products API, covering all CRUD and pagination endpoints.

---

## 1. Create Product

```bash
curl -X POST http://localhost:8080/products \
  -H "Content-Type: application/json" \
  -d '{
    "name": "Essential Cotton Tee",
    "category": "fashion",
    "description": "Premium organic cotton t-shirt with a relaxed fit.",
    "price": 4900,
    "featured": true,
    "images": [
      "https://images.unsplash.com/photo-1512436991641-6745cdb1723f?w=600&q=80&fit=crop",
      "https://images.unsplash.com/photo-1517841905240-472988babdf9?w=600&q=80&fit=crop"
    ],
    "variants": []
  }'
```

#### With Variants

```bash
curl -X POST http://localhost:8080/products \
  -H "Content-Type: application/json" \
  -d '{
    "name": "Minimal Leather Jacket",
    "category": "fashion",
    "description": "Genuine leather jacket with clean lines and modern cut.",
    "price": 0,
    "featured": false,
    "images": [
      "https://images.unsplash.com/photo-1512436991641-6745cdb1723f?w=600&q=80&fit=crop",
      "https://images.unsplash.com/photo-1519125323398-675f0ddb6308?w=600&q=80&fit=crop"
    ],
    "variants": [
      {
        "sku": "LJ-BLK-M",
        "color": "Black",
        "size": "M",
        "image_url": "https://images.unsplash.com/photo-1512436991641-6745cdb1723f?w=600&q=80&fit=crop",
        "price": 29900,
        "in_stock": true
      },
      {
        "sku": "LJ-BRN-L",
        "color": "Brown",
        "size": "L",
        "image_url": "https://images.unsplash.com/photo-1519125323398-675f0ddb6308?w=600&q=80&fit=crop",
        "price": 31900,
        "in_stock": true
      }
    ]
  }'
```

---

## 2. Update Product

```bash
curl -X PUT http://localhost:8080/products/<PRODUCT_ID> \
  -H "Content-Type: application/json" \
  -d '{
    "name": "Updated Name",
    "category": "fashion",
    "description": "Updated description.",
    "price": 5000,
    "featured": false,
    "images": [
      "https://images.unsplash.com/photo-1512436991641-6745cdb1723f?w=600&q=80&fit=crop"
    ],
    "variants": []
  }'
```

---

## 3. Delete Product

```bash
curl -X DELETE http://localhost:8080/products/<PRODUCT_ID>
```

---

## 4. Get Product by ID

```bash
curl http://localhost:8080/products/<PRODUCT_ID>
```

---

## 5. List Products (Paginated)

```bash
curl "http://localhost:8080/products?skip=0&take=10"
```

---

### Notes

- Replace `<PRODUCT_ID>` with the actual product ID (string).
- All prices are in the smallest currency unit (e.g., cents).
- For products with variants, set `"price": 0` in the main product and specify prices in each variant.
- The API returns JSON responses.

If you need example responses or more advanced queries, let me know!
