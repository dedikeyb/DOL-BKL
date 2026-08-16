# DOL BENGKULU (VST3)

Plugin sampler VST3 untuk *dol* — alat musik tradisional Bengkulu.
JUCE 9.0.1 · C++17 · CMake ≥ 3.22 · Format: VST3 (macOS & Windows 64-bit).

## Struktur

```
Source/            Kode plugin (Processor, SampleManager, DOLVoice, Editor)
Samples/V1/        Sampel (Head: V01-V08 x RR01-05; Edge: V01-V03 x RR01-05)
ThirdParty/        foleys_gui_magic
Builds/DOL_BKL/    Direktori build aktif
scripts/           Script packaging (macOS DMG/PKG, Windows build one-click)
installer/         Script installer Windows (Inno Setup .iss)
```

## Build — macOS

```bash
# JUCE dicari di DOL_JUCE_ROOT (default: /Volumes/SSD Ex 1 Tb/Development/JUCE)
cmake -S . -B Builds/DOL_BKL
cmake --build Builds/DOL_BKL --target DOL_BKL_VST3 -j 8
```

Hasil: `Builds/DOL_BKL/DOL_BKL_artefacts/VST3/DOL BENGKULU.vst3`
`COPY_PLUGIN_AFTER_BUILD` otomatis menyalin bundle ke `~/Library/Audio/Plug-Ins/VST3/`.

## Build — Windows (64-bit)

Prasyarat: Visual Studio (C++), CMake ≥ 3.22, JUCE 9.0.1, Inno Setup 6.1+.

Cara tercepat — **tanpa installer**: script membangun `.vst3` lalu
langsung menaruhnya di folder VST3 user (siap pakai di DAW, tanpa admin):

```bat
scripts\build_windows.bat
```

Script otomatis:
- menemukan JUCE (atau **mengunduh** JUCE 9.0.1 dari GitHub, sekali saja),
- membangun VST3 Release (sampel terbundle), lalu
- menyalin `DOL BENGKULU.vst3` ke `%LOCALAPPDATA%\Programs\Common\VST3\`

Jika Inno Setup terpasang, script sekalian membuat `dist\DOL_BKL-0.1.0-win64.exe`
(bisa dilewati dengan `-SkipInstaller`). JUCE dapat ditunjuk eksplisit:

```bat
scripts\build_windows.ps1 -JUCE C:\path\to\JUCE
```

Folder portabel siap-copy (semua source + script + instruksi, tinggal
jalankan `build_windows.bat` di Windows) tersedia di:
`dist/DOL_BKL_Windows_Build/` (+ `dist/DOL_BKL_Windows_Build.zip`).

Hasil `.vst3` adalah folder portabel — untuk distribusi ke user lain cukup
copy folder `DOL BENGKULU.vst3` ke `C:\Program Files\Common Files\VST3\`
atau `%LOCALAPPDATA%\Programs\Common\VST3\` di PC mereka.

Build manual (tanpa installer):

```bat
cmake -S . -B Builds\DOL_BKL -DDOL_JUCE_ROOT=C:\path\to\JUCE -DDOL_COPY_AFTER_BUILD=OFF
cmake --build Builds\DOL_BKL --config Release --target DOL_BKL_VST3
```

Hasil (Windows): `Builds\DOL_BKL\DOL_BKL_artefacts\Release\VST3\DOL BENGKULU.vst3`
(salin manual ke `C:\Program Files\Common Files\VST3\` untuk uji coba tanpa installer).

> `DOL_COPY_AFTER_BUILD=OFF` dimatikan saat packaging agar build tidak
> butuh terminal admin. Di macOS default `ON` (salin otomatis ke
> `~/Library/Audio/Plug-Ins/VST3/` untuk kenyamanan develop).

## Sampel

Sampel otomatis **dibundle ke dalam** bundle VST3 saat build:

```
<bundle>.vst3/Contents/Resources/Samples/
```

Saat runtime, plugin mencari sampel relatif terhadap lokasi binary plugin
(bekerja di macOS & Windows). `DOL_SAMPLE_PATH` (path folder dev) hanya dipakai
sebagai fallback.

## Installer

### macOS — DMG

```bash
./scripts/make_dmg.sh
```

Menghasilkan `dist/DOL_BKL-0.1.0-macos.dmg` berisi `.vst3` + petunjuk
instalasi (salin ke `~/Library/Audio/Plug-Ins/VST3/`).

### Windows — EXE (Inno Setup)

**Satu perintah (build + installer):**

```bat
scripts\build_windows.bat
```

Menghasilkan `dist\DOL_BKL-0.1.0-win64.exe`. Installernya memasang plugin
langsung ke lokasi default VST3 Windows:
`C:\Program Files\Common Files\VST3\DOL BENGKULU.vst3`
(termasuk uninstaller, lewat "Add or remove programs").

Manual (opsional, tanpa script):

```bat
cmake -S . -B Builds\DOL_BKL -DDOL_JUCE_ROOT=C:\path\to\JUCE -DDOL_COPY_AFTER_BUILD=OFF
cmake --build Builds\DOL_BKL --config Release --target DOL_BKL_VST3
"C:\Program Files (x86)\Inno Setup 6\ISCC.exe" installer\DOL_BKL_installer.iss
```

## Penandatanganan (code signing)

- **macOS:** build lokal di-sign ad-hoc. Tanpa Developer ID + notarisasi,
  DAW/gatekeeper bisa menampilkan peringatan sekali (klik kanan → Open).
  Untuk distribusi publik, tanda tangani dengan Developer ID & notarisasi.
- **Windows:** tanpa sertifikat, SmartScreen menampilkan peringatan
  "unknown publisher". Untuk rilis publik, tanda tangani dengan
  sertifikat code-signing.
