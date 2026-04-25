#!/usr/bin/env bash
set -euo pipefail

# Canonical workflow test script for token-based album sharing.
exec "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/e2e/run.sh"
