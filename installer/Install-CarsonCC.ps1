[CmdletBinding()]
param(
    [Parameter()]
    [string]$InstallDir
)

$ErrorActionPreference = "Stop"

$scriptDir = (Resolve-Path -LiteralPath $PSScriptRoot).Path
$sourceDir = $scriptDir
if (-not (Test-Path -LiteralPath (Join-Path $sourceDir "carsoncc.exe") -PathType Leaf)) {
    $repositoryRoot = Split-Path -Parent $scriptDir
    if (Test-Path -LiteralPath (Join-Path $repositoryRoot "carsoncc.exe") -PathType Leaf) {
        $sourceDir = $repositoryRoot
    }
    else {
        throw "carsoncc.exe was not found beside this script. Extract the complete CarsonCC Windows ZIP, then run Install-CarsonCC.ps1 from that folder."
    }
}

if ([string]::IsNullOrWhiteSpace($InstallDir)) {
    $InstallDir = Join-Path $sourceDir "CarsonCC-Portable"
}

$destination = [System.IO.Path]::GetFullPath($InstallDir)

if ([string]::Equals($sourceDir.TrimEnd('\'), $destination.TrimEnd('\'), [System.StringComparison]::OrdinalIgnoreCase)) {
    throw "The install folder cannot be the same as the source folder. Choose a separate folder with -InstallDir."
}

New-Item -ItemType Directory -Path $destination -Force | Out-Null

Copy-Item -LiteralPath (Join-Path $sourceDir "carsoncc.exe") -Destination $destination -Force
$readme = Join-Path $sourceDir "README.md"
if (Test-Path -LiteralPath $readme -PathType Leaf) {
    Copy-Item -LiteralPath $readme -Destination $destination -Force
}

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
