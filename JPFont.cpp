//
// Created by pasquale on 08/07/2018.
//

#include "JPFont.h"

using namespace std;

uint16_t JPFont::KANJI_ORE = 26178;
uint16_t JPFont::KANJI_MINUTI = 20998;
uint16_t JPFont::KANJI_SETTIMANA = 26332;
uint16_t JPFont::KANJI_GIORNO_SUFFIX = 26085;
uint16_t JPFont::KANJI_MESE = 26376;

JPFont::JPFont(char *filename, char *font_definiton) {
    delimiterFontFileDefinition = "=";
    jpCharList = endList = NULL;

    Image *image = (Image *) NULL;
    ExceptionInfo exception;
    char infile[MaxTextExtent];

    strncpy(infile, filename, MaxTextExtent-1 );
    ImageInfo *imageInfo = CloneImageInfo(0);
    GetExceptionInfo(&exception);

    strcpy(imageInfo->filename, infile);
    image = ReadImage(imageInfo, &exception);
    if (image == (Image *) NULL) {
        CatchException(&exception);
    }

    unsigned int w = image->columns;
    unsigned int h = image->rows;

    printf("JPFont -> W: %d, H: %d\n", w, h);

    processaFontDefinition(font_definiton);
    displayList();
    processaFontGlyph(image);
    displayList();

    if (image != (Image *) NULL)
        DestroyImage(image);
}

JPFont::~JPFont() {
    while(jpCharList!=NULL) {
        JPChar *x = popList();
        delete x;
    }

    cout<<"Distruzione JPFont"<<endl;
}


void JPFont::processaFontDefinition(char *font_definiton) {
    string line;
    ifstream f (font_definiton);
    if (!f.is_open()) {
        perror("error while opening file");
        return;
    }

    int numChar = 0;
    int lineIdx = 0;

    while(getline(f, line)) {
        if(lineIdx==0) {
            numChar = JPFont::string2Int(line);
            cout << "Num char: " << numChar << endl;
            lineIdx++;
        } else {
            processaRiga(jpCharList, line);
            lineIdx++;
        }
    }

    if (f.bad()) {
        perror("error while reading file");
        return;
    }

    f.close();
}

void JPFont::processaRiga(JPChar *list, string line) {
    size_t pos = 0;
    std::string tokens[2];
    int posToken=0;
    while (posToken<2) {
        pos = line.find(delimiterFontFileDefinition);
        tokens[posToken] = line.substr(0, pos);
        //std::cout << tokens[posToken] << std::endl;
        line.erase(0, pos + delimiterFontFileDefinition.length());
        //std::cout << line << std::endl;
        posToken++;
    }

    JPChar *tmp = new JPChar();
    tmp->code = atoi(tokens[0].c_str());
    tmp->width = JPFont::string2Int(tokens[1]);

    pushList(tmp);
}

int JPFont::string2Int(std::string inputString) {
    int val=0;

    stringstream ss;
    ss<<inputString;
    ss>>val;
    ss.str("");
    ss.clear();

    return val;
}

JPChar *JPFont::pushList(JPChar *newItem) {
    if(jpCharList==NULL) {
        jpCharList = newItem;
        endList = jpCharList;
    }
    else {
        endList->next = newItem;
        endList = newItem;
    }

    return endList;
}

JPChar *JPFont::popList() {
    JPChar *tmp = jpCharList;
    jpCharList = jpCharList->next;
    return tmp;
}

JPChar *JPFont::get(uint16_t theChar) {
    JPChar *tmp = jpCharList;
    while(tmp!=NULL && tmp->code!=theChar) {
        tmp = tmp->next;
    }

    return tmp;
}

void JPFont::displayList() {
    JPChar *tmp = jpCharList;
    if(tmp==NULL) {
        cout << "Lista Vuota" <<endl;
        return;
    }

    while(tmp!=NULL) {
        cout << tmp->toString() << endl;
        tmp = tmp->next;
    }
}


void JPFont::processaFontGlyph(Image *glyphs) {
    JPChar *tmp = jpCharList;
    if(tmp==NULL) {
        cout << "Lista Vuota" <<endl;
        return;
    }

    printf("\n");

    int posOrizStartChar = 0;
    unsigned int h = glyphs->rows;

    while(tmp!=NULL) {
        int w = tmp->width;
        tmp->height = h;
        tmp->dimBuffer = (1+(w/6))*h;
        //cout << tmp->code << " dimBuffer:" << tmp->dimBuffer << endl;
        tmp->data = new uint8_t[tmp->dimBuffer];
        tmp->waste = static_cast<int>(ceil((1.0 - (w / 6.0 - floor(w / 6.0))) * 100.0 * (6.0/100.0)));
        //cout << "Buffer allocato." << endl;
        int dataIndex = 0;
        uint8_t lcd8Pixel = 0;
        uint8_t lcdPixel_i = 0;
        for(int y=0; y<h; y++) {
            for (int x=0; x<w; x++) {
                PixelPacket p = GetOnePixel(glyphs, x+posOrizStartChar, y);
                int pxl = p.red>240 ? 1 : 0;
                //printf(pxl ? "X" : ".");
                lcd8Pixel = lcd8Pixel<<1; //solo lo shift inserisce uno 0 nel nuovo pixel, se lo incremento inserisce 1
                if(pxl) {
                    lcd8Pixel++;
                }
                lcdPixel_i++;
                if(lcdPixel_i==6) {
                    tmp->data[dataIndex] = lcd8Pixel;
                    lcdPixel_i = 0;
                    lcd8Pixel = 0;
                    dataIndex++;
                }
            }
            if(lcdPixel_i<6) {
                for(int i = lcdPixel_i; i<6; i++)
                    lcd8Pixel = lcd8Pixel<<1;
                tmp->data[dataIndex] = lcd8Pixel;
                lcdPixel_i = 0;
                lcd8Pixel = 0;
                dataIndex++;
            }
            //printf("\n");
        }
        //printf("\n");

        posOrizStartChar += w;
        tmp = tmp->next;
    }
}

uint16_t JPFont::dayOfWeekToUnicode(uint8_t d) {
    switch(d) {
        case 0: return 26085;
        case 1: return 26376;
        case 2: return 28779;
        case 3: return 27700;
        case 4: return 26408;
        case 5: return 37329;
        case 6: return 22303;
    }
}

uint16_t JPFont::numberToUnicode(uint8_t d) {
    switch(d) {
        case 1: return 19968;
        case 2: return 20108;
        case 3: return 19977;
        case 4: return 22235;
        case 5: return 20116;
        case 6: return 20845;
        case 7: return 19971;
        case 8: return 20843;
        case 9: return 20061;
        case 10: return 21313;
    }
}

void JPFont::convertDayOfWeekToJapaneseKanji(uint16_t *kanjiUnicodeList, uint8_t day) {
    //uint16_t kanjiUnicodeList[4] = {0, 0, 0, 0};

    kanjiUnicodeList[0] = this->dayOfWeekToUnicode(day);
    kanjiUnicodeList[1] = JPFont::KANJI_SETTIMANA;
    kanjiUnicodeList[2] = JPFont::KANJI_GIORNO_SUFFIX;
    kanjiUnicodeList[3] = 0;
}

void JPFont::convertTimeToJapaneseKanji(uint16_t *kanjiUnicodeList, uint8_t hour, uint8_t minute) {
    uint8_t idx=0;
    int decine = hour/10;
    int unita = hour%10;

    //Se ci troviamo tra le 0:00 e le 0:59, non disegno le ore ma solo i minuti
    if(hour==0) {
    }
    else if(hour>0 && hour<11) {
        kanjiUnicodeList[idx] = this->numberToUnicode(hour);
    }
    else {
        if(decine>1) {
            kanjiUnicodeList[idx] = this->numberToUnicode(decine);
            idx++;
        }

        kanjiUnicodeList[idx] = this->numberToUnicode(10); //L'Unicode della cifra 10 in giapponese
        idx++;
        if(unita>0) {
            kanjiUnicodeList[idx] = this->numberToUnicode(unita);
        }
        else
            idx--;
    }

    if(hour>0) {
        //Metto il kanji delle ore
        idx++;
        kanjiUnicodeList[idx] = JPFont::KANJI_ORE;
        idx++;
    }

    decine = minute/10;
    unita = minute%10;

    if(minute>0 && minute<11) {
        kanjiUnicodeList[idx] = this->numberToUnicode(minute);
    }
    else if(minute>10){
        if(decine>1) {
            kanjiUnicodeList[idx] = this->numberToUnicode(decine);
            idx++;
        }
        kanjiUnicodeList[idx] = this->numberToUnicode(10);
        idx++;
        if(unita>0) {
            kanjiUnicodeList[idx] = this->numberToUnicode(unita);
        }
        else
            idx--;
    }

    //Metto il kanji dei minuti
    if(minute==0) {
        idx--;
    }
    else if(minute>0) {
        idx++;
        kanjiUnicodeList[idx] = JPFont::KANJI_MINUTI;
    }

    idx++;
    if(idx<9) {
        kanjiUnicodeList[idx] = 0;
    }
}

void JPFont::convertDateToJapaneseKanji(uint16_t *kanjiUnicodeList, uint8_t day, uint8_t month) {
    uint8_t idx=0;
    int decine = day/10;
    int unita = day%10;

    //Se ci troviamo tra le 0:00 e le 0:59, non disegno le ore ma solo i minuti
    if(day==0) {
    }
    else if(day>0 && day<11) {
        kanjiUnicodeList[idx] = this->numberToUnicode(day);
    }
    else {
        if(decine>1) {
            kanjiUnicodeList[idx] = this->numberToUnicode(decine);
            idx++;
        }

        kanjiUnicodeList[idx] = this->numberToUnicode(10); //L'Unicode della cifra 10 in giapponese
        idx++;
        if(unita>0) {
            kanjiUnicodeList[idx] = this->numberToUnicode(unita);
        }
        else
            idx--;
    }

    if(day>0) {
        //Metto il kanji delle ore
        idx++;
        kanjiUnicodeList[idx] = JPFont::KANJI_GIORNO_SUFFIX;
        idx++;
    }

    decine = month/10;
    unita = month%10;

    if(month>0 && month<11) {
        kanjiUnicodeList[idx] = this->numberToUnicode(month);
    }
    else if(month>10){
        if(decine>1) {
            kanjiUnicodeList[idx] = this->numberToUnicode(decine);
            idx++;
        }
        kanjiUnicodeList[idx] = this->numberToUnicode(10);
        idx++;
        if(unita>0) {
            kanjiUnicodeList[idx] = this->numberToUnicode(unita);
        }
        else
            idx--;
    }

    //Metto il kanji dei minuti
    if(month==0) {
        idx--;
    }
    else if(month>0) {
        idx++;
        kanjiUnicodeList[idx] = JPFont::KANJI_MESE;
    }

    idx++;
    if(idx<9) {
        kanjiUnicodeList[idx] = 0;
    }
}
