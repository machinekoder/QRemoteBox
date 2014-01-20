#include "storage.h"

static StorageItem storageItem;

static void initializeStorageItem();

void initializeStorageItem()
{
    storageItem.commandHeader.version = 0u;
}

void Storage_initialize()
{   
    Storage_initializeFlashMemory();
    StorageManager_initialize(StorageFlash);
}

int8 Storage_initializeFlashMemory(void)
{
    Gpio_Pair selPair;
    
    Pin_setFunction(FlashSckPort, FlashSckPin, FlashSckFunction);
    Pin_setMode(FlashSckPort, FlashSckPin, Pin_Mode_NoPullUpDown);
    Pin_setFunction(FlashMisoPort, FlashMisoPin, FlashMisoFunction);
    Pin_setMode(FlashMisoPort, FlashMisoPin, Pin_Mode_NoPullUpDown);
    Pin_setFunction(FlashMosiPort, FlashMosiPin, FlashMosiFunction);
    Pin_setMode(FlashMosiPort, FlashMosiPin, Pin_Mode_NoPullUpDown);
    
    selPair.pin = FlashSelPin;
    selPair.port = FlashSelPort;
    
    SpiFlash_initialize(StorageFlash, FlashSsp, selPair);
    
    return (int8)(0);
}

int8 Storage_deinitializeFlashMemory(void)
{
    Pin_setFunction(FlashSckPort, FlashSckPin, Pin_Function_Primary);
    Pin_setFunction(FlashMisoPort, FlashMisoPin, Pin_Function_Primary);
    Pin_setFunction(FlashMosiPort, FlashMosiPin, Pin_Function_Primary);
     
    return (int8)(0); 
}

StorageItem* Storage_getStorageItem()
{
    initializeStorageItem();
    return &storageItem;
}

int8 Storage_getData(uint16 pos, StorageItem *data)
{
    return StorageManager_getData(pos, (uint8*)data, sizeof(StorageItem));
}

int8 Storage_addData(StorageItem *data)
{
    return StorageManager_addData((uint8*)data, sizeof(StorageItem));
}

int8 Storage_removeData(uint16 pos)
{
    return StorageManager_removeData(pos);
}

int8 Storage_swapData(uint16 pos1, uint16 pos2)
{
    return StorageManager_swapData(pos1, pos2);
}

uint16 Storage_getDataCount()
{
    return StorageManager_getDataCount();
}

void Storage_clearAllData()
{
    StorageManager_clearAllData();
}
