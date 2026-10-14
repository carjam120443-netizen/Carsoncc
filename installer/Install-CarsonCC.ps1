[CmdletBinding()]
param(
    [Parameter()]
    [string]$InstallDir = (Join-Path $PSScriptRoot "CarsonCC-Portable")
)

$ErrorActionPreference = "Stop"

$sourceDir = (Resolve-Path -LiteralPath $PSScriptRoot).Path
$destination = [System.IO.Path]::GetFullPath($InstallDir)

if (-not (Test-Path -LiteralPath (Join-Path $sourceDir "carsoncc.exe") -PathType Leaf)) {
    throw "carsoncc.exe was not found beside this script. Extract the complete CarsonCC Windows ZIP, then run Install-CarsonCC.ps1 from that folder."
}

if ([string]::Equals($sourceDir.TrimEnd('\'), $destination.TrimEnd('\'), [System.StringComparison]::OrdinalIgnoreCase)) {
    throw "The install folder cannot be the same as the ZIP's extracted folder. Choose a separate folder with -InstallDir."
}

New-Item -ItemType Directory -Path $destination -Force | Out-Null

Copy-Item -LiteralPath (Join-Path $sourceDir "carsoncc.exe") -Destination $destination -Force
Copy-Item -LiteralPath (Join-Path $sourceDir "README.md") -Destination $destination -Force

foreach ($directory in @("examples", "osappreleasepackages")) {
    $sourcePath = Join-Path $sourceDir $directory
    if (Test-Path -LiteralPath $sourcePath -PathType Container) {
        Copy-Item -LiteralPath $sourcePath -Destination $destination -Recurse -Force
    }
}

Write-Host ""
Write-Host "CarsonCC portable install complete!" -ForegroundColor Green
Write-Host "Folder: $destination"
Write-Host ""
Write-Host "Try it from PowerShell:"
Write-Host ('  & "' + (Join-Path $destination "carsoncc.exe") + '" "' + (Join-Path $destination "examples\hello.car") + '" -S -o "' + (Join-Path $destination "hello.asm") + '"')
Write-Host ""
Write-Host "No administrator rights were used, and your PATH was not changed."
Write-Host "You can move the installed folder or keep it on removable storage."
