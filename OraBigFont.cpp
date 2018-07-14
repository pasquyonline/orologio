//
// Created by pasquale on 24/06/2018.
//

#include "OraBigFont.h"

using namespace std;

OraBigFont::OraBigFont(char *filename, char *font_definiton) {
    delimiterFontFileDefinition = "=";
    bigCharList = endList = NULL;

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

    printf("OraBigFont -> W: %d, H: %d\n", w, h);

    processaFontDefinition(font_definiton);
    displayList();
    processaFontGlyph(image);
    displayList();
/*
    FILE* fp;
    char buf[1024];

    if ((fp = fopen(filename, "r")) == NULL)
    {
        perror("fopen source-file");
    }

    while (fgets(buf, sizeof(buf), fp) != NULL)
    {
        buf[strlen(buf) - 1] = '\0'; // eat the newline fgets() stores
        printf("%s\n", buf);
    }
    fclose(fp);
*/
    if (image != (Image *) NULL)
        DestroyImage(image);
}

OraBigFont::~OraBigFont() {
    while(bigCharList!=NULL) {
        BigChar *x = popList();
        delete x;
    }

    cout<<"Distruzione BigFont"<<endl;
}

void OraBigFont::processaFontDefinition(char *font_definiton) {
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
            numChar = OraBigFont::string2Int(line);
            cout << "Num char: " << numChar << endl;
            lineIdx++;
        } else {
            processaRiga(bigCharList, line);
            lineIdx++;
        }
    }

    if (f.bad()) {
        perror("error while reading file");
        return;
    }

    f.close();
}

void OraBigFont::processaRiga(BigChar *list, string line) {
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

    BigChar *tmp = new BigChar();
    tmp->code = tokens[0][0];
    tmp->width = OraBigFont::string2Int(tokens[1]);

    pushList(tmp);
}

int OraBigFont::string2Int(std::string inputString) {
    int val=0;

    stringstream ss;
    ss<<inputString;
    ss>>val;
    ss.str("");
    ss.clear();

    return val;
}

BigChar *OraBigFont::pushList(BigChar *newItem) {
    if(bigCharList==NULL) {
        bigCharList = newItem;
        endList = bigCharList;
    }
    else {
        endList->next = newItem;
        endList = newItem;
    }

    return endList;
}

BigChar *OraBigFont::popList() {
    BigChar *tmp = bigCharList;
    bigCharList = bigCharList->next;
    return tmp;
}

BigChar *OraBigFont::get(char theChar) {
    BigChar *tmp = bigCharList;
    while(tmp!=NULL && tmp->code!=theChar) {
        tmp = tmp->next;
    }

    return tmp;
}

void OraBigFont::displayList() {
    BigChar *tmp = bigCharList;
    if(tmp==NULL) {
        cout << "Lista Vuota" <<endl;
        return;
    }

    while(tmp!=NULL) {
        cout << tmp->toString() << endl;
        tmp = tmp->next;
    }
}

void OraBigFont::processaFontGlyph(Image *glyphs) {
    BigChar *tmp = bigCharList;
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

