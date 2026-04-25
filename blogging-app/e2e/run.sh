#!/usr/bin/env bash
set -euo pipefail

API_BASE="${API_BASE:-http://localhost:8080/api}"
ADMIN_KEY="${ADMIN_KEY:-change-me-admin-key}"

log() { echo "[e2e] $*"; }
pass() { echo "✅ $*"; }
fail() { echo "❌ $*"; exit 1; }

assert_json_expr() {
  local payload="$1"
  local expr="$2"
  local message="$3"
  if echo "$payload" | jq -e "$expr" >/dev/null; then
    pass "$message"
  else
    echo "$payload" | jq '.' || true
    fail "$message"
  fi
}

assert_http_code() {
  local got="$1"
  local want="$2"
  local context="$3"
  if [[ "$got" == "$want" ]]; then
    pass "$context (HTTP $got)"
  else
    fail "$context (expected $want, got $got)"
  fi
}

post_json_with_code() {
  local url="$1"
  local body="$2"
  local auth_header="${3:-}"
  if [[ -n "$auth_header" ]]; then
    curl -sS -w $'\n%{http_code}' -X POST "$url" -H 'Content-Type: application/json' -H "$auth_header" -d "$body"
  else
    curl -sS -w $'\n%{http_code}' -X POST "$url" -H 'Content-Type: application/json' -d "$body"
  fi
}

get_with_code() {
  local url="$1"
  local auth_header="${2:-}"
  if [[ -n "$auth_header" ]]; then
    curl -sS -w $'\n%{http_code}' -X GET "$url" -H "$auth_header"
  else
    curl -sS -w $'\n%{http_code}' -X GET "$url"
  fi
}

split_body_code() {
  local response="$1"
  local http_code
  http_code="$(echo "$response" | tail -n1)"
  local body
  body="$(echo "$response" | sed '$d')"
  printf '%s\n%s' "$body" "$http_code"
}

log "Checking service health at $API_BASE/albums/stats/overview"
health="$(get_with_code "$API_BASE/albums/stats/overview")"
health_body="$(echo "$health" | sed '$d')"
health_code="$(echo "$health" | tail -n1)"
assert_http_code "$health_code" "200" "health endpoint reachable"
assert_json_expr "$health_body" '.success == true' "health response success"

log "1) Unauthorized admin call must fail"
unauth="$(get_with_code "$API_BASE/admin/albums")"
unauth_body="$(echo "$unauth" | sed '$d')"
unauth_code="$(echo "$unauth" | tail -n1)"
assert_http_code "$unauth_code" "401" "admin endpoint protected"
assert_json_expr "$unauth_body" '.success == false' "unauthorized payload"

log "2) Admin creates album and gets one-time token"
create_resp="$(post_json_with_code "$API_BASE/admin/albums" '{"title":"E2E Album","description":"Album moderation flow","uploader_name":"e2e-user"}' "X-Admin-Key: $ADMIN_KEY")"
create_body="$(echo "$create_resp" | sed '$d')"
create_code="$(echo "$create_resp" | tail -n1)"
assert_http_code "$create_code" "201" "album created"
assert_json_expr "$create_body" '.success == true and .data.id != null and .data.token != null' "album id/token present"
ALBUM_ID="$(echo "$create_body" | jq -r '.data.id')"
TOKEN="$(echo "$create_body" | jq -r '.data.token')"

IMG1_B64='iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mP8/x8AAwMCAO+X6xkAAAAASUVORK5CYII='
IMG2_B64='iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mP8z8AARQMCuY3Y+QAAAABJRU5ErkJggg=='

log "3) Token upload two images"
upload_payload="$(jq -n --arg album_id "$ALBUM_ID" --arg token "$TOKEN" --arg i1 "$IMG1_B64" --arg i2 "$IMG2_B64" '{album_id:$album_id, token:$token, images:[{file_name:"a.png", content_base64:$i1, alt_text:"a", caption:"first"}, {file_name:"b.png", content_base64:$i2, alt_text:"b", caption:"second"}]}')"
upload_resp="$(post_json_with_code "$API_BASE/admin/images/upload" "$upload_payload")"
upload_body="$(echo "$upload_resp" | sed '$d')"
upload_code="$(echo "$upload_resp" | tail -n1)"
assert_http_code "$upload_code" "200" "upload endpoint"
assert_json_expr "$upload_body" '.success == true and .data.uploaded_count == 2' "both images uploaded"

log "4) Submit draft with token (consumes token)"
submit_resp="$(post_json_with_code "$API_BASE/admin/albums/submit" "{\"album_id\":\"$ALBUM_ID\",\"token\":\"$TOKEN\"}")"
submit_body="$(echo "$submit_resp" | sed '$d')"
submit_code="$(echo "$submit_resp" | tail -n1)"
assert_http_code "$submit_code" "200" "submit draft"
assert_json_expr "$submit_body" '.data.status == "submitted"' "album moved to submitted"

log "5) Reusing token must fail"
reuse_resp="$(post_json_with_code "$API_BASE/admin/albums/submit" "{\"album_id\":\"$ALBUM_ID\",\"token\":\"$TOKEN\"}")"
reuse_code="$(echo "$reuse_resp" | tail -n1)"
assert_http_code "$reuse_code" "401" "token reuse blocked"

log "6) Admin moderates images: approve first, reject second"
admin_detail="$(get_with_code "$API_BASE/admin/albums/$ALBUM_ID" "X-Admin-Key: $ADMIN_KEY")"
admin_detail_body="$(echo "$admin_detail" | sed '$d')"
assert_json_expr "$admin_detail_body" '.data.images | length == 2' "admin sees both images"
IMAGE1_ID="$(echo "$admin_detail_body" | jq -r '.data.images[0].id')"
IMAGE2_ID="$(echo "$admin_detail_body" | jq -r '.data.images[1].id')"

approve_resp="$(post_json_with_code "$API_BASE/admin/images/$IMAGE1_ID/approve" "{\"album_id\":\"$ALBUM_ID\",\"image_id\":\"$IMAGE1_ID\"}" "X-Admin-Key: $ADMIN_KEY")"
assert_http_code "$(echo "$approve_resp" | tail -n1)" "200" "approve image"

reject_resp="$(post_json_with_code "$API_BASE/admin/images/$IMAGE2_ID/reject" "{\"album_id\":\"$ALBUM_ID\",\"image_id\":\"$IMAGE2_ID\"}" "X-Admin-Key: $ADMIN_KEY")"
assert_http_code "$(echo "$reject_resp" | tail -n1)" "200" "reject image"

log "7) Publish album"
publish_resp="$(post_json_with_code "$API_BASE/admin/albums/$ALBUM_ID/publish" '{}' "X-Admin-Key: $ADMIN_KEY")"
assert_http_code "$(echo "$publish_resp" | tail -n1)" "200" "publish album"

log "8) Public album view only shows approved image"
public_one="$(get_with_code "$API_BASE/albums/$ALBUM_ID")"
public_body="$(echo "$public_one" | sed '$d')"
assert_http_code "$(echo "$public_one" | tail -n1)" "200" "public album fetch"
assert_json_expr "$public_body" '.data.status == "published" and (.data.images | length) == 1' "public sees moderated approved images only"

log "9) Public list and stats"
public_list="$(get_with_code "$API_BASE/albums?page=1&limit=20")"
assert_http_code "$(echo "$public_list" | tail -n1)" "200" "public list fetch"
assert_json_expr "$(echo "$public_list" | sed '$d')" '.data | length >= 1' "album appears in public feed"

stats="$(get_with_code "$API_BASE/albums/stats/overview")"
assert_http_code "$(echo "$stats" | tail -n1)" "200" "stats fetch"
assert_json_expr "$(echo "$stats" | sed '$d')" '.data.published_albums >= 1 and .data.public_images >= 1' "stats include published/approved data"

log "10) Admin archive then delete"
archive_resp="$(post_json_with_code "$API_BASE/admin/albums/$ALBUM_ID/archive" '{}' "X-Admin-Key: $ADMIN_KEY")"
assert_http_code "$(echo "$archive_resp" | tail -n1)" "200" "archive album"

delete_resp="$(curl -sS -w $'\n%{http_code}' -X DELETE "$API_BASE/admin/albums/$ALBUM_ID" -H "X-Admin-Key: $ADMIN_KEY")"
assert_http_code "$(echo "$delete_resp" | tail -n1)" "200" "delete album"

pass "E2E suite finished successfully"
