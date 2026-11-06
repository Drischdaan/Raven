#Requires -Version 5.1
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$RepoRoot = $PSScriptRoot
$HooksDir = '.githooks'

function Invoke-Git {
    param([Parameter(ValueFromRemainingArguments)] [string[]] $Arguments)
    $output = & git -C $RepoRoot @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "git $($Arguments -join ' ') failed with exit code $LASTEXITCODE"
    }
    return $output
}

try {
    if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
        throw 'Git is not installed or not on PATH. Install it from https://git-scm.com'
    }

    $null = Invoke-Git rev-parse --is-inside-work-tree

    $hooksPath = Join-Path $RepoRoot $HooksDir
    if (-not (Test-Path $hooksPath -PathType Container)) {
        throw "Hooks directory not found: $hooksPath"
    }

    Invoke-Git config core.hooksPath $HooksDir
    $configured = Invoke-Git config --get core.hooksPath

    Write-Host "Git hooks configured: core.hooksPath = $configured" -ForegroundColor Green

    if (-not (Get-Command xmake -ErrorAction SilentlyContinue)) {
        throw 'xmake is not installed or not on PATH. Install it from https://xmake.io'
    }

    Push-Location $RepoRoot
    try {
        & xmake project -k vsxmake2026 ./Engine/Build/
        if ($LASTEXITCODE -ne 0) {
            throw "xmake project generation failed with exit code $LASTEXITCODE"
        }
    }
    finally {
        Pop-Location
    }

    Write-Host 'Visual Studio project generated in Engine/Build/' -ForegroundColor Green
}
catch {
    Write-Host "Setup failed: $_" -ForegroundColor Red
    exit 1
}

