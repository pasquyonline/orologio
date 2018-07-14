//
// Created by pasquale on 24/06/2018.
//

#ifndef OROLOGIO_ORABIGFONT_H
#define OROLOGIO_ORABIGFONT_H

#include <stdint.h>
#include <cstring>
#include <string>
#include <cstdio>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <magick/api.h>

using namespace std;

class BigChar {
public:
    char code;
    int width;
    int height;
    uint8_t *data;
    BigChar *next;
    int dimBuffer;
    int waste;

    BigChar() {
        code = 0;
        width = -1;
        height = -1;
        data = NULL;
        next = NULL;
        dimBuffer = 0;
        waste = 0;
    }

    ~BigChar() {
        if(data==NULL) {
            delete data;
        }
        next = NULL;
        cout << "Distruzione BigChar " << code <<endl;
    }

    string toString() {
        std::ostringstream s;
        s << "{ code=" << code << " [w=" << width << ", h=" << height << ", t=" << waste << "]}";
        return s.str();
    }
};

class OraBigFont {
private:
    BigChar *bigCharList,
            *endList;

    std::string delimiterFontFileDefinition;

public:
    static int string2Int(std::string inputString);

    OraBigFont(char *filename, char *font_definiton);
    ~OraBigFont();

    void processaFontDefinition(char *font_definiton);
    void processaRiga(BigChar *list, string line);
    void processaFontGlyph(Image *glyphs);

    BigChar *pushList(BigChar *newItem);
    BigChar *popList();
    BigChar *get(char theChar);
    void displayList();
};

#endif //OROLOGIO_ORABIGFONT_H
