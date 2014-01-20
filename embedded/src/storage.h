/**
 * @file storage.h
 * @author Alexander Rössler
 * @brief IRemote specific functions
 * @date 18-01-2014
 */
#pragma once

#include <storagemanager.h>
#include <spiflash.h>
#include <pincon.h>
#include "remotecontrol.h"

typedef struct
{
    uint8 version;
    char commandName[50u];
    uint8 reserved[100u];
} CommandHeader;

typedef struct
{
    CommandHeader commandHeader;
    RemoteCommand remoteCommand;
} StorageItem;

static const uint8  FlashSelPort = 1u;
static const uint8  FlashSelPin = 22u;
static const uint8  FlashHldPort = 1u;
static const uint8  FlashHldPin = 29u;
static const uint8  FlashWpPort = 2u;
static const uint8  FlashWpPin = 10u;
static const uint8  FlashSckPort = 1u;
static const uint8  FlashSckPin = 20u;
static const Pin_Function FlashSckFunction = Pin_Function_ThirdAlternate;
static const uint8  FlashMisoPort = 1u;
static const uint8  FlashMisoPin = 23u;
static const Pin_Function FlashMisoFunction = Pin_Function_ThirdAlternate;
static const uint8  FlashMosiPort = 1u;
static const uint8  FlashMosiPin = 24u;
static const Pin_Function FlashMosiFunction = Pin_Function_ThirdAlternate;
static const Ssp    FlashSsp = Ssp0;
static const SpiFlash StorageFlash = SpiFlash_0;

void Storage_initialize();
int8 Storage_initializeFlashMemory(void);
int8 Storage_deinitializeFlashMemory(void);
StorageItem* Storage_getStorageItem();
int8 Storage_getData(uint16 pos, StorageItem *data);
int8 Storage_addData(StorageItem *data);
int8 Storage_removeData(uint16 pos);
int8 Storage_swapData(uint16 pos1, uint16 pos2);
uint16 Storage_getDataCount();
void Storage_clearAllData();
