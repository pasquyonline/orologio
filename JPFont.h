//
// Created by pasquale on 08/07/2018.
//

#ifndef OROLOGIO_JPFONT_H
#define OROLOGIO_JPFONT_H

#include <stdint.h>
#include <cstring>
#include <string>
#include <cstdio>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <math.h>
#include <magick/api.h>

using namespace std;


class JPChar {
public:
    uint16_t code;
    int width;
    int height;
    uint8_t *data;
    JPChar *next;
    int dimBuffer;
    int waste;

    JPChar() {
        code = 0;
        width = -1;
        height = -1;
        data = NULL;
        next = NULL;
        dimBuffer = 0;
        waste = 0;
    }

    ~JPChar() {
        if(data==NULL) {
            delete data;
        }
        next = NULL;
        cout << "Distruzione JPChar " << code <<endl;
    }

    string toString() {
        std::ostringstream s;
        s << "{ code=" << code << " [w=" << width << ", h=" << height << ", t=" << waste << "]}";
        return s.str();
    }
};

class JPFont {
private:
    JPChar *jpCharList,
            *endList;

    std::string delimiterFontFileDefinition;

public:
    static uint16_t KANJI_ORE;
    static uint16_t KANJI_MINUTI;
    static uint16_t KANJI_SETTIMANA;
    static uint16_t KANJI_GIORNO_SUFFIX;
    static uint16_t KANJI_MESE;

    static int string2Int(std::string inputString);

    JPFont(char *filename, char *font_definiton);
    ~JPFont();

    void processaFontDefinition(char *font_definiton);
    void processaRiga(JPChar *list, string line);
    void processaFontGlyph(Image *glyphs);

    JPChar *pushList(JPChar *newItem);
    JPChar *popList();
    JPChar *get(uint16_t theChar);
    void displayList();
    uint16_t dayOfWeekToUnicode(uint8_t d);
    uint16_t numberToUnicode(uint8_t d);
    void convertDayOfWeekToJapaneseKanji(uint16_t *kanjiUnicodeList, uint8_t day);
    void convertTimeToJapaneseKanji(uint16_t *kanjiUnicodeList, uint8_t hour, uint8_t minute);
    void convertDateToJapaneseKanji(uint16_t *kanjiUnicodeList, uint8_t day, uint8_t month);
};


#endif //OROLOGIO_JPFONT_H
