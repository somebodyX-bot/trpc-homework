#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
run_dir="evidence/runs/$(date +%Y%m%d-%H%M%S)"
mkdir -p "$run_dir"
if [[ "${1:-}" == "--build" ]]; then
  docker compose --progress plain build 2>&1 | tee "$run_dir/build.log"
fi
docker compose up -d --wait --wait-timeout 120 server 2>&1 | tee "$run_dir/start.log"
restore() {
  local result=$?
  trap - EXIT
  docker compose up -d --wait --wait-timeout 120 server || result=1
  docker compose logs --no-color server > "$run_dir/server.log" 2>&1
  docker compose ps --all > "$run_dir/containers.log"
  exit "$result"
}
trap restore EXIT
docker compose run --rm --no-deps client 2>&1 | tee "$run_dir/client.log"
docker compose stop server 2>&1 | tee "$run_dir/stop.log"
started=$SECONDS
set +e
timeout 30 docker compose run --rm --no-deps client 2>&1 | tee "$run_dir/unavailable.log"
status=${PIPESTATUS[0]}
set -e
[[ "$status" == 2 ]] || { echo "Expected RPC failure exit 2, got $status"; exit 1; }
echo "PASS unavailable server: exit=$status elapsed=$((SECONDS-started)) seconds" | tee "$run_dir/negative-result.log"
echo "PASS: normal, boundary, and unavailable-server tests. Evidence: $run_dir" | tee "$run_dir/result.log"
