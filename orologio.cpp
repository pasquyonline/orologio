
#include <bcm2835.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <cstring>
#include <math.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>

#include <signal.h>

#include <magick/api.h>
#include "LCD240x128.h"
#include "OraBigFont.h"
#include "JPFont.h"

volatile sig_atomic_t done = 0;

void getIPAddress(char *msg) {
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];

    if(getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
    }
    else {
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr == NULL) continue;
            s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if ((strcmp(ifa->ifa_name, "lo") != 0)/*&&(ifa->ifa_addr->sa_family==AF_INET)*/) {
                if (s != 0) {}
                else {
                    int l = strlen(host);
                    strcpy(msg, host);
                    cout << host << endl;
                }
            }
        }
    }
    freeifaddrs(ifaddr);
}


inline void setupStatusLED() {
    //bcm2835_gpio_fsel(RPI_BPLUS_GPIO_J8_19, BCM2835_GPIO_FSEL_OUTP);
    //bcm2835_gpio_write(RPI_BPLUS_GPIO_J8_19, HIGH);
// Define pin 7 as output
    INP_GPIO(10);
    OUT_GPIO(10);
}

inline void statusLedON() {
    GPIO_SET = 1 << 10;
}

inline void statusLedOFF() {
    GPIO_CLR = 1 << 10;
}

void displayBitmap(char *filename, LCD240x128 *lcd240x128, uint8_t *pixelLCD) {
    struct timespec start, stop;
    unsigned short idx_pixelLCD=0;
    Image *image = (Image *) NULL;
    ExceptionInfo exception;

    clock_gettime(CLOCK_REALTIME, &start);

    ImageInfo *imageInfo = CloneImageInfo(0);
    GetExceptionInfo(&exception);

    strcpy(imageInfo->filename, filename);
    image = ReadImage(imageInfo, &exception);
    if (image == (Image *) NULL) {
        CatchException(&exception);
        return;
    }
    clock_gettime(CLOCK_REALTIME, &stop);

    //PixelPacket *pixel_cache = GetPixels(image);
    unsigned int w = image->columns;
    unsigned int h = image->rows;

    if(w>240) w = 240;
    if(h>128) h = 128;

    printf("Lettura pixel...\n");
    clock_gettime(CLOCK_REALTIME, &start);
    for(int y=0; y<h; y++) {
        for (int x = 0; x < w; x++) {
            PixelPacket p = GetOnePixel(image, x, y);
            pixelLCD[idx_pixelLCD] = p.red>240 ? 1 : 0;
            idx_pixelLCD++;
        }
    }

    clock_gettime(CLOCK_REALTIME, &stop);
    lcd240x128->printDuration(&start, &stop);

    printf("Invio pixel all'LCD...\n");
    clock_gettime(CLOCK_REALTIME, &start);

    lcd240x128->drawBuffer(LCD240x128::START_GRAPHIC_ADDR, pixelLCD);

    clock_gettime(CLOCK_REALTIME, &stop);
    lcd240x128->printDuration(&start, &stop);

    if (image != (Image *) NULL)
        DestroyImage(image);
}

void drawTimeITWide(LCD240x128 *lcd240x128, OraBigFont *oraBigFont,
                    int *_cursorPos, unsigned short *_graphicAddr, unsigned short startPosizioneOra,
                    char *dateString, char *timeString, char *timeStringPrecedente
) {

    int cursorPos = *_cursorPos;
    int h = 0;
    unsigned short graphicAddr = (*_graphicAddr);
    uint8_t timeIdx = 0,
            timeDiffIdx = 0,
            byteOrizz = 0;

    //Mostra la data in modalita' testo
    lcd240x128->displayStr(26, 11, dateString);

    while(timeString[timeIdx]==timeStringPrecedente[timeIdx]) {
        timeIdx++;
    }

    timeDiffIdx = timeIdx;
    timeIdx = 0;

    while (timeString[timeIdx] != 0) {
        BigChar *c = oraBigFont->get(timeString[timeIdx]);
        if (c == NULL) {
            cout << "non trovato" << endl;
            timeIdx++;
            continue;
        }

        h = c->height;

        byteOrizz = 1 + (c->width / 6);
        if(timeIdx>=timeDiffIdx) {
            int tempCursorPos = cursorPos;
            lcd240x128->write2DataAndCommand((uint8_t) (graphicAddr & 0xFF), (uint8_t) (graphicAddr >> 8), 0x24);
            for (int b = 0; b < c->dimBuffer; b++) {
                if (b % byteOrizz == 0) {
                    lcd240x128->write2DataAndCommand((uint8_t) (graphicAddr & 0xFF), (uint8_t) (graphicAddr >> 8), 0x24);
                    tempCursorPos = cursorPos;
                }

                //Per evitare l'overflow orizzontale che andrebbe a scrivere sul primo carattere
                if(tempCursorPos<40)
                    lcd240x128->write1DataAndCommand(c->data[b], 0xC0);

                if (b % byteOrizz == 0) {
                    graphicAddr += 40;
                }
                tempCursorPos++;
            }
        }

        cursorPos += byteOrizz;
        graphicAddr = startPosizioneOra + LCD240x128::START_GRAPHIC_ADDR + cursorPos;
        lcd240x128->write2DataAndCommand((uint8_t) (graphicAddr & 0xFF), (uint8_t) (graphicAddr >> 8), 0x24);

        timeIdx++;
    }

    if(cursorPos) {
        for(int b=0; b<h; b++) {
            for(int bb=cursorPos; bb<40; bb++) {
                lcd240x128->write1DataAndCommand(0, 0xC0);
            }
            graphicAddr += 40;
            lcd240x128->write2DataAndCommand((uint8_t) (graphicAddr & 0xFF), (uint8_t) (graphicAddr >> 8), 0x24);
        }
    }

    (*_cursorPos) = cursorPos;
    (*_graphicAddr) = graphicAddr;
}

uint8_t sizeOfJPBuffer(uint16_t *buff, uint8_t maxSize) {
    uint8_t timeIdx = 0;
    while(timeIdx<maxSize && buff[timeIdx]!=0) {
        timeIdx++;
    }

    return timeIdx;
}

bool bufferUguali(uint16_t *B1, uint16_t *B2, uint8_t  max) {
    bool uguali = true;
    uint8_t timeIdx = 0;

    while(uguali && timeIdx<max) {
        uguali &= B1[timeIdx]==B2[timeIdx];
        if(B1[timeIdx]==0 || B2[timeIdx]==0) {
            break;
        }
        timeIdx++;
    }

    return uguali;
}

void drawJPtext(LCD240x128 *lcd240x128, JPFont *jpFont, struct tm *tm,
                int *_cursorPos, unsigned short *_graphicAddr, unsigned short startPosizioneOraJP,
                uint16_t *timeJP, uint16_t *timeJPprecedente) {

    int cursorPos = *_cursorPos;
    int h = 0;
    unsigned short graphicAddr = (*_graphicAddr);
    uint8_t timeIdx = 0,
            byteOrizz = 0;

    graphicAddr = startPosizioneOraJP + LCD240x128::START_GRAPHIC_ADDR;
    lcd240x128->write2DataAndCommand((uint8_t) (graphicAddr & 0xFF), (uint8_t) (graphicAddr >> 8), 0x24);

    bool uguali = bufferUguali(timeJP, timeJPprecedente, 9);

    if(uguali) {
        return;
    }

    timeIdx=0;
    while (timeJP[timeIdx] != 0) {
        JPChar *c = jpFont->get(timeJP[timeIdx]);
        if (c == NULL) {
            cout << "jp non trovato" << endl;
            timeIdx++;
            continue;
        }

        h = c->height;

        byteOrizz = 1 + (c->width / 6);
        int tempCursorPos = cursorPos;
        lcd240x128->write2DataAndCommand((uint8_t) (graphicAddr & 0xFF), (uint8_t) (graphicAddr >> 8), 0x24);
        for (int b = 0; b < c->dimBuffer; b++) {
            if (b % byteOrizz == 0) {
                lcd240x128->write2DataAndCommand((uint8_t) (graphicAddr & 0xFF), (uint8_t) (graphicAddr >> 8), 0x24);
                tempCursorPos = cursorPos;
            }

            //Per evitare l'overflow orizzontale che andrebbe a scrivere sul primo carattere
            if(tempCursorPos<40)
                lcd240x128->write1DataAndCommand(c->data[b], 0xC0);

            if (b % byteOrizz == 0) {
                graphicAddr += 40;
            }
            tempCursorPos++;
        }

        cursorPos += byteOrizz;
        graphicAddr = startPosizioneOraJP + LCD240x128::START_GRAPHIC_ADDR + cursorPos;
        lcd240x128->write2DataAndCommand((uint8_t) (graphicAddr & 0xFF), (uint8_t) (graphicAddr >> 8), 0x24);

        timeIdx++;
    }

    if(cursorPos) {
        for(int b=0; b<h; b++) {
            for(int bb=cursorPos; bb<40; bb++) {
                lcd240x128->write1DataAndCommand(0, 0xC0);
            }
            graphicAddr += 40;
            lcd240x128->write2DataAndCommand((uint8_t) (graphicAddr & 0xFF), (uint8_t) (graphicAddr >> 8), 0x24);
        }
    }

    uint8_t sizeTimeJP = sizeOfJPBuffer(timeJP, 9);

    for(uint8_t i=0; i<sizeTimeJP; i++) {
        timeJPprecedente[i] = timeJP[i];
    }
    timeJPprecedente[sizeTimeJP] = 0;

    (*_cursorPos) = cursorPos;
    (*_graphicAddr) = graphicAddr;
}

void drawTimeJP(LCD240x128 *lcd240x128, JPFont *jpFont, struct tm *tm,
                int *_cursorPos, unsigned short *_graphicAddr, unsigned short startPosizioneOraJP,
                uint16_t *timeJP, uint16_t *timeJPprecedente) {

    jpFont->convertTimeToJapaneseKanji(timeJP, tm->tm_hour, tm->tm_min/*, tm->tm_sec*/);
    drawJPtext(lcd240x128, jpFont, tm, _cursorPos, _graphicAddr, startPosizioneOraJP, timeJP, timeJPprecedente);
}

void drawDayOfWeekJP(LCD240x128 *lcd240x128, JPFont *jpFont, struct tm *tm,
                     int *_cursorPos, unsigned short *_graphicAddr, unsigned short startPosizioneOraJP,
                     uint16_t *timeJP, uint16_t *timeJPprecedente) {

    jpFont->convertDayOfWeekToJapaneseKanji(timeJP, tm->tm_wday);
    drawJPtext(lcd240x128, jpFont, tm, _cursorPos, _graphicAddr, startPosizioneOraJP, timeJP, timeJPprecedente);
}

void drawDateJP(LCD240x128 *lcd240x128, JPFont *jpFont, struct tm *tm,
                     int *_cursorPos, unsigned short *_graphicAddr, unsigned short startPosizioneOraJP,
                     uint16_t *timeJP, uint16_t *timeJPprecedente) {

    jpFont->convertDateToJapaneseKanji(timeJP, tm->tm_mday, 1+tm->tm_mon);
    drawJPtext(lcd240x128, jpFont, tm, _cursorPos, _graphicAddr, startPosizioneOraJP, timeJP, timeJPprecedente);
}

void drawString(LCD240x128* lcd240x128, JPFont *itaFont,
                     int *_cursorPos, unsigned short *_graphicAddr, unsigned short startPosizione,
                     char *temperatura, char *temperaturaPrecedente) {
    int cursorPos = *_cursorPos;
    int h = 0;
    unsigned short graphicAddr = (*_graphicAddr);
    uint8_t timeIdx = 0,
            byteOrizz = 0;

    graphicAddr = startPosizione + LCD240x128::START_GRAPHIC_ADDR;
    lcd240x128->write2DataAndCommand((uint8_t) (graphicAddr & 0xFF), (uint8_t) (graphicAddr >> 8), 0x24);

    bool uguali = strcmp(temperatura, temperaturaPrecedente)==0;

    if(uguali) {
        return;
    }

    timeIdx=0;
    while (temperatura[timeIdx] != 0) {
        JPChar *c = itaFont->get(temperatura[timeIdx]);
        if (c == NULL) {
            cout << "ita non trovato" << endl;
            timeIdx++;
            continue;
        }

        h = c->height;

        byteOrizz = 1 + (c->width / 6);
        int tempCursorPos = cursorPos;
        lcd240x128->write2DataAndCommand((uint8_t) (graphicAddr & 0xFF), (uint8_t) (graphicAddr >> 8), 0x24);
        for (int b = 0; b < c->dimBuffer; b++) {
            if (b % byteOrizz == 0) {
                lcd240x128->write2DataAndCommand((uint8_t) (graphicAddr & 0xFF), (uint8_t) (graphicAddr >> 8), 0x24);
                tempCursorPos = cursorPos;
            }

            //Per evitare l'overflow orizzontale che andrebbe a scrivere sul primo carattere
            if(tempCursorPos<40)
                lcd240x128->write1DataAndCommand(c->data[b], 0xC0);

            if (b % byteOrizz == 0) {
                graphicAddr += 40;
            }
            tempCursorPos++;
        }

        cursorPos += byteOrizz;
        graphicAddr = startPosizione + LCD240x128::START_GRAPHIC_ADDR + cursorPos;
        lcd240x128->write2DataAndCommand((uint8_t) (graphicAddr & 0xFF), (uint8_t) (graphicAddr >> 8), 0x24);

        timeIdx++;
    }

    if(cursorPos) {
        for(int b=0; b<h; b++) {
            for(int bb=cursorPos; bb<40; bb++) {
                lcd240x128->write1DataAndCommand(0, 0xC0);
            }
            graphicAddr += 40;
            lcd240x128->write2DataAndCommand((uint8_t) (graphicAddr & 0xFF), (uint8_t) (graphicAddr >> 8), 0x24);
        }
    }

    strcpy(temperaturaPrecedente, temperatura);

    (*_cursorPos) = cursorPos;
    (*_graphicAddr) = graphicAddr;
}

void signalterminate(int signum) {
    done = 1;
    printf("Orologio LCD 240x128 exit on sigterm %d.\n", signum);
}

void setupSignalAction() {
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = signalterminate;
    sigaction(SIGTERM, &action, NULL);
}

int main(int argc, char **argv) {
    Image *image = (Image *) NULL;
    ImageInfo *imageInfo;
    ExceptionInfo exception;
    char infile[MaxTextExtent];
    struct timespec start, stop;
    struct timespec startCiclo, endCiclo;
    uint8_t pixelLCD[LCD240x128::TOTAL_PIXEL]; //240*128

    if(!bcm2835_init()) {
        printf("Errore inizializzazione bcm2835\n");
        return 1;
    }

    if(map_peripheral(&gpio) == -1) {
        printf("Failed to map the physical GPIO registers into the virtual memory space.\n");
        return -1;
    }

    setupSignalAction();

    char ipAddressString[20];
    getIPAddress(ipAddressString);

    setupStatusLED();
    statusLedON();

    InitializeMagick(NULL);

    OraBigFont *oraBigFont = new OraBigFont(
            const_cast<char *>("resources/OraBigFont.bmp"),
            const_cast<char *>("resources/def_big_font.txt"));

    JPFont *jpFont = new JPFont(
            const_cast<char *>("resources/JPFont.bmp"),
            const_cast<char *>("resources/def_jp_font.txt"));

    JPFont *itaFont = new JPFont(
            const_cast<char *>("resources/numeri-lettere-essenziali.bmp"),
            const_cast<char *>("resources/def_font.txt"));


    LCD240x128 *lcd240x128 = new LCD240x128();

    if(argc==2) {
        strncpy(infile, argv[1], MaxTextExtent-1 );
    } else {
        strncpy(infile, const_cast<char *>("test/2030-1.bmp"), MaxTextExtent-1 );
    }

    printf("End setup.\n");

    //lcd240x128->displayStr(0, 0, const_cast<char *>("Test load image..."));

    displayBitmap(infile, lcd240x128, pixelLCD);
    //lcd240x128->lcdClear(0);
    //lcd240x128->lcdClearGraphic(0x00);
    displayBitmap(const_cast<char *>("resources/background.bmp"), lcd240x128, pixelLCD);

    struct timeval tv;
    struct timezone tz;
    struct tm *tm;

    char secondString[4];
    char dateString[13];
    char timeString[13];
    char timeStringPrecedente[13];
    int timeDiffIdx = 0;
    int cursorPos = 0;
    int byteOrizz = 0;
    int x_coord = 0;
    int y_coord = 0;
    int h;
    unsigned short graphicAddr = 0;
    unsigned short startPosizioneOra = 160; //Y=4
    unsigned short startPosizioneOraJP = 3960; //Y=99
    timeStringPrecedente[0] = 0;

    int modalita = 0;
    clock_gettime(CLOCK_REALTIME, &startCiclo);
    int modalitaMassima=5;
    uint16_t timeJP[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint16_t timeJPprecedente[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

    char temperatura[20], temperaturaPrecedente[20];
    char pressione[20], pressionePrecedente[20];
    char umidita[20], umiditaPrecedente[20];

    /* initialize random seed: */
    srand (time(NULL));

    lcd240x128->displayStr(0, 11, ipAddressString);
    while(done==0) {
        graphicAddr = startPosizioneOra + LCD240x128::START_GRAPHIC_ADDR;
        lcd240x128->write2DataAndCommand((uint8_t) (graphicAddr & 0xFF), (uint8_t) (graphicAddr >> 8), 0x24);

        gettimeofday(&tv, &tz);
        tm = localtime(&tv.tv_sec);

        sprintf(secondString, "%02d", tm->tm_sec);
        sprintf(timeString, "%2d:%02d", tm->tm_hour, tm->tm_min/*, tm->tm_sec*/);
        sprintf(dateString, "%02d/%02d/%d, ", tm->tm_mday, 1 + tm->tm_mon, 1900 + tm->tm_year);

        sprintf(temperatura, "T: %.02f*C", (rand()%3000)/100.0);
        sprintf(pressione, "P: %.02fhPa", (rand()%100) + 924.0);
        sprintf(umidita, "U: %.02f%%", (rand()%100)*1.0);

        h=0;
        while(temperatura[h]!='*') h++;
        temperatura[h] = 0xA7; //167 il simbolo del grado
        h=0;

        if(!strcmp(timeStringPrecedente, timeString)) {
            lcd240x128->displayStr(38, 11, secondString);
            usleep(100000);
        } else {
            cursorPos = 0;
            //Aggiorno l'ora grande solo se e' cambiata la stringa che la rappresenta
            clock_gettime(CLOCK_REALTIME, &start);
            drawTimeITWide(lcd240x128, oraBigFont,
                           &cursorPos, &graphicAddr, startPosizioneOra,
                           dateString, timeString, timeStringPrecedente);
            clock_gettime(CLOCK_REALTIME, &stop);
            //lcd240x128->printDuration(&start, &stop);

            strcpy(timeStringPrecedente, timeString);
        }

        //printf("Local Date: %02d/%02d/%d, ", tm->tm_mday, 1 + tm->tm_mon, 1900 + tm->tm_year);
        //printf("Local Time: %s\n", dateString);

        cursorPos = 0;

        //Disegno la parte inferiore dello schermo
        clock_gettime(CLOCK_REALTIME, &start);
        switch(modalita) {
            case 0:
                drawTimeJP(lcd240x128, jpFont, tm, &cursorPos, &graphicAddr, startPosizioneOraJP, timeJP, timeJPprecedente);
                break;
            case 1:
                drawDayOfWeekJP(lcd240x128, jpFont, tm, &cursorPos, &graphicAddr, startPosizioneOraJP, timeJP, timeJPprecedente);
                break;
            case 2:
                drawDateJP(lcd240x128, jpFont, tm, &cursorPos, &graphicAddr, startPosizioneOraJP, timeJP, timeJPprecedente);
                break;
            case 3:
                drawString(lcd240x128, itaFont, &cursorPos, &graphicAddr, startPosizioneOraJP, temperatura, temperaturaPrecedente);
                break;
            case 4:
                drawString(lcd240x128, itaFont, &cursorPos, &graphicAddr, startPosizioneOraJP, pressione, pressionePrecedente);
                break;
            case 5:
                drawString(lcd240x128, itaFont, &cursorPos, &graphicAddr, startPosizioneOraJP, umidita, umiditaPrecedente);
                break;
        }
        clock_gettime(CLOCK_REALTIME, &stop);
        //lcd240x128->printDuration(&start, &stop);

        clock_gettime(CLOCK_REALTIME, &endCiclo);
        double elapsedTime = ( endCiclo.tv_sec - startCiclo.tv_sec ) + (( endCiclo.tv_nsec - startCiclo.tv_nsec ) / BILLION);
        //cout << "elapsed:" << elapsedTime << endl;
        if(elapsedTime>5) {
            modalita++;
            clock_gettime(CLOCK_REALTIME, &startCiclo);
        }
        if(modalitaMassima==modalita) {
            modalita=0;
        }
    }

    delete oraBigFont;
    delete jpFont;
    delete itaFont;

    DestroyMagick();

    statusLedOFF();

    bcm2835_close();

    return 0;
}