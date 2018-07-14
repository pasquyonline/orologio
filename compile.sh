#!/bin/bash

echo "Remove old app..."
rm orologio
echo "Compile..."
gcc -v -o orologio orologio.cpp OraBigFont.cpp JPFont.cpp LCD240x128.cpp RPI.cpp -L/usr/local/lib -lbcm2835 -lstdc++ -lrt -O `GraphicsMagick-config --cppflags --ldflags --libs`
echo "Launch app..."
sudo ./orologio test/kanji.png
