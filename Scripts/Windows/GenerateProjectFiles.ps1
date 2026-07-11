#Requires -Version 5.1
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false

$root = Resolve-Path (Join-Path $PSScriptRoot '..\..')
Push-Location $root
try {
    $vsxmake = 'Engine\Build\Output\vsxmake2026'
    $vsxTemp = 'Engine\Build\Output\vsxmake2026-temp'

    function Restore-Backup {
        if (Test-Path $vsxTemp) {
            if (Test-Path $vsxmake) { Remove-Item -Recurse -Force $vsxmake }
            Copy-Item -Recurse -Force $vsxTemp $vsxmake
            Remove-Item -Recurse -Force $vsxTemp
        }
    }

    if (Test-Path $vsxmake) {
        if (Test-Path $vsxTemp) { Remove-Item -Recurse -Force $vsxTemp }
        Copy-Item -Recurse -Force $vsxmake $vsxTemp
        Remove-Item -Recurse -Force $vsxmake
    }

    if (Test-Path '.\Raven.slnx') {
        Remove-Item -Force '.\Raven.slnx'
    }

    xmake project -y -k vsxmake2026 -m 'debug,development,shipping' .\Engine\Build\Output
    if ($LASTEXITCODE -ne 0) {
        Write-Host '[ERROR] Failed to generate project files'
        Restore-Backup
        exit 1
    }

    dotnet sln .\Engine\Build\Output\vsxmake2026\Raven.sln migrate *> $null
    if ($LASTEXITCODE -eq 0) {
        Write-Host '[INFO] Migrated solution file to .slnx format'
    } else {
        Write-Host '[ERROR] Failed to migrate solution file'
        Restore-Backup
    }

    if (Test-Path $vsxTemp) {
        Remove-Item -Recurse -Force $vsxTemp
    }

    $slnxSource = '.\Engine\Build\Output\vsxmake2026\Raven.slnx'
    if (Test-Path $slnxSource) {
        (Get-Content -Raw $slnxSource) `
            -replace 'Path="', 'Path="Engine/Build/Output/vsxmake2026/' |
            Set-Content -NoNewline '.\Raven.slnx'
        Remove-Item -Force $slnxSource
    }
}
finally {
    Pop-Location
}
