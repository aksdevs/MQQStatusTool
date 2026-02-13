#!/usr/bin/env pwsh

# MQQStatusTool Build Script for Windows (PowerShell)
# Uses GCC/MinGW 64-bit with CMake

param(
    [ValidateSet("Release", "Debug")]
    [string]$Configuration = "Release",

    [switch]$Clean,
    [switch]$Help
)

# Configuration
$ProjectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$BuildDir = Join-Path $ProjectRoot "build"
$MQInstallPath = "C:\Program Files\IBM\MQ"
$MQToolsPath = "C:\Program Files\IBM\MQ\tools"
$GccPath = "C:\mingw64\mingw64"

function Show-Help {
    Write-Host @"
MQQStatusTool Build Script
==========================

Usage: .\build.ps1 [OPTIONS]

Options:
  -Configuration <Release|Debug>  Build configuration (default: Release)
  -Clean                          Clean build artifacts before building
  -Help                           Show this help message

Examples:
  .\build.ps1                           # Build in Release mode
  .\build.ps1 -Configuration Debug      # Build in Debug mode
  .\build.ps1 -Clean                    # Clean and rebuild

"@
}

function Test-Prerequisites {
    Write-Host "Checking prerequisites..." -ForegroundColor Cyan

    # Check GCC
    $gccExe = Join-Path $GccPath "bin\g++.exe"
    if (-not (Test-Path $gccExe)) {
        Write-Host "ERROR: GCC not found at $gccExe" -ForegroundColor Red
        return $false
    }
    Write-Host "OK: GCC found" -ForegroundColor Green

    # Check CMake
    $cmake = Get-Command cmake -ErrorAction SilentlyContinue
    if (-not $cmake) {
        Write-Host "ERROR: CMake not found" -ForegroundColor Red
        return $false
    }
    Write-Host "OK: CMake found" -ForegroundColor Green

    # Check IBM MQ installation
    if (-not (Test-Path $MQToolsPath)) {
        Write-Host "ERROR: IBM MQ tools not found" -ForegroundColor Red
        return $false
    }
    Write-Host "OK: IBM MQ tools found" -ForegroundColor Green

    return $true
}

function Clean-Build {
    Write-Host "Cleaning build artifacts..." -ForegroundColor Yellow
    if (Test-Path $BuildDir) {
        Remove-Item $BuildDir -Recurse -Force
        Write-Host "OK: Cleaned" -ForegroundColor Green
    }
}

function Build-Project {
    Write-Host "Building project..." -ForegroundColor Cyan

    # Create build directory
    if (-not (Test-Path $BuildDir)) {
        New-Item -ItemType Directory -Path $BuildDir | Out-Null
    }

    Push-Location $BuildDir
    try {
        # Run CMake with MinGW Makefiles generator
        Write-Host "Running CMake..." -ForegroundColor Yellow
        # Convert paths to forward slashes for CMake
        $gccCompiler = "$GccPath\bin\g++.exe" -replace '\\', '/'
        $cCompiler = "$GccPath\bin\gcc.exe" -replace '\\', '/'
        $mqInstallPath = "$MQInstallPath" -replace '\\', '/'
        $mqToolsPath = "$MQToolsPath" -replace '\\', '/'

        & cmake -G "MinGW Makefiles" `
            -DCMAKE_BUILD_TYPE=$Configuration `
            -DCMAKE_CXX_COMPILER="$gccCompiler" `
            -DCMAKE_C_COMPILER="$cCompiler" `
            -DIBM_MQ_INSTALL_PATH="$mqInstallPath" `
            -DIBM_MQ_TOOLS_PATH="$mqToolsPath" `
            ..

        if ($LASTEXITCODE -ne 0) {
            Write-Host "ERROR: CMake failed" -ForegroundColor Red
            Pop-Location
            return $false
        }
        Write-Host "OK: CMake completed" -ForegroundColor Green

        # Build using mingw32-make
        Write-Host "Building with mingw32-make..." -ForegroundColor Yellow
        & mingw32-make

        if ($LASTEXITCODE -ne 0) {
            Write-Host "ERROR: Build failed" -ForegroundColor Red
            Pop-Location
            return $false
        }
        Write-Host "OK: Build completed" -ForegroundColor Green

        # Verify executable was created
        $exe = Join-Path $BuildDir "MQQStatusTool.exe"
        if (Test-Path $exe) {
            Write-Host "OK: Executable created" -ForegroundColor Green
            Pop-Location
            return $true
        } else {
            Write-Host "ERROR: Executable not found" -ForegroundColor Red
            Pop-Location
            return $false
        }
    }
    catch {
        Write-Host "ERROR: Exception during build: $_" -ForegroundColor Red
        Pop-Location
        return $false
    }
}

# Main script
if ($Help) {
    Show-Help
    exit 0
}

Write-Host "MQQStatusTool Build Script" -ForegroundColor Cyan
Write-Host "===========================" -ForegroundColor Cyan
Write-Host ""

if (-not (Test-Prerequisites)) {
    exit 1
}

if ($Clean) {
    Clean-Build
}

if (-not (Build-Project)) {
    exit 1
}

Write-Host ""
Write-Host "Build completed successfully!" -ForegroundColor Green
Write-Host "Run with: .\run.ps1" -ForegroundColor Cyan

