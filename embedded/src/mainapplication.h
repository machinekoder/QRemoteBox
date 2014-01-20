/**
 * @file mainapplication.h
 * @author Alexander Rössler
 * @brief The main application of the project
 * @date 4-01-2012
 */
#pragma once

#define UART_WRITE_BUFFER_SIZE  500u
#define UART_READ_BUFFER_SIZE   100u
#define USE_UART0 0u
#define USE_UART2 0u
#define USE_UART3 0u

#include <wifly.h>
#include <uart.h>
#include <led.h>
#include <timer.h>
#include <led.h>
#include <iap.h>
#include <timeout.h>
#include <button.h>
#include <pwm.h>
#include <crc.h>
#include <pincon.h>
#include <debug.h>
#include <xprintf.h>
#include <switch.h>
#include <generic.h>
#include <button.h>
#include "remotecontrol.h"
#include "storage.h"

typedef enum {
    ApplicationState_Idle = 0u,
    ApplicationState_CaptureIrCommand = 1u,
    ApplicationState_CaptureRadio433MhzCommand = 4u,
    ApplicationState_CaptureRadio868MhzCommand = 5u,
    ApplicationState_RunCommand = 2u,
    ApplicationState_FlashFirmware = 3u,
    ApplicationState_WiFlyTest = 100u
} ApplicationState;

typedef enum {
    NetworkMode_AccessPoint = 0u,
    NetworkMode_Infrastructure = 1u,
    NetworkMode_None = 2u
} NetworkMode;

typedef enum {
        SerialConnection = 0x01u,
        NetworkConnection = 0x02u
    } ActiveConnection;

typedef struct {
    uint8  debugMode;
    uint8  firstStartIdentificator; // if this is not 42 then we have a first start
    uint32 irReceiveTimeout;
    uint32 irSendTimeout;
    uint32 irRepeatCount;
    uint32 radio433ReceiveTimeout;
    uint32 radio433SendTimeout;
    uint32 radio433RepeatCount;
    uint32 radio868ReceiveTimeout;
    uint32 radio868SendTimeout;
    uint32 radio868RepeatCount;
    char   wlanSsid[100];
    char   wlanPhrase[100];
    char   wlanKey[100];
    char   wlanHostname[100];
    uint8  wlanAuth;
    uint8  wlanDhcp;
    char   wlanIp[20];
    char   wlanMask[20];
    char   wlanGateway[20];
    uint8  networkMode;
    char   wlanPrimaryDnsAddress[20];
    char   wlanSecondaryDnsAddress[20];
} ApplicationSettings;

typedef enum {
    LedYellow2 = Led1,
    LedGreen = Led2,
    LedYellow = Led3,
    LedRed = Led4,
    LedDebug = Led5
} ApplicationLed;

static const uint8  LedDebugPort = 1u;
static const uint8  LedDebugPin = 15u;
static const uint8  Led1Port = 1u;
static const uint8  Led1Pin = 10u;
static const uint8  Led2Port = 1u;
static const uint8  Led2Pin = 9u;
static const uint8  Led3Port = 1u;
static const uint8  Led3Pin = 8u;
static const uint8  Led4Port = 1u;
static const uint8  Led4Pin = 4u;
static const uint8          LedDebugEnablePort = 4u;
static const uint8          LedDebugEnablePin = 29u;
static const Switch_Type    LedDebugEnableType = Switch_Type_HighActive;
static const Pin_Mode       LedDebugEnablePinMode = Pin_Mode_PullUp;
static const Pin_OpenDrain  LedDebugEnableOpenDrain = Pin_OpenDrain_Disable;
static const Switch         LedDebugEnableSwitch = Switch4;
static const uint8          LedLatchEnablePort = 1u;
static const uint8          LedLatchEnablePin = 14u;
static const Switch_Type    LedLatchEnableType = Switch_Type_HighActive;
static const Pin_Mode       LedLatchEnablePinMode = Pin_Mode_PullUp;
static const Pin_OpenDrain  LedLatchEnableOpenDrain = Pin_OpenDrain_Disable;
static const Switch         LedLatchEnableSwitch = Switch5;

static const uint8  Button1Port = 2u;
static const uint8  Button1Pin = 6u;
static const uint8  Button2Port = 2u;
static const uint8  Button2Pin = 7u;
static const uint8  Button3Port = 2u;
static const uint8  Button3Pin = 8u;
static const uint32 ButtonClock = 1000u; // 1000kHz
static const uint32 ButtonSampleInterval = 10000u; // 10ms
static const uint32 ButtonTimeout = 100000; // 100ms

static const uint8  WiflyUartTxPort = 2u;
static const uint8  WiflyUartTxPin = 0u;
static const Pin_Function  WiflyUartTxFunction = Pin_Function_SecondAlternate;
static const uint8  WiflyUartRxPort = 2u;
static const uint8  WiflyUartRxPin = 1u;
static const Pin_Function  WiflyUartRxFunction = Pin_Function_SecondAlternate;
static const Uart   WiflyUart = Uart1;
static const uint32 WiflyUartBaudrate = 115200u;
static const uint32 WiflyUartFactoryBaudrate = 9600u;

static const uint8  SerialTxPort = 0u;
static const uint8  SerialTxPin = 0u;
static const Pin_Function SerialTxFunction = Pin_Function_FirstAlternate;
static const uint8  SerialRxPort = 0u;
static const uint8  SerialRxPin = 0u;
static const Pin_Function SerialRxFunction = Pin_Function_FirstAlternate;
static const Uart   SerialUart = Uart0;
static const uint32 SerialBaudrate = 115200u;

int8 MainApplication_initializeHardware(void);
int8 MainApplication_initializeVariables(void);

void MainApplication_mainTask(void);
void MainApplication_ledTask(void);
void MainApplication_buttonTask(void);
