#!/bin/bash
cd /c/Users/Pietro/.gemini/antigravity/scratch/FarmLifeSimulator
echo "[BUILD] Cartella: $(pwd)"
echo "[BUILD] Compilazione..."
/ucrt64/bin/g++ main.cpp \
    -o FarmLifeSimulator.exe \
    -mwindows \
    -municode \
    -lshell32 \
    -lshlwapi \
    -O2 \
    -std=c++17
RESULT=$?
echo "[BUILD] Exit code: $RESULT"
if [ $RESULT -eq 0 ]; then
  echo "[BUILD] Successo!"
  ls -lh FarmLifeSimulator.exe
else
  echo "[BUILD] FALLITO con codice $RESULT"
fi
