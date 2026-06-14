param(
    [string]$Version = "2.32.10",
    [string]$ProjectRoot = (Split-Path -Parent $PSScriptRoot)
)

$ErrorActionPreference = "Stop"

$deps = Join-Path $ProjectRoot ".deps"
$target = Join-Path $deps "SDL2-$Version"
$mingwRoot = Join-Path $target "x86_64-w64-mingw32"
$dll = Join-Path $mingwRoot "bin\SDL2.dll"

if (Test-Path -LiteralPath $dll) {
    Write-Host "SDL2 $Version already installed at $mingwRoot"
    exit 0
}

New-Item -ItemType Directory -Force -Path $deps | Out-Null

$archive = Join-Path $deps "SDL2-devel-$Version-mingw.zip"
$url = "https://www.libsdl.org/release/SDL2-devel-$Version-mingw.zip"

Write-Host "Downloading $url"
Invoke-WebRequest -Uri $url -OutFile $archive

Write-Host "Extracting $archive"
Expand-Archive -LiteralPath $archive -DestinationPath $deps -Force

if (!(Test-Path -LiteralPath $dll)) {
    throw "SDL2 extraction did not produce $dll"
}

Write-Host "SDL2 $Version ready at $mingwRoot"
Write-Host "Run: pio run -e native"
