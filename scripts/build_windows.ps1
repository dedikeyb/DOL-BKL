<#
.SYNOPSIS
    Build DOL BENGKULU (VST3) di Windows, lalu langsung menaruhnya di
    folder VST3 Windows — TANPA installer dan TANPA perlu admin.

.DESCRIPTION
    Script ini otomatis:
      1. Menemukan JUCE SDK (parameter -JUCE, env JUCE_ROOT, folder JUCE di
         samping script, atau lokasi umum) — jika tidak ada, MENGUNDUH
         JUCE 9.0.1 dari GitHub secara otomatis.
      2. Menemukan CMake, mengkonfigurasi & membangun VST3 Release
         (sampel otomatis terbundle di dalam .vst3).
      3. Menyalin hasil .vst3 langsung ke folder VST3 milik user:
             %LOCALAPPDATA%\Programs\Common\VST3\DOL BENGKULU.vst3
         -> langsung bisa dipakai di DAW, tanpa installer, tanpa admin.
      4. (Opsional) Jika Inno Setup terpasang, sekalian membuat installer
         EXE: dist\DOL_BKL-0.1.0-win64.exe

.EXAMPLE
    .\scripts\build_windows.ps1
    .\scripts\build_windows.ps1 -JUCE C:\Development\JUCE
    .\scripts\build_windows.ps1 -SkipInstaller
#>
[CmdletBinding()]
param(
    [string]$JUCE,
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Release",
    [switch]$SkipInstaller
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
Set-Location $Root

Write-Host "==> DOL BENGKULU - Windows build (tanpa installer)" -ForegroundColor Cyan

# ---------- 1. Cari / unduh JUCE SDK ----------
if (-not $JUCE -and $env:JUCE_ROOT) { $JUCE = $env:JUCE_ROOT }
if (-not $JUCE) {
    foreach ($cand in @(
            "$Root\JUCE", "C:\JUCE", "C:\Development\JUCE", "C:\dev\JUCE",
            "D:\JUCE", "$env:USERPROFILE\JUCE",
            "$env:USERPROFILE\Development\JUCE"
        )) {
        if (Test-Path (Join-Path $cand "CMakeLists.txt")) { $JUCE = $cand; break }
    }
}
if (-not $JUCE -or -not (Test-Path (Join-Path $JUCE "CMakeLists.txt"))) {
    $JUCE = Join-Path $Root "JUCE"
    if (-not (Test-Path (Join-Path $JUCE "CMakeLists.txt"))) {
        Write-Host "JUCE SDK tidak ditemukan -> mengunduh JUCE 9.0.1..." -ForegroundColor Yellow
        try {
            $Zip = Join-Path $env:TEMP "JUCE-9.0.1.zip"
            $Url = "https://github.com/juce-framework/JUCE/archive/refs/tags/9.0.1.zip"
            [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
            Invoke-WebRequest -Uri $Url -OutFile $Zip -UseBasicParsing
            $Tmp = Join-Path $env:TEMP "JUCE-9.0.1-src"
            if (Test-Path $Tmp) { Remove-Item $Tmp -Recurse -Force }
            Expand-Archive -Path $Zip -DestinationPath $Tmp
            $Src = Get-ChildItem $Tmp -Directory | Select-Object -First 1
            if (-not $Src) { throw "Isi arsip JUCE tidak ditemukan" }
            Move-Item $Src.FullName $JUCE
            Remove-Item $Tmp -Recurse -Force
            Remove-Item $Zip -Force
            Write-Host "JUCE 9.0.1 terunduh ke: $JUCE" -ForegroundColor Green
        } catch {
            Write-Host "Gagal mengunduh JUCE otomatis: $_" -ForegroundColor Red
            Write-Host "Unduh manual dari https://juce.com/download/ ," -ForegroundColor Yellow
            Write-Host "ekstrak, lalu jalankan:" -ForegroundColor Yellow
            Write-Host "    .\scripts\build_windows.ps1 -JUCE C:\path\ke\JUCE" -ForegroundColor Yellow
            exit 1
        }
    }
}
Write-Host "JUCE SDK : $JUCE"

# ---------- 2. Cari CMake ----------
$cmake = (Get-Command cmake -ErrorAction SilentlyContinue).Source
if (-not $cmake) {
    Write-Host "cmake tidak ditemukan. Install dulu (copy-paste di PowerShell admin):" -ForegroundColor Red
    Write-Host "    winget install -e --id Kitware.CMake" -ForegroundColor Yellow
    exit 1
}
Write-Host "CMake    : $cmake"

# ---------- 3. Configure + build ----------
$BuildDir = Join-Path $Root "Builds\DOL_BKL"
Write-Host "==> Configure CMake..." -ForegroundColor Cyan
& cmake -S $Root -B $BuildDir -DDOL_JUCE_ROOT=$JUCE -DDOL_COPY_AFTER_BUILD=OFF
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$Jobs = if ($env:NUMBER_OF_PROCESSORS) { [int]$env:NUMBER_OF_PROCESSORS } else { 8 }
Write-Host "==> Build VST3 ($Config, -j $Jobs)... (bisa beberapa menit)" -ForegroundColor Cyan
& cmake --build $BuildDir --config $Config --target DOL_BKL_VST3 --parallel $Jobs
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

# ---------- 4. Verifikasi bundle ----------
$Bundle = Join-Path $Root "Builds\DOL_BKL\DOL_BKL_artefacts\VST3\DOL BENGKULU.vst3"
if (-not (Test-Path $Bundle)) {
    Write-Host "Bundle VST3 tidak ditemukan: $Bundle" -ForegroundColor Red
    exit 1
}
$WavCount = (Get-ChildItem -Path $Bundle -Recurse -Filter *.wav -ErrorAction SilentlyContinue).Count
Write-Host "Bundle   : $Bundle ($WavCount sampel terbundle)" -ForegroundColor Green

# ---------- 5. Salin langsung ke folder VST3 user (tanpa admin) ----------
$UserVst3 = Join-Path $env:LOCALAPPDATA "Programs\Common\VST3"
New-Item -ItemType Directory -Force -Path $UserVst3 | Out-Null
$Target = Join-Path $UserVst3 "DOL BENGKULU.vst3"
if (Test-Path $Target) { Remove-Item $Target -Recurse -Force }
Copy-Item -Path $Bundle -Destination $Target -Recurse
Write-Host "" -ForegroundColor Cyan
Write-Host "SELESAI! Plugin siap dipakai di DAW:" -ForegroundColor Green
Write-Host "    $Target" -ForegroundColor Green
Write-Host "Buka ulang DAW -> cari di folder vendor 'Digiethnica' -> 'DOL BENGKULU'." -ForegroundColor Green

# ---------- 6. (Opsional) Installer EXE ----------
if (-not $SkipInstaller) {
    $iscc = $null
    foreach ($cand in @(
            "C:\Program Files (x86)\Inno Setup 6\ISCC.exe",
            "C:\Program Files\Inno Setup 6\ISCC.exe",
            (Get-Command ISCC.exe -ErrorAction SilentlyContinue).Source
        )) {
        if ($cand -and (Test-Path $cand)) { $iscc = $cand; break }
    }
    if ($iscc) {
        Write-Host "==> Inno Setup ditemukan - membuat installer EXE juga..." -ForegroundColor Cyan
        & $iscc (Join-Path $Root "installer\DOL_BKL_installer.iss")
        if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
        $Exe = Join-Path $Root "dist\DOL_BKL-0.1.0-win64.exe"
        if (Test-Path $Exe) {
            $Size = [math]::Round((Get-Item $Exe).Length / 1MB, 1)
            Write-Host "Installer EXE : $Exe ($Size MB)" -ForegroundColor Green
        }
    } else {
        Write-Host "(Inno Setup tidak terpasang - lewati installer EXE. Plugin .vst3 sudah siap dipakai.)" -ForegroundColor Yellow
    }
}

Write-Host "" -ForegroundColor Cyan
Write-Host "Tips menyebarkan ke user lain (tanpa installer):" -ForegroundColor Green
Write-Host "    Copy folder 'DOL BENGKULU.vst3' ke PC mereka, lalu paste ke:" -ForegroundColor Green
Write-Host "    %LOCALAPPDATA%\Programs\Common\VST3\  (tanpa admin) atau" -ForegroundColor Green
Write-Host "    C:\Program Files\Common Files\VST3\   (butuh admin, lebih standar)" -ForegroundColor Green
