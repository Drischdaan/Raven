Push-Location $PSScriptRoot\..

act push -W .github/workflows/ci.yml -P windows-2025-vs2026=-self-hosted --artifact-server-path .act-artifacts --matrix mode:debug
if ($LASTEXITCODE -ne 0) {
  Write-Host "Debug build failed with exit code $LASTEXITCODE" -ForegroundColor Red
  exit 1
}

act push -W .github/workflows/ci.yml -P windows-2025-vs2026=-self-hosted --artifact-server-path .act-artifacts --matrix mode:development
if ($LASTEXITCODE -ne 0) {
  Write-Host "Development build failed with exit code $LASTEXITCODE" -ForegroundColor Red
  exit 1
}

act push -W .github/workflows/ci.yml -P windows-2025-vs2026=-self-hosted --artifact-server-path .act-artifacts --matrix mode:shipping
if ($LASTEXITCODE -ne 0) {
  Write-Host "Shipping build failed with exit code $LASTEXITCODE" -ForegroundColor Red
  exit 1
}

Pop-Location