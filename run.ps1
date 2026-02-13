#!/usr/bin/env pwsh

# MQQStatusTool Run Script for Windows (PowerShell)
# Runs the compiled executable with specified parameters

param(
    [Parameter(ValueFromRemainingArguments=$true)]
    [string[]]$Arguments,

    [switch]$Help
)

# Configuration
$ProjectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$BuildDir = Join-Path $ProjectRoot "build"
$Exe = Join-Path $BuildDir "MQQStatusTool.exe"
$ConfigFile = Join-Path $ProjectRoot "config.toml"

function Show-Help {
    Write-Host @"
MQQStatusTool Run Script
========================

Usage: .\run.ps1 [OPTIONS]

The script automatically passes --config config.toml.
You must provide at least --qm <name> to specify which queue manager to use.

Options (passed to the executable):
  --qm <name>         Queue manager section name (must exist in config)
  --queue <name>      Queue name for GET/PUT operations
  --status            Get status of all local queues (default)
  --get               Get messages from specified queue
  --put               Put test message to specified queue
  --input-file <file> Text file with queue manager names for batch processing
  --help              Show executable help message

Examples:
  .\run.ps1 --qm default --status
  .\run.ps1 --qm default --queue APP1.REQ --get
  .\run.ps1 --qm default --queue APP1.REQ --put
  .\run.ps1 --qm MQQM1 --status

"@
}

function Test-Executable {
    if (-not (Test-Path $Exe)) {
        Write-Host "ERROR: Executable not found: $Exe" -ForegroundColor Red
        Write-Host "Run '.\build.ps1' to build the project first" -ForegroundColor Yellow
        return $false
    }
    return $true
}

# Main script
if ($Help -or ($Arguments.Count -eq 1 -and $Arguments[0] -eq "--help")) {
    Show-Help
    exit 0
}

Write-Host "MQQStatusTool Runner" -ForegroundColor Cyan
Write-Host "==================" -ForegroundColor Cyan
Write-Host ""

if (-not (Test-Executable)) {
    exit 1
}

Write-Host "Executable: $Exe" -ForegroundColor Green
Write-Host "Config: $ConfigFile" -ForegroundColor Green
Write-Host ""

# Prepare arguments - use a different variable name to avoid conflict with $args
$exeArgs = @("--config", $ConfigFile)

# If arguments provided, append them
if ($Arguments.Count -gt 0) {
    $exeArgs += $Arguments
}

Write-Host "Running: $Exe $($exeArgs -join ' ')" -ForegroundColor Yellow
Write-Host ""

# Run the executable
& $Exe @exeArgs

$exitCode = $LASTEXITCODE
Write-Host ""
if ($exitCode -eq 0) {
    Write-Host "Execution completed successfully" -ForegroundColor Green
} else {
    Write-Host "Execution failed with exit code: $exitCode" -ForegroundColor Red
}

exit $exitCode
