; Inno Setup installer for DOL BENGKULU (VST3, Windows 64-bit)
;
; Installs the plugin to the standard VST3 location:
;   C:\Program Files\Common Files\VST3\DOL BENGKULU.vst3
;
; Cara tercepat (build + installer otomatis):
;   scripts\build_windows.bat
;
; Atau manual:
;   1. Build VST3 di Windows dulu:
;        cmake -S . -B Builds\DOL_BKL -DDOL_JUCE_ROOT=C:\path\to\JUCE -DDOL_COPY_AFTER_BUILD=OFF
;        cmake --build Builds\DOL_BKL --config Release --target DOL_BKL_VST3
;   2. Install Inno Setup 6 (https://jrsoftware.org/isinfo.php)
;   3. Compile script ini:
;        "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" installer\DOL_BKL_installer.iss
;
; Output: dist\DOL_BKL-0.1.0-win64.exe
;
; Catatan: {commoncf64} butuh Inno Setup 6.1+ (versi saat ini 6.4).

#define MyAppName "DOL BENGKULU"
#define MyAppVersion "0.1.0"
#define MyAppPublisher "Digiethnica"
#define MyAppBundleName "DOL BENGKULU.vst3"
; Path relatif terhadap file .iss ini (folder installer\)
#define Vst3Dir "..\Builds\DOL_BKL\DOL_BKL_artefacts\VST3"

[Setup]
AppId={{E7B1A2D0-4C3B-4A9F-9E2D-7B6C5A4D3E2F}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL=https://digiethnica.com
DefaultDirName={autopf}\DOL BKL
DefaultGroupName=DOL BKL
DisableProgramGroupPage=yes
OutputDir=..\dist
OutputBaseFilename=DOL_BKL-{#MyAppVersion}-win64
Compression=lzma2
SolidCompression=yes
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
WizardStyle=modern
UninstallDisplayName={#MyAppName} {#MyAppVersion}

[Files]
; Bundle VST3 (folder berisi Contents\...) -> folder VST3 standar Windows
Source: "{#Vst3Dir}\{#MyAppBundleName}\*"; DestDir: "{commoncf64}\VST3\{#MyAppBundleName}"; Flags: recursesubdirs createallsubdirs ignoreversion

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf64}\VST3\{#MyAppBundleName}"
