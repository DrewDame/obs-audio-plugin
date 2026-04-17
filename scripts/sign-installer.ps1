# Signs the NSIS installer (or any PE file) with Authenticode.
# Smart App Control / SmartScreen trust unsigned installers; signing with a
# valid code-signing certificate is what allows install without disabling security.
#
# Prerequisites:
#   - Windows SDK (signtool.exe), or path to signtool via -SigntoolPath
#   - A code-signing certificate (.pfx) from a public CA, or your org's cert
#
# Usage (PowerShell, from repo root):
#   $env:OBS_AUDIO_SIGN_PFX = "C:\path\cert.pfx"
#   $env:OBS_AUDIO_SIGN_PASSWORD = "secret"
#   .\scripts\sign-installer.ps1 -Path ".\build\obs-audio-recorder-1.0.0-windows-x64.exe"
#
# Optional: set OBS_AUDIO_TIMESTAMP_URL (default: DigiCert RFC3161)

param(
    [Parameter(Mandatory = $true)]
    [string] $Path,
    [string] $CertificatePath = $env:OBS_AUDIO_SIGN_PFX,
    [string] $CertificatePassword = $env:OBS_AUDIO_SIGN_PASSWORD,
    [string] $TimestampUrl = $(if ($env:OBS_AUDIO_TIMESTAMP_URL) { $env:OBS_AUDIO_TIMESTAMP_URL } else { "http://timestamp.digicert.com" }),
    [string] $SigntoolPath = ""
)

$ErrorActionPreference = "Stop"

if (-not (Test-Path -LiteralPath $Path)) {
    throw "File not found: $Path"
}
if (-not $CertificatePath -or -not (Test-Path -LiteralPath $CertificatePath)) {
    throw "Set OBS_AUDIO_SIGN_PFX to your .pfx path, or pass -CertificatePath."
}

function Find-Signtool {
    $roots = @(
        "${env:ProgramFiles(x86)}\Windows Kits\10\bin",
        "${env:ProgramFiles}\Windows Kits\10\bin"
    )
    foreach ($root in $roots) {
        if (-not (Test-Path $root)) { continue }
        $candidates = Get-ChildItem -Path $root -Recurse -Filter "signtool.exe" -ErrorAction SilentlyContinue |
            Where-Object { $_.FullName -match "\\x64\\signtool\.exe$" }
        if ($candidates) {
            return ($candidates | Sort-Object FullName -Descending | Select-Object -First 1).FullName
        }
    }
    return $null
}

if (-not $SigntoolPath) {
    $SigntoolPath = Find-Signtool
}
if (-not $SigntoolPath -or -not (Test-Path -LiteralPath $SigntoolPath)) {
    throw "signtool.exe not found. Install the Windows SDK or pass -SigntoolPath."
}

$signArgs = @(
    "sign",
    "/fd", "SHA256",
    "/f", $CertificatePath
)
if ($CertificatePassword) {
    $signArgs += "/p"
    $signArgs += $CertificatePassword
}
$signArgs += @("/tr", $TimestampUrl, "/td", "SHA256", $Path)

Write-Host "Signing: $Path"
Write-Host "Using:   $SigntoolPath"

& $SigntoolPath @signArgs

if ($LASTEXITCODE -ne 0) {
    throw "signtool failed with exit code $LASTEXITCODE"
}
Write-Host "Done."
