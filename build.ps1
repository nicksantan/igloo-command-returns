# Build script for Igloo Command Returns (PowerShell version)
# Sets up SGDK environment and builds the ROM

$env:GDK = "C:\sgdk"
$env:PATH = "C:\sgdk\bin;C:\Program Files\Java\jre1.8.0_471\bin;$env:PATH"

Write-Host "Building Igloo Command Returns..." -ForegroundColor Green

# Run make with any additional arguments passed to this script
& "C:\sgdk\bin\make.exe" -f "C:\sgdk\makefile.gen" @args
