#Requires -Version 5.1
$ErrorActionPreference = 'Stop'

& (Join-Path $PSScriptRoot 'Scripts\Windows\GenerateProjectFiles.ps1')
exit $LASTEXITCODE
