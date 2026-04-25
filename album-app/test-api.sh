#!/usr/bin/env bash
set -euo pipefail

# Backward-compatible entrypoint: run the complete album E2E suite.
exec "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/e2e/run.sh"
