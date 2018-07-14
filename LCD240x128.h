//
// Created by pasquale on 24/06/2018.
//
#include <bcm2835.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <string.h>
#include <math.h>

#include "RPI.h"

#ifndef UNTITLED_LCD240X128_H
#define UNTITLED_LCD240X128_H


#define CHIP_ENABLE 0
#define WAIT 1
#define BILLION  1000000000.0

/*Arduino - LCD240x64
  D4      -> 19 - FS [Font Select (H:6x8, L: 8x8)]
  D5      ->  5 - WR [Write Signal]
  A3/17   ->  6 - RD [Read Signal]
  A2/16   ->  8 - CD [H: Data / L: Instruction Code]
  A1/15   -> 10 - RS [Reset]
  A0/14   ->  7 - CE [Chip Enable]
  D6      -> 11 - DB0
  D7      -> 12 - DB1
  D8      -> 13 - DB2
  D9      -> 14 - DB3
  D10     -> 15 - DB4
  D11     -> 16 - DB5
  D12     -> 17 - DB6
  D13     -> 18 - DB7
*/

    /*
  RPI_BPLUS_GPIO_J8_05 = 3,  RPI_BPLUS_GPIO_J8_07 = 4,  RPI_BPLUS_GPIO_J8_08 = 14, RPI_BPLUS_GPIO_J8_10 = 15,
  RPI_BPLUS_GPIO_J8_11 = 17, RPI_BPLUS_GPIO_J8_12 = 18, RPI_BPLUS_GPIO_J8_13 = 27, RPI_BPLUS_GPIO_J8_15 = 22,
  RPI_BPLUS_GPIO_J8_16 = 23, RPI_BPLUS_GPIO_J8_18 = 24, RPI_BPLUS_GPIO_J8_19 = 10, RPI_BPLUS_GPIO_J8_21 = 9,
  RPI_BPLUS_GPIO_J8_22 = 25, RPI_BPLUS_GPIO_J8_23 = 11, RPI_BPLUS_GPIO_J8_24 = 8,  RPI_BPLUS_GPIO_J8_26 = 7,
  RPI_BPLUS_GPIO_J8_29 = 5,  RPI_BPLUS_GPIO_J8_31 = 6,  RPI_BPLUS_GPIO_J8_32 = 12, RPI_BPLUS_GPIO_J8_33 = 13,
  RPI_BPLUS_GPIO_J8_35 = 19, RPI_BPLUS_GPIO_J8_36 = 16, RPI_BPLUS_GPIO_J8_37 = 26, RPI_BPLUS_GPIO_J8_38 = 20,
  RPI_BPLUS_GPIO_J8_40 = 21
 */
/*
 * Data pins:
 * D0=20, D1=16, D2=12, D3=21, D4=24, D5=25, D6=8, D7=7
 *
 * Control pins:
 * WR=3, RD=4, CE=17, CD=27, RS=22
 *
*/
#define D0 RPI_BPLUS_GPIO_J8_38
#define D1 RPI_BPLUS_GPIO_J8_36
#define D2 RPI_BPLUS_GPIO_J8_32
#define D3 RPI_BPLUS_GPIO_J8_40
#define D4 RPI_BPLUS_GPIO_J8_18
#define D5 RPI_BPLUS_GPIO_J8_22
#define D6 RPI_BPLUS_GPIO_J8_24
#define D7 RPI_BPLUS_GPIO_J8_26

#define WR RPI_BPLUS_GPIO_J8_29
#define RD RPI_BPLUS_GPIO_J8_31
#define CE RPI_BPLUS_GPIO_J8_33
#define CD RPI_BPLUS_GPIO_J8_35
#define RS RPI_BPLUS_GPIO_J8_37
/*
#define WR RPI_BPLUS_GPIO_J8_05
#define RD RPI_BPLUS_GPIO_J8_07
#define CE RPI_BPLUS_GPIO_J8_11
#define CD RPI_BPLUS_GPIO_J8_13
#define RS RPI_BPLUS_GPIO_J8_15
*/
class LCD240x128 {
private:
    uint8_t *data_pin;

public:
    static unsigned short START_GRAPHIC_ADDR;
    static unsigned short TOTAL_PIXEL;

    LCD240x128();
    LCD240x128(bool clearChar, uint8_t clearCharByte, bool clearCharGen, bool clearGraphic, uint8_t clearByte);

    int setup(bool clearChar, uint8_t clearCharByte, bool clearCharGen, bool clearGraphic, uint8_t clearByte);
    void clearLCDCharBuffer(uint8_t clearCharByte, struct timespec *start, struct timespec *stop);
    void clearLCDCharGenBuffer(struct timespec *start, struct timespec *stop);
    void clearLCDGraphicBuffer(uint8_t clearByte, struct timespec *start, struct timespec *stop);

    void LCDInitPinData();
    void LCDInit();
    void lcdClear(uint8_t clearCharByte);
    void lcdClearCharGen();
    void lcdClearGraphic(uint8_t c);

    void setDataPinOutputMode(uint8_t mode);
    uint8_t readLCDStatus();
    void writeByte(uint8_t d, bool isData);
    void waitLCDReadyS0S1();
    void waitLCDReadyS3();
    void writeByteData(uint8_t d);
    void writeByteCommand(uint8_t cmd);
    void writeCommand(uint8_t cmd);
    void write1DataAndCommand(uint8_t d, uint8_t cmd);
    void write2DataAndCommand(uint8_t d1, uint8_t d2, uint8_t cmd);
    void locateXY(uint8_t xa, uint8_t ya, bool graphicMode);
    void displayStr(uint8_t x, uint8_t y, char *ascii);
    static void printDuration(struct timespec *start, struct timespec *stop);
    void drawBuffer(uint16_t graphicAddr, uint8_t *pixelLCD);
};


#endif //UNTITLED_LCD240X128_H
