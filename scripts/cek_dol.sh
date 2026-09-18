#!/bin/bash
# ============================================================
#  DIAGNOSTIK DOL BENGKULU (jalankan di MacBook yang bermasalah)
#  Cara pakai:
#    1. Simpan file ini di Desktop MacBook
#    2. Buka Terminal, jalankan:
#         bash ~/Desktop/cek_dol.sh
#    3. Salin seluruh hasilnya dan kirim kembali
# ============================================================
GOLD="\033[1;33m"; GREEN="\033[0;32m"; RED="\033[0;31m"; NC="\033[0m"
echo "${GOLD}================ DIAGNOSTIK DOL BENGKULU ================${NC}"

echo; echo "${GOLD}--- [1] Arsitektur CPU Mac ini:${NC}"
uname -m
sysctl -n machdep.cpu.brand_string 2>/dev/null

echo; echo "${GOLD}--- [2] Rosetta 2 terpasang?${NC}"
if /usr/bin/pgrep -q oahd; then echo "Rosetta 2: TERPASANG (terpakai oleh proses Intel)"; else echo "Rosetta 2: tidak terpasang"; fi

echo; echo "${GOLD}--- [3] Bundle DOL yang ditemukan:${NC}"
for B in "/Library/Audio/Plug-Ins/VST3/DOL BENGKULU.vst3" "$HOME/Library/Audio/Plug-Ins/VST3/DOL BENGKULU.vst3" "/Library/Audio/Plug-Ins/VST3/DOL BENGKULU PROTOTYPE"; do
  if [ -e "$B" ]; then
    echo "ADA     : $B"
    if [ -r "$B" ]; then
      BIN=$(find "$B" -type f -perm +111 -name "DOL*" -path "*MacOS*" 2>/dev/null | head -1)
      [ -n "$BIN" ] && echo "   arsitektur: $(lipo -archs "$BIN" 2>/dev/null) | ukuran: $(du -h "$BIN" | cut -f1)"
      echo "   wav asli : $(find "$B" -name '*.wav' ! -name '._*' 2>/dev/null | wc -l | tr -d ' ')"
      echo "   sidecar _: $(find "$B" -name '._*' 2>/dev/null | wc -l | tr -d ' ')"
    else
      echo "   ${RED}TIDAK BISA DIBACA (izin tertutup) - inilah masalahnya${NC}"
    fi
  else
    echo "tidak ada: $B"
  fi
done

echo; echo "${GOLD}--- [4] Izin bundle terpasang (path yg ditemukan di atas):${NC}"
ls -le "/Library/Audio/Plug-Ins/VST3/" 2>/dev/null | grep -i dol
ls -le "$HOME/Library/Audio/Plug-Ins/VST3/" 2>/dev/null | grep -i dol

echo; echo "${GOLD}--- [5] Proses DAW yang berjalan sekarang + arsitekturnya:${NC}"
ps -axo comm | grep -vi grep | grep -Ei "ableton|logic|reaper|cubase|bitwig|studio one|fl studio|garageband|mainstage|reason|digital performer|tracktion|waveform|lmms" | while read -r P; do
  ARCH=$(ps -o arch= -p "$(pgrep -x "$(basename "$P")" | head -1)" 2>/dev/null || echo "?")
  echo "$P  [arsitektur proses: $ARCH]"
done
echo "(kalau kosong: buka DAW dulu, lalu jalankan script ini lagi)"

echo; echo "${GOLD}--- [6] Log crash/error terkait DOL (7 hari terakhir):${NC}"
log show --last 7d --predicate 'eventMessage CONTAINS "DOL" OR process == "DOL BENGKULU"' 2>/dev/null | grep -iE "dol|error|crash" | tail -15
echo "(kosong = tidak ada error tercatat)"

echo; echo "${GOLD}================ SELESAI - salin semua teks di atas ================${NC}"
