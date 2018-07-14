//
// Created by pasquale on 24/06/2018.
//

#include "LCD240x128.h"

/**
 * Indirizzo nella meoria del display di inizio area grafica
 */
unsigned short LCD240x128::START_GRAPHIC_ADDR = 0x0800;

/**
 * Numero totale di pixel del display 240x128 = 30720
 */
unsigned short LCD240x128::TOTAL_PIXEL = 30720;

/**
 * Costruttore
 */
LCD240x128::LCD240x128(){
    LCDInitPinData();
    setup(true, 0x00, false, true, 0);
}

/**
 *
 * @param clearChar
 * @param clearCharByte
 * @param clearCharGen
 * @param clearGraphic
 * @param clearByte
 */
LCD240x128::LCD240x128(bool clearChar, uint8_t clearCharByte, bool clearCharGen, bool clearGraphic, uint8_t clearByte) {
    LCDInitPinData();
    setup(clearChar, clearCharByte, clearCharGen, clearGraphic, clearByte);
}

/**
 *
 */
void LCD240x128::LCDInitPinData() {
    //data_pin[8] = {D0, D1, D2, D3, D4, D5, D6, D7};
    data_pin = new uint8_t[8];
    data_pin[0] = D0;
    data_pin[1] = D1;
    data_pin[2] = D2;
    data_pin[3] = D3;
    data_pin[4] = D4;
    data_pin[5] = D5;
    data_pin[6] = D6;
    data_pin[7] = D7;
}

/**
 *
 * @param clearCharByte
 */
void LCD240x128::lcdClear(uint8_t clearCharByte) {
    printf("\tLCD Clear...\n");
    locateXY(0, 0, false);
    for(unsigned int a=0; a<640; a++) {
        write1DataAndCommand(clearCharByte, 0xC0);
    }
    //writeCommand(0xb2);
    printf("\t...LCD Clear Fine.\n");
}

/**
 *
 */
void LCD240x128::lcdClearCharGen() {
    printf("\tLCD Clear Char gen...\n");
    write2DataAndCommand(0, 0x10, 0x24);
    for(unsigned int i = 0; i < 2048; i++) {
        write1DataAndCommand(0, 0xC0);
    }
    //writeCommand(0xb2);
    printf("\t...LCD Clear Char gen. Fine.\n");
}

/**
 *
 * @param c
 */
void LCD240x128::lcdClearGraphic(uint8_t c) {
    printf("\tLCD Clear Graphic...\n");
    write2DataAndCommand(0x00, 0x08, 0x24);
    //for(unsigned int a=0x800; a<0x1BFF; a++) {
    /*
     * Ricorda che il display ha il Font-Select a livello HIGH, ovvero caratteri 6x8 (WxH),
     * 40 colonne (di 6 pixel l'una = 240pixel orizzontali) x 128 righe => 40x128=5120
     * Questo implica che gli ultimi 2 bit del byte di cancellazione sono ignorati
     * */
    for(unsigned int a=0; a<5120; a++) {
        write1DataAndCommand(c, 0xC0);
    }
    writeCommand(0xb2);
    printf("\t...LCD Clear Graphic Fine.\n");
}

/**
 *
 */
void LCD240x128::LCDInit() {
    printf("\tStart init cmd...\n");
    //Font Select a 0 -> caratteri 8x8, 1 -> caratteri 6x8
    /*
    bcm2835_gpio_fsel(RD, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(RS, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(CE, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(CD, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(WR, BCM2835_GPIO_FSEL_OUTP);
    */

    INP_GPIO(RD);
    OUT_GPIO(RD);

    INP_GPIO(RS);
    OUT_GPIO(RS);

    INP_GPIO(CE);
    OUT_GPIO(CE);

    INP_GPIO(CD);
    OUT_GPIO(CD);

    INP_GPIO(WR);
    OUT_GPIO(WR);

    setDataPinOutputMode(BCM2835_GPIO_FSEL_OUTP);

    GPIO_CLR = 1 << CE; //bcm2835_gpio_write(CE, CHIP_ENABLE); //Abilitazione del display -> L:Active
    GPIO_SET = 1 << RD; //bcm2835_gpio_write(RD, HIGH);
    GPIO_SET = 1 << WR; //bcm2835_gpio_write(WR, HIGH);

    GPIO_CLR = 1 << RS; //bcm2835_gpio_write(RS, LOW); //Inizio del reset
    usleep(WAIT);
    GPIO_SET = 1 << RS; //bcm2835_gpio_write(RS, HIGH); //Fine del reset

    waitLCDReadyS0S1();

    write2DataAndCommand(0x00, 0x00, 0x40); //Set Text Home Address
    write2DataAndCommand(0x28, 0x00, 0x41); //Set Text Area Width
    write2DataAndCommand(0x00, 0x08, 0x42); //Set Graphic Home Address
    write2DataAndCommand(0x28, 0x00, 0x43); //Set Graphic Area Width
    writeCommand(0xA7); //Cursor Pattern Select
    writeCommand(0x81); //XOR mode
    writeCommand(0x9C); //Text on, graphic on
    //writeCommand(0x93); //Cursor blink
    //writeCommand(0x94); //Text on, graphic off
    writeCommand(0xB2); //Auto-reset

    printf("\t...end init cmd.\n");
}

/**
 *
 * @param clearChar
 * @param clearCharByte
 * @param clearCharGen
 * @param clearGraphic
 * @param clearByte
 * @return
 */
int LCD240x128::setup(bool clearChar, uint8_t clearCharByte, bool clearCharGen, bool clearGraphic, uint8_t clearByte) {
    struct timespec start, stop;

    clock_gettime(CLOCK_REALTIME, &start);
    LCDInit();
    clock_gettime(CLOCK_REALTIME, &stop);
    LCD240x128::printDuration(&start, &stop);

    if(clearChar) {
        clearLCDCharBuffer(clearCharByte, &start, &stop);
    }

    if(clearCharGen) {
        clearLCDCharGenBuffer(&start, &stop);
    }

    if(clearGraphic) {
        clearLCDGraphicBuffer(clearByte, &start, &stop);
    }

    return 0;
}

/**
 *
 * @param clearCharByte
 * @param start
 * @param stop
 */
void LCD240x128::clearLCDCharBuffer(uint8_t clearCharByte, struct timespec *start, struct timespec *stop) {
    clock_gettime(CLOCK_REALTIME, start);
    lcdClear(clearCharByte);
    clock_gettime(CLOCK_REALTIME, stop);
    LCD240x128::printDuration(start, stop);
}

/**
 *
 * @param start
 * @param stop
 */
void LCD240x128::clearLCDCharGenBuffer(struct timespec *start, struct timespec *stop) {
    clock_gettime(CLOCK_REALTIME, start);
    lcdClearCharGen();
    clock_gettime(CLOCK_REALTIME, stop);
    LCD240x128::printDuration(start, stop);
}

/**
 *
 * @param clearByte
 * @param start
 * @param stop
 */
void LCD240x128::clearLCDGraphicBuffer(uint8_t clearByte, struct timespec *start, struct timespec *stop) {
    clock_gettime(CLOCK_REALTIME, start);
    lcdClearGraphic(clearByte);
    clock_gettime(CLOCK_REALTIME, stop);
    LCD240x128::printDuration(start, stop);
}

/**
 *
 * @param mode - PIN mode
 */
void LCD240x128::setDataPinOutputMode(uint8_t mode) {
    //Init dei pin del bus dati per ricevere dati dal display
    /*for(uint8_t i=0; i<8; i++) {
        bcm2835_gpio_fsel(data_pin[i], mode);
    }*/

    for(uint8_t i=0; i<8; i++) {
        INP_GPIO(data_pin[i]);
    }

    if(mode==BCM2835_GPIO_FSEL_OUTP) {
        for(uint8_t i=0; i<8; i++) {
            OUT_GPIO(data_pin[i]);
        }
    }
}

/**
 *
 * @return
 */
uint8_t LCD240x128::readLCDStatus() {
    uint8_t lcdstatus[8];

    setDataPinOutputMode(BCM2835_GPIO_FSEL_INPT);
    GPIO_SET = 1 << CD; //bcm2835_gpio_write(CD, HIGH);
    GPIO_SET = 1 << WR; //bcm2835_gpio_write(WR, HIGH);
    GPIO_CLR = 1 << RD; //bcm2835_gpio_write(RD, LOW);

    GPIO_CLR = 1 << CE; //bcm2835_gpio_write(CE, CHIP_ENABLE);
    for(uint8_t i=0; i<8; i++) {
        lcdstatus[i] = /*GPIO_READ(data_pin[i]); //*/bcm2835_gpio_lev(data_pin[i]);
    }

    GPIO_SET = 1 << CE; //bcm2835_gpio_write(CE, !CHIP_ENABLE);
    uint8_t b =
            (lcdstatus[0] ? 0x01 : 0x00) |
            (lcdstatus[1] ? 0x02 : 0x00) |
            (lcdstatus[2] ? 0x04 : 0x00) |
            (lcdstatus[3] ? 0x08 : 0x00) |
            (lcdstatus[4] ? 0x10 : 0x00) |
            (lcdstatus[5] ? 0x20 : 0x00) |
            (lcdstatus[6] ? 0x40 : 0x00) |
            (lcdstatus[7] ? 0x80 : 0x00);
    return b;
}

/**
 *
 * @param d
 * @param isData
 */
void LCD240x128::writeByte(uint8_t d, bool isData) {
    setDataPinOutputMode(BCM2835_GPIO_FSEL_OUTP);
    GPIO_SET = 1 << RD; //bcm2835_gpio_write(RD, HIGH);

    if(isData) { //bcm2835_gpio_write(CD, isData ? LOW : HIGH);
        GPIO_CLR = 1 << CD;
    } else {
        GPIO_SET = 1 << CD;
    }
    GPIO_SET = 1 << WR; //bcm2835_gpio_write(WR, HIGH);

    for(uint8_t i=0; i<8; i++) {
        uint8_t b = (0x01<<i) & d;
        //bcm2835_gpio_write(data_pin[i], b);
        if(b) {
            GPIO_SET = 1 << data_pin[i];
        } else {
            GPIO_CLR = 1 << data_pin[i];
        }
    }

    GPIO_CLR = 1 << CE; //bcm2835_gpio_write(CE, CHIP_ENABLE);
    GPIO_CLR = 1 << WR; //bcm2835_gpio_write(WR, LOW);
    usleep(WAIT);
    GPIO_SET = 1 << WR; //bcm2835_gpio_write(WR, HIGH);
    GPIO_SET = 1 << CE; //bcm2835_gpio_write(CE, !CHIP_ENABLE);
}

/**
 *
 */
void LCD240x128::waitLCDReadyS0S1() {
    uint8_t b;

    do {
        b = readLCDStatus();
    } while(!(b && 0b00000011));
}

/**
 *
 */
void LCD240x128::waitLCDReadyS3() {
    uint8_t b;

    do {
        b = readLCDStatus();
    } while(!(b && 0b00001000));
}

/**
 *
 * @param d
 */
void LCD240x128::writeByteData(uint8_t d) {
    writeByte(d, true);
}

/**
 *
 * @param cmd
 */
void LCD240x128::writeByteCommand(uint8_t cmd) {
    writeByte(cmd, false);
}

/**
 *
 * @param cmd
 */
void LCD240x128::writeCommand(uint8_t cmd) {
    waitLCDReadyS3();
    writeByteCommand(cmd);
}

/**
 *
 * @param d
 * @param cmd
 */
void LCD240x128::write1DataAndCommand(uint8_t d, uint8_t cmd) {
    waitLCDReadyS0S1();
    writeByteData(d);

    waitLCDReadyS3();
    writeByteCommand(cmd);
}

/**
 *
 * @param d1
 * @param d2
 * @param cmd
 */
void LCD240x128::write2DataAndCommand(uint8_t d1, uint8_t d2, uint8_t cmd) {
    waitLCDReadyS0S1();
    writeByteData(d1);

    waitLCDReadyS0S1();
    writeByteData(d2);

    waitLCDReadyS3();
    writeByteCommand(cmd);
}

/**
 *
 * @param xa
 * @param ya
 * @param graphicMode
 */
void LCD240x128::locateXY(uint8_t xa, uint8_t ya, bool graphicMode) {
    unsigned int a=0;
    if(graphicMode) {
        //Graphic mode   START ADDRESS ：0800
        a=ya*40+xa;
        write2DataAndCommand((uint8_t)a, (uint8_t)(a>>8)+0x08, 0x24);
    }
    else {
        //Character mode START ADDRESS ：0000
        a = ya*40+xa;
        write2DataAndCommand((uint8_t)a, (uint8_t)(a>>8), 0x24);
    }
}

/**
 *
 * @param x
 * @param y
 * @param ascii
 */
void LCD240x128::displayStr(uint8_t x, uint8_t y, char *ascii) {
    locateXY(x, y, false);
    uint8_t i=0;
    while(ascii[i]>0) {
        write1DataAndCommand(ascii[i]-0x20, 0xc0);
        i++;
    }
}

/**
 *
 * @param start
 * @param stop
 */
void LCD240x128::printDuration(struct timespec *start, struct timespec *stop) {
    double elapsedPirOn = ( stop->tv_sec - start->tv_sec ) + (( stop->tv_nsec - start->tv_nsec ) / BILLION);
    printf("%0.6f sec\n", elapsedPirOn);
}

/**
 *
 * @param graphicAddr
 * @param pixelLCD
 */
void LCD240x128::drawBuffer(uint16_t graphicAddr, uint8_t *pixelLCD) {
    uint8_t lcd8Pixel = 0;
    uint8_t lcdPixel_i = 0;

    this->write2DataAndCommand((uint8_t)(graphicAddr & 0xFF), (uint8_t)(graphicAddr >> 8), 0x24);

    for(uint16_t idx_pixelLCD=0; idx_pixelLCD<LCD240x128::TOTAL_PIXEL; idx_pixelLCD++) {
        lcd8Pixel = lcd8Pixel<<1;
        if(pixelLCD[idx_pixelLCD]) {
            lcd8Pixel++;
        }
        lcdPixel_i++;
        if(lcdPixel_i==6) {
            this->write1DataAndCommand(lcd8Pixel, 0xC0);
            lcdPixel_i = 0;
            lcd8Pixel = 0;
        }
    }
}
