# Photo Sharing API (Album-only)

Base URL: `http://localhost:8080/api`

This service is album-centric (Instagram-like photo sharing), with strict moderation:
- One token creates one album submission flow.
- Max 40 images per album.
- Album starts as `draft`, then gets `submitted` once uploader finalizes.
- Admin moderates images and album publication.
- Local disk image storage today (R2 adapter can be plugged in later).

## Security Model

- Admin endpoints require `X-Admin-Key` header.
- Upload/submit endpoints require album upload token.
- Upload endpoint includes rate limiting (IP + token).
- Only `jpg`, `jpeg`, `png` allowed.
- Public API only exposes approved images from published albums.

## Response Shape

```json
{
  "success": true,
  "message": "...",
  "data": {}
}
```

---

## Public Endpoints

### `GET /albums`
Get published albums.

Query params:
- `page` (default `1`)
- `limit` (default `10`)

### `GET /albums/{id}`
Get one album for public consumption (approved images only).

### `GET /albums/stats/overview`
Read-only stats for published/public totals.

---

## Admin Endpoints

### `POST /admin/albums`
Create album + one-time upload token.

Headers:
- `X-Admin-Key: <admin-key>`

Body:
```json
{
  "title": "Trip to Alps",
  "description": "Winter album",
  "uploader_name": "john"
}
```

### `POST /admin/images/upload`
Upload one or many images while album is in `draft`.

JSON body:
```json
{
  "album_id": "<album-id>",
  "token": "<album-token>",
  "images": [
    {
      "file_name": "cover.png",
      "content_base64": "<base64-image-bytes>",
      "alt_text": "optional",
      "caption": "optional"
    }
  ]
}
```

### `POST /admin/albums/submit`
Finalize draft upload and consume token. After this, no more image upload.

Body:
```json
{
  "album_id": "<album-id>",
  "token": "<album-token>"
}
```

### `GET /admin/albums`
List all albums (all states). Requires admin key.

### `GET /admin/albums/{id}`
Get full album details (all images). Requires admin key.

### `POST /admin/images/{id}/approve`
Approve one image.

Body:
```json
{
  "album_id": "<album-id>",
  "image_id": "<image-id>"
}
```

### `POST /admin/images/{id}/reject`
Reject one image.

### `POST /admin/images/{id}/nsfw`
Flag image as NSFW.

Body:
```json
{
  "album_id": "<album-id>",
  "image_id": "<image-id>",
  "reason": "policy_violation"
}
```

### `POST /admin/albums/{id}/publish`
Publish album to public site.

### `POST /admin/albums/{id}/archive`
Archive album.

### `DELETE /admin/albums/{id}`
Delete album.

---

## Stateless Scaling Notes

Current upload rate limiting is in-process (works per instance). To make it truly stateless across replicas, replace limiter storage with Redis and keep the same API contract.
