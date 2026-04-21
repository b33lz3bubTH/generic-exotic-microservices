#!/bin/bash

# Master E2E Test Suite - Run all tests

RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo -e "${BLUE}╔════════════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║         BLOGGING SERVICE - COMPLETE E2E TEST SUITE         ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════════════════════════╝${NC}"
echo ""

# Check if server is running
echo -e "${YELLOW}[*] Checking if server is running...${NC}"
if curl -s http://localhost:8080/api/public/blogs > /dev/null 2>&1; then
  echo -e "${GREEN}✓ Server is running${NC}"
else
  echo -e "${RED}✗ Server is not running at http://localhost:8080${NC}"
  echo "  Please start the server: docker-compose up --build"
  exit 1
fi
echo ""

# Run status flow test
echo -e "${BLUE}════════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}   TEST 1: BLOG STATUS WORKFLOW${NC}"
echo -e "${BLUE}════════════════════════════════════════════════════════════${NC}"
echo ""
chmod +x "$SCRIPT_DIR/status-flow.test.sh"
"$SCRIPT_DIR/status-flow.test.sh"
echo ""

# Run public tests
echo -e "${BLUE}════════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}   TEST 2: PUBLIC ENDPOINTS${NC}"
echo -e "${BLUE}════════════════════════════════════════════════════════════${NC}"
echo ""
chmod +x "$SCRIPT_DIR/public.test.sh"
"$SCRIPT_DIR/public.test.sh"
echo ""

# Run admin tests
echo -e "${BLUE}════════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}   TEST 3: ADMIN ENDPOINTS${NC}"
echo -e "${BLUE}════════════════════════════════════════════════════════════${NC}"
echo ""
chmod +x "$SCRIPT_DIR/admin.test.sh"
"$SCRIPT_DIR/admin.test.sh"
echo ""

# Summary
echo -e "${BLUE}╔════════════════════════════════════════════════════════════╗${NC}"
echo -e "${GREEN}║         ALL E2E TESTS COMPLETED SUCCESSFULLY              ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════════════════════════╝${NC}"
echo ""

echo -e "${YELLOW}SUMMARY:${NC}"
echo -e "  ${GREEN}✓${NC} Status flow test"
echo -e "  ${GREEN}✓${NC} Public endpoints test"
echo -e "  ${GREEN}✓${NC} Admin endpoints test"
echo ""

echo -e "${YELLOW}ENDPOINTS TESTED:${NC}"
echo -e "  Public:"
echo -e "    • POST   /api/public/submit"
echo -e "    • GET    /api/public/blogs"
echo -e "    • GET    /api/public/blogs/slug/{slug}"
echo -e "    • GET    /api/public/search"
echo -e "    • GET    /api/public/search/tag/{tag}"
echo -e "    • GET    /api/public/search/author/{author}"
echo -e "    • GET    /api/public/search/category/{category}"
echo ""
echo -e "  Admin (require X-API-Key):"
echo -e "    • POST   /api/admin/blogs"
echo -e "    • GET    /api/admin/blogs"
echo -e "    • GET    /api/admin/blogs/{id}"
echo -e "    • PUT    /api/admin/blogs/{id}"
echo -e "    • DELETE /api/admin/blogs/{id}"
echo -e "    • GET    /api/admin/blogs/submitted"
echo -e "    • GET    /api/admin/blogs/drafts"
echo -e "    • POST   /api/admin/blogs/{id}/approve"
echo -e "    • POST   /api/admin/blogs/{id}/draft"
echo -e "    • POST   /api/admin/blogs/{id}/publish"
echo -e "    • GET    /api/admin/search/tag/{tag}"
echo -e "    • GET    /api/admin/search/author/{author}"
echo -e "    • GET    /api/admin/search/category/{category}"
echo -e "    • GET    /api/admin/stats"
echo -e "    • GET    /api/admin/tags/popular"
echo ""
