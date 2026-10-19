param([switch]$Build)
$ErrorActionPreference = 'Stop'
Set-Location (Join-Path $PSScriptRoot '..')
$runDir = Join-Path 'evidence/runs' (Get-Date -Format 'yyyyMMdd-HHmmss')
New-Item -ItemType Directory -Force $runDir | Out-Null
if ($Build) {
    docker compose --progress plain build 2>&1 | Tee-Object "$runDir/build.log"
    if ($LASTEXITCODE -ne 0) { throw 'Image build failed' }
}
docker compose up -d --wait --wait-timeout 120 server 2>&1 | Tee-Object "$runDir/start.log"
if ($LASTEXITCODE -ne 0) { throw 'Server did not become healthy' }
try {
    docker compose run --rm --no-deps client 2>&1 | Tee-Object "$runDir/client.log"
    if ($LASTEXITCODE -ne 0) { throw 'RPC verification failed' }
    docker compose stop server 2>&1 | Tee-Object "$runDir/stop.log"
    if ($LASTEXITCODE -ne 0) { throw 'Cannot stop server for negative test' }
    $started = Get-Date
    docker compose run --rm --no-deps client 2>&1 | Tee-Object "$runDir/unavailable.log"
    $clientExit = $LASTEXITCODE
    $seconds = ((Get-Date) - $started).TotalSeconds
    if ($clientExit -ne 2) { throw "Expected RPC failure exit 2; got $clientExit" }
    if ($seconds -gt 30) { throw "Negative test exceeded 30 seconds: $seconds" }
    "PASS unavailable server: exit=$clientExit elapsed=$seconds seconds" | Tee-Object "$runDir/negative-result.log"
} finally {
    docker compose up -d --wait --wait-timeout 120 server
    $restoreExit = $LASTEXITCODE
    docker compose logs --no-color server 2>&1 | Set-Content "$runDir/server.log" -Encoding utf8
    docker compose ps --all | Set-Content "$runDir/containers.log" -Encoding utf8
    if ($restoreExit -ne 0) { throw 'Could not restore healthy server' }
}
"PASS: normal, boundary, and unavailable-server tests. Evidence: $runDir" | Tee-Object "$runDir/result.log"
