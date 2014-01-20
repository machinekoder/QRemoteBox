#include "mainapplication.h"


static ActiveConnection activeConnections;

static RemoteCommand currentCommand;
static ApplicationState applicationState;
static ApplicationSettings applicationSettings;

static int8 startWlanInfrastructureMode(ApplicationSettings *settings);
static int8 startWlanAccessPoint(void);
    
static void printfData(char* fmt, ...);
static int8 putcharData(char c);

static void errorWiFly(void);
static void printUnknownCommand(void);
static void printParameterMissing(void);
static void printAcknowledgement(void);
static void printError(char *message);
static void printAliveMessage(void);
static bool compareBaseCommand(char *original, char *received);
static bool compareExtendedCommand(char *original, char *received);
static void processCommand(char *buffer);
static void processUdpCommand(char *buffer);

static void startState(ApplicationState state);

static void loadDefaultApplicationSettings(void);
static int8 initializeSerialConnection(void);
static int8 initializeNetworkConnection(void);
static void outputCommand(RemoteCommand *command);
static void outputStorageItem(StorageItem *storageItem);

void loadDefaultApplicationSettings(void)
{
        applicationSettings.debugMode = true;
        applicationSettings.irReceiveTimeout = 25000u;
        applicationSettings.irSendTimeout = 50000u;
        applicationSettings.irRepeatCount = 2u;
        applicationSettings.radio433ReceiveTimeout = 9000u;
        applicationSettings.radio433SendTimeout = 10000u;
        applicationSettings.radio433RepeatCount = 10u;
        applicationSettings.radio868ReceiveTimeout = 9000u;
        applicationSettings.radio868SendTimeout = 10000u;
        applicationSettings.radio868RepeatCount = 10u;
        strcpy(applicationSettings.wlanSsid, "TESTNET");
        strcpy(applicationSettings.wlanPhrase, "***");
        strcpy(applicationSettings.wlanKey, "");
        strcpy(applicationSettings.wlanHostname, "QRemoteBox");
        applicationSettings.wlanAuth = 0u;
        applicationSettings.wlanDhcp = 1u;
        strcpy(applicationSettings.wlanIp, "169.254.1.1");
        strcpy(applicationSettings.wlanMask, "255.255.0.0");
        strcpy(applicationSettings.wlanGateway, "169.254.1.2");
        strcpy(applicationSettings.wlanPrimaryDnsAddress, "192.168.1.1");
        strcpy(applicationSettings.wlanSecondaryDnsAddress, "192.168.1.2");
        applicationSettings.networkMode = NetworkMode_AccessPoint;
        
        applicationSettings.firstStartIdentificator = 42u;   // remove the first start indicator    
        Iap_saveApplicationSettings(&applicationSettings, sizeof(ApplicationSettings));  
}

int8 initializeSerialConnection(void)
{
    //TODO no serial connection so far
    return (int8)(-1);
}

int8 initializeNetworkConnection(void)
{
    bool initialized = FALSE;
    bool afterReboot = FALSE;
    
    Pin_setFunction(WiflyUartRxPort, WiflyUartRxPin, WiflyUartRxFunction);          // intialize Uart pins
    Pin_setMode(WiflyUartRxPort, WiflyUartRxPin, Pin_Mode_NoPullUpDown);
    Pin_setFunction(WiflyUartTxPort, WiflyUartTxPin, WiflyUartTxFunction);
    Pin_setMode(WiflyUartTxPort, WiflyUartTxPin, Pin_Mode_NoPullUpDown);
    
    if (WiFly_initialize(WiflyUart, WiflyUartBaudrate) == (int8)(0))
    {
        initialized = TRUE;
    }
    else if (WiFly_initialize(WiflyUart, WiflyUartFactoryBaudrate) == (int8)(0))    // try default settings
    {
        if (WiFly_actionEnterCommandMode(afterReboot) == (int8)(0))                 // Configure the wlan module to the default settings
        {
            WiFly_setUartBaud(WiflyUartBaudrate);
            WiFly_fileIoSaveDefault();
            WiFly_actionReboot();
            if (WiFly_initialize(WiflyUart, WiflyUartBaudrate) == (int8)(-1))
            {
                return (int8)(-1);
            }
            initialized = TRUE;
        }
        else
        {
            return (int8)(-1);
        }
    }
        
    if (initialized ==  TRUE)
    {
        if (WiFly_actionEnterCommandMode(afterReboot) == (int8)(0))      // Configure the wlan module
        {
            //WiFly_setSysPrintlvl(0u);             // Turn off unessesary messages
            //WiFly_setWlanJoin(2u);              // Auto join
            WiFly_setIpRemote(5487u);             // Set the remote port
            //WiFly_setDnsName("QRemoteBox");       // Sets the default hostname
            //WiFly_setDnsBackup("QRemote");        // Sets the backup hostname
            WiFly_setIpProtocol(0x3u);              // TCP and UDP
            WiFly_setIpFlags(0x47u);                // Enable auto pairing
            WiFly_setIpHost("0.0.0.0");             // To allow auto pairing
            WiFly_setBroadcastInterval(0u);         // Disable UDP broadcasts
            WiFly_actionExitCommandMode();
            
            WiFly_setProcessFunction(&processCommand);
            WiFly_setUdpProcessFunction(&processUdpCommand);
            WiFly_setErrorFunction(&errorWiFly);
            
            return (int8)(0);
        }
        else
        {
            return (int8)(-1);
        }
    }
    
    return (int8)(-1);
}

int8 startWlanInfrastructureMode(ApplicationSettings *settings)
{
    if (WiFly_actionEnterCommandMode(FALSE) == (int8)(-1))
        return (int8)(-1);
    if (WiFly_setInfrastructureParams() == (int8)(-1))
        return (int8)(-1);
    if (WiFly_setWlanSsid(settings->wlanSsid) == (int8)(-1))
        return (int8)(-1);
    if (WiFly_setWlanAuth(settings->wlanAuth) == (int8)(-1))
        return (int8)(-1);
    if ((settings->wlanAuth == 1u) || (settings->wlanAuth == 8u))
    {
        if (WiFly_setWlanKey(settings->wlanKey) == (int8)(-1))
            return (int8)(-1);
    }
    else 
    {
        if (WiFly_setWlanPhrase(settings->wlanPhrase) == (int8)(-1))
            return (int8)(-1);
    }
    if (WiFly_setWlanJoin(1u) == (int8)(-1))  // Turn on Auto Join mode
        return (int8)(-1);
    //if (WiFly_setDnsName(settings->wlanHostname) == (int8)(-1))
    //    return (int8)(-1);
    if (WiFly_setIpDhcp(settings->wlanDhcp) == (int8)(-1))
        return (int8)(-1);
    if (WiFly_setIpAddress(settings->wlanIp) == (int8)(-1))
        return (int8)(-1);
    if (WiFly_setIpNetmask(settings->wlanMask) == (int8)(-1))
        return (int8)(-1);
    if (WiFly_setIpGateway(settings->wlanGateway) == (int8)(-1))
        return (int8)(-1);
    if (WiFly_setDnsAddr(settings->wlanPrimaryDnsAddress) == (int8)(-1))
        return (int8)(-1);
    //if (WiFly_setDnsBackup(settings->wlanSecondaryDnsAddress) == (int8)(-1))
    //    return (int8)(-1);
    if (WiFly_fileIoSaveDefault() == (int8)(-1))
        return (int8)(-1);
    
    if (WiFly_actionReboot() == (int8)(-1))
    {
        applicationSettings.networkMode = NetworkMode_None;
        return (int8)(-1);
    }
    else
    {
        applicationSettings.networkMode = NetworkMode_Infrastructure;
        Iap_saveApplicationSettings(&applicationSettings, sizeof(ApplicationSettings));
        return (int8)(0);
    }
}

int8 startWlanAccessPoint(void)
{
    char buffer[100u];
    xsnprintf(buffer, 100u, "QRemoteBox_%u", Iap_readSerial());
    
    if (WiFly_createAccessPoint(buffer) == (int8)(-1))
    {
        applicationSettings.networkMode = NetworkMode_None;
        return (int8)(-1);
    }
    else
    {
        applicationSettings.networkMode = NetworkMode_AccessPoint;
        Iap_saveApplicationSettings(&applicationSettings, sizeof(ApplicationSettings));
        return (int8)(0);
    }
}

void outputCommand(RemoteCommand *command)
{
    uint16 commandSize = sizeof(RemoteCommand);
    
    printfData("*DATA");
    Generic_base64Encode((uint8*)command, commandSize, &putcharData);
    printfData("\r");
}

void outputStorageItem(StorageItem *storageItem)
{
    uint16 commandSize = sizeof(StorageItem);
    
    Generic_base64Encode((uint8*)storageItem, commandSize, &putcharData);
    printfData("\r");
}

void printfData(char* fmt, ... )
{
    va_list arg_ptr;
    
    va_start(arg_ptr,fmt);
    if (activeConnections & NetworkConnection)
    {
        xformat(WiFly_putat, NULL, fmt, arg_ptr);
    }
    else if (activeConnections & SerialConnection)
    {
        xformat(Uart_putat, (void*)&SerialUart, fmt, arg_ptr);
    }
    va_end(arg_ptr);
}

int8 putcharData(char c)
{  
    if (activeConnections & NetworkConnection)
    {
        return WiFly_putchar(c);
    }
    else if (activeConnections & SerialConnection)
    {
        return Uart_putchar(Uart0, c);
    }
    
    return (int8)(-1);
}

void errorWiFly(void)
{
    printfData("ERR: WiFly command too long\r");
}

void printUnknownCommand(void)
{
    printfData("CMD?\r");
}

void printParameterMissing(void)
{
    printfData("ERR: 2few Args\r");
}

void printAcknowledgement(void)
{
    printfData("ACK\r");
}

void printError(char *message)
{
    printfData("ERR: %s\r", message);
}

void printAliveMessage(void)
{
    printfData("yes\r");
}

bool compareBaseCommand(char *original, char *received)
{
    return (strcmp(original,received) == (int)(0));
}

bool compareExtendedCommand(char *original, char *received)
{
    return (((strlen(received) == 1u) 
            && (strncmp(original,received,1u) == (int)(0))) 
            || (strcmp(original,received) == (int)(0)));
}

void processUdpCommand(char *buffer)
{
    if (strcmp("QRC:Broadcast",buffer) == (int)0)
    {
        WiFly_printf("QRC:BoxNotConnected");
    }
}

void processCommand(char *buffer)
{
    char *dataPointer;
    char *savePointer;
    int64 value;
    
    if (buffer[0] == (char)10)  //omit the LF character
    {
        buffer = &(buffer[1]);
    }
    
    if (strlen(buffer) == 0u)
    {
        return;
    }
    
    if (strcmp("QRC:Broadcast",buffer) == (int)0)
    {
        return;
    }
    
    Led_set(LedYellow);  // set the yellow led to indicate incoming data status
    
    dataPointer = strtok_r(buffer, " ", &savePointer);
    
    if (compareBaseCommand("alive", dataPointer))
    {
        // We have a keep alive command
        printAliveMessage();
    }
    else if (compareBaseCommand("run", dataPointer))
    {
        // We have a run command
        dataPointer = strtok_r(NULL, " ", &savePointer);
        if (dataPointer == NULL)
        {
            printParameterMissing();
        }
        else if (compareExtendedCommand("command",dataPointer))
        {
            dataPointer = strtok_r(NULL, " ", &savePointer);
            if (dataPointer != NULL)
            {
                uint16 commandSize = strlen(dataPointer);
                if (Generic_base64Decode(dataPointer, commandSize, (uint8*)(&currentCommand)) == (int8)(0))
                {
                    startState(ApplicationState_RunCommand);
                }
                else
                {
                    printError("Command format wrong");
                }
            }
            else
            {
                printParameterMissing();
            }
        }
        else if (compareExtendedCommand("data",dataPointer))
        {
            dataPointer = strtok_r(NULL, " ", &savePointer);
            if ((dataPointer != NULL) && (xatoi(&dataPointer, &value) == 1u))
            {
                int8 successful;
                StorageItem *storageItem;
                
                storageItem = Storage_getStorageItem();
                RemoteControl_deinitializeRadio433();
                Storage_initializeFlashMemory();
                successful = Storage_getData((uint8)value, storageItem);
                Storage_deinitializeFlashMemory();
                RemoteControl_initializeRadio433();
                
                if (successful == (int8)0)
                {
                    memcpy((void*)(&currentCommand), 
                           (void*)(&(storageItem->remoteCommand)), 
                           sizeof(RemoteCommand));
                    startState(ApplicationState_RunCommand);
                }
                else
                {
                    printError("data not existent");
                }
            }
            else
            {
                printUnknownCommand();
            }
        }
        else
        {
            printUnknownCommand();
        }
        
    }
    else if (compareBaseCommand("capture", dataPointer))
    {
        dataPointer = strtok_r(NULL, " ", &savePointer);
        if (dataPointer == NULL)
        {
            printParameterMissing();
        }
        else if (compareExtendedCommand("ir",dataPointer))
        {
            startState(ApplicationState_CaptureIrCommand);
        }
        else if (compareExtendedCommand("433", dataPointer))
        {
            startState(ApplicationState_CaptureRadio433MhzCommand);
        }
        else if (compareExtendedCommand("868", dataPointer))
        {
            startState(ApplicationState_CaptureRadio868MhzCommand);
        }
        else
        {
            printUnknownCommand();
        }
    }
    else if (compareBaseCommand("stop", dataPointer))
    {
        startState(ApplicationState_Idle);
        printAcknowledgement();
    }
    else if (compareBaseCommand("flash", dataPointer))
    {
        // We have a flash command
        char buffer[100u];
        uint16 receivedChecksum;
        uint16 calculatedChecksum;
        
        dataPointer = strtok_r(NULL, " ", &savePointer);
        if (dataPointer != NULL)
        {
            uint16 commandSize = strlen(dataPointer);
            uint16 i;
            for (i = 0u; i < commandSize; i+=2u)
            {
                buffer[i/2u] = (char)Generic_hex2int(dataPointer+i,2u);
            }
            
            dataPointer = strtok_r(NULL, " ", &savePointer);
            if (dataPointer != NULL)
            {
                receivedChecksum = (uint16)Generic_hex2int(dataPointer,4u);;
                calculatedChecksum = Crc_fast(buffer, 100u);
                if (receivedChecksum == calculatedChecksum)
                {
                    printAcknowledgement();
                }
                else
                {
                    printfData("%u %u %u\r",commandSize,receivedChecksum,calculatedChecksum);
                }
            }
        }
        
    }
    else if (compareBaseCommand("set", dataPointer))
    {
        // starting a set command
        dataPointer = strtok_r(NULL, " ", &savePointer);
        if (dataPointer == NULL)
        {
            printUnknownCommand();
        }
        else if (compareExtendedCommand("wlan",dataPointer))
        {
            // set wlan
            dataPointer = strtok_r(NULL, " ", &savePointer);
            if (dataPointer == NULL)
            {
                printUnknownCommand();
            }
            else if (compareExtendedCommand("ssid",dataPointer))
            {
                // set ssid
                dataPointer = strtok_r(NULL, " ", &savePointer);
                if (dataPointer != NULL)
                {
                    strncpy(applicationSettings.wlanSsid, dataPointer, 100u);
                    printAcknowledgement();
                }
                else
                {
                    printUnknownCommand();
                }
            }
            else if (compareExtendedCommand("phrase",dataPointer))
            {
                // set ssid
                dataPointer = strtok_r(NULL, " ", &savePointer);
                if (dataPointer != NULL)
                {
                    strncpy(applicationSettings.wlanPhrase, dataPointer, 100u);
                    printAcknowledgement();
                }
                else
                {
                    printUnknownCommand();
                }
            }
            else if (compareExtendedCommand("key",dataPointer))
            {
                // set ssid
                dataPointer = strtok_r(NULL, " ", &savePointer);
                if (dataPointer != NULL)
                {
                    strncpy(applicationSettings.wlanKey, dataPointer, 100u);
                    printAcknowledgement();
                }
                else
                {
                    printUnknownCommand();
                }
            }
            else if (compareExtendedCommand("hostname",dataPointer))
            {
                // set hostname
                dataPointer = strtok_r(NULL, " ", &savePointer);
                if (dataPointer != NULL)
                {
                    strncpy(applicationSettings.wlanHostname, dataPointer, 100u);
                    printAcknowledgement();
                }
                else
                {
                    printUnknownCommand();
                }
            }
            else if (compareExtendedCommand("auth",dataPointer))
            {
                // set auth
                dataPointer = strtok_r(NULL, " ", &savePointer);
                if ((dataPointer != NULL) && (xatoi(&dataPointer, &value) == 1u))
                {
                    applicationSettings.wlanAuth = (uint8)value;
                    printAcknowledgement();
                }
                else
                {
                    printUnknownCommand();
                }
            }
            else if (compareExtendedCommand("dhcp",dataPointer))
            {
                // set auth
                dataPointer = strtok_r(NULL, " ", &savePointer);
                if ((dataPointer != NULL) && (xatoi(&dataPointer, &value) == 1u))
                {
                    applicationSettings.wlanDhcp = (uint8)value;
                    printAcknowledgement();
                }
                else
                {
                    printUnknownCommand();
                }
            }
            else if (compareExtendedCommand("ip",dataPointer))
            {
                // set auth
                dataPointer = strtok_r(NULL, " ", &savePointer);
                if (dataPointer != NULL)
                {
                    strncpy(applicationSettings.wlanIp, dataPointer, 20u);
                    printAcknowledgement();
                }
                else
                {
                    printUnknownCommand();
                }
            }
            else if (compareExtendedCommand("mask",dataPointer))
            {
                // set auth
                dataPointer = strtok_r(NULL, " ", &savePointer);
                if (dataPointer != NULL)
                {
                    strncpy(applicationSettings.wlanMask, dataPointer, 20u);
                    printAcknowledgement();
                }
                else
                {
                    printUnknownCommand();
                }
            }
            else if (compareExtendedCommand("gateway",dataPointer))
            {
                // set auth
                dataPointer = strtok_r(NULL, " ", &savePointer);
                if (dataPointer != NULL)
                {
                    strncpy(applicationSettings.wlanGateway, dataPointer, 20u);
                    printAcknowledgement();
                }
                else
                {
                    printUnknownCommand();
                }
            }
            else if (compareExtendedCommand("dns",dataPointer))
            {
                // set auth
                dataPointer = strtok_r(NULL, " ", &savePointer);
                if (dataPointer != NULL)
                {
                    strncpy(applicationSettings.wlanPrimaryDnsAddress, dataPointer, 20u);
                    printAcknowledgement();
                }
                else
                {
                    printUnknownCommand();
                }
            }
            else if (compareExtendedCommand("backupDns",dataPointer))
            {
                // set auth
                dataPointer = strtok_r(NULL, " ", &savePointer);
                if (dataPointer != NULL)
                {
                    strncpy(applicationSettings.wlanSecondaryDnsAddress, dataPointer, 20u);
                    printAcknowledgement();
                }
                else
                {
                    printUnknownCommand();
                }
            }
            else
            {
                printUnknownCommand();
            }
        }
        else if (compareExtendedCommand("ir",dataPointer))
        {
            // set ir
            dataPointer = strtok_r(NULL, " ", &savePointer);
            if (dataPointer == NULL)
            {
                printUnknownCommand();
            }
            else if (compareExtendedCommand("receiveTimeout",dataPointer))
            {
                dataPointer = strtok_r(NULL, " ", &savePointer);
                if ((dataPointer != NULL) && (xatoi(&dataPointer, &value) == 1u))
                {
                    applicationSettings.irReceiveTimeout = (uint32)value*1000u;
                    RemoteControl_setReceiveTimeout(RemoteCommand_Medium_Ir, applicationSettings.irReceiveTimeout);
                    printAcknowledgement();
                }
                else
                {
                    printParameterMissing();
                }
            }
            else if (compareExtendedCommand("sendTimeout",dataPointer))
            {

                dataPointer = strtok_r(NULL, " ", &savePointer);
                if ((dataPointer != NULL) && (xatoi(&dataPointer, &value) == 1u))
                {
                    applicationSettings.irSendTimeout = (uint32)value*1000u;
                    RemoteControl_setSendTimeout(RemoteCommand_Medium_Ir, applicationSettings.irSendTimeout);
                    printAcknowledgement();
                }
                else
                {
                    printParameterMissing();
                }
            }
            else if (compareExtendedCommand("count",dataPointer))
            {

                dataPointer = strtok_r(NULL, " ", &savePointer);
                if ((dataPointer != NULL) && (xatoi(&dataPointer, &value) == 1u))
                {
                    applicationSettings.irRepeatCount = (uint32)value;
                    RemoteControl_setRepeatCount(RemoteCommand_Medium_Ir, applicationSettings.irRepeatCount);
                    printAcknowledgement();
                }
                else
                {
                    printParameterMissing();
                }
            }
            else
            {
                printUnknownCommand();
            }
        }
        else if (compareExtendedCommand("433",dataPointer))
        {
            // set ir
            dataPointer = strtok_r(NULL, " ", &savePointer);
            if (dataPointer == NULL)
            {
                printUnknownCommand();
            }
            else if (compareExtendedCommand("receiveTimeout",dataPointer))
            {

                dataPointer = strtok_r(NULL, " ", &savePointer);
                if ((dataPointer != NULL) && (xatoi(&dataPointer, &value) == 1u))
                {
                    applicationSettings.radio433ReceiveTimeout = (uint32)value*1000u;
                    RemoteControl_setReceiveTimeout(RemoteCommand_Medium_433Mhz, applicationSettings.radio433ReceiveTimeout);
                    printAcknowledgement();
                }
                else
                {
                    printParameterMissing();
                }
            }
            else if (compareExtendedCommand("sendTimeout",dataPointer))
            {

                dataPointer = strtok_r(NULL, " ", &savePointer);
                if ((dataPointer != NULL) && (xatoi(&dataPointer, &value) == 1u))
                {
                    applicationSettings.radio433SendTimeout = (uint32)value*1000u;
                    RemoteControl_setSendTimeout(RemoteCommand_Medium_433Mhz, applicationSettings.radio433SendTimeout);
                    printAcknowledgement();
                }
                else
                {
                    printParameterMissing();
                }
            }
            else if (compareExtendedCommand("count",dataPointer))
            {

                dataPointer = strtok_r(NULL, " ", &savePointer);
                if ((dataPointer != NULL) && (xatoi(&dataPointer, &value) == 1u))
                {
                    applicationSettings.radio433RepeatCount = (uint32)value;
                    RemoteControl_setRepeatCount(RemoteCommand_Medium_433Mhz, applicationSettings.radio433RepeatCount);
                    printAcknowledgement();
                }
                else
                {
                    printParameterMissing();
                }
            }
            else
            {
                printUnknownCommand();
            }
        }
        else if (compareExtendedCommand("868",dataPointer))
        {
            // set ir
            dataPointer = strtok_r(NULL, " ", &savePointer);
            if (dataPointer == NULL)
            {
                printUnknownCommand();
            }
            else if (compareExtendedCommand("receiveTimeout",dataPointer))
            {

                dataPointer = strtok_r(NULL, " ", &savePointer);
                if ((dataPointer != NULL) && (xatoi(&dataPointer, &value) == 1u))
                {
                    applicationSettings.radio868ReceiveTimeout = (uint32)value*1000u;
                    RemoteControl_setReceiveTimeout(RemoteCommand_Medium_868Mhz, applicationSettings.radio868ReceiveTimeout);
                    printAcknowledgement();
                }
                else
                {
                    printParameterMissing();
                }
            }
            else if (compareExtendedCommand("sendTimeout",dataPointer))
            {

                dataPointer = strtok_r(NULL, " ", &savePointer);
                if ((dataPointer != NULL) && (xatoi(&dataPointer, &value) == 1u))
                {
                    applicationSettings.radio868SendTimeout = (uint32)value*1000u;
                    RemoteControl_setSendTimeout(RemoteCommand_Medium_868Mhz, applicationSettings.radio868SendTimeout);
                    printAcknowledgement();
                }
                else
                {
                    printParameterMissing();
                }
            }
            else if (compareExtendedCommand("count",dataPointer))
            {

                dataPointer = strtok_r(NULL, " ", &savePointer);
                if ((dataPointer != NULL) && (xatoi(&dataPointer, &value) == 1u))
                {
                    applicationSettings.radio868RepeatCount = (uint32)value;
                    RemoteControl_setRepeatCount(RemoteCommand_Medium_868Mhz, applicationSettings.radio868RepeatCount);
                    printAcknowledgement();
                }
                else
                {
                    printParameterMissing();
                }
            }
            else
            {
                printUnknownCommand();
            }
        }
        else if (compareExtendedCommand("debugMode",dataPointer))
        {
            dataPointer = strtok_r(NULL, " ", &savePointer);
            if ((dataPointer != NULL) && (xatoi(&dataPointer, &value) == 1u))
            {
                if ((uint8)value > 1u)
                {
                    printError("Value out of range");
                }
                else
                {
                    applicationSettings.debugMode = (uint8)value;
                    Switch_set(LedDebugEnableSwitch, value);
                    printAcknowledgement();
                }
            }
            else
            {
                printParameterMissing();
            }
        }
        else
        {
            printUnknownCommand();
        }
    }
    else if (compareBaseCommand("get", dataPointer))            // starting a get command
    {
        dataPointer = strtok_r(NULL, " ", &savePointer);
        if (dataPointer == NULL)
        {
            printUnknownCommand();
        }
        else if (compareExtendedCommand("wlan",dataPointer))
        {
            // get wlan
            dataPointer = strtok_r(NULL, " ", &savePointer);
            if (dataPointer == NULL)
            {
                printUnknownCommand();
            }
            else if (compareExtendedCommand("ssid",dataPointer))
            {
                printfData("%s\r", applicationSettings.wlanSsid);
                printAcknowledgement();
            }
            else if (compareExtendedCommand("phrase",dataPointer))
            {
                printfData("%s\r", applicationSettings.wlanPhrase);
                printAcknowledgement();
            }
            else if (compareExtendedCommand("key",dataPointer))
            {
                printfData("%s\r", applicationSettings.wlanKey);
                printAcknowledgement();
            }
            else if (compareExtendedCommand("hostname",dataPointer))
            {
                printfData("%s\r", applicationSettings.wlanHostname);
                printAcknowledgement();
                return;
            }
            else if (compareExtendedCommand("auth",dataPointer))
            {
                printfData("%u\r", applicationSettings.wlanAuth);
                printAcknowledgement();
            }
            else if (compareExtendedCommand("dhcp",dataPointer))
            {
                printfData("%u\r", applicationSettings.wlanDhcp);
                printAcknowledgement();
            }
            else if (compareExtendedCommand("ip",dataPointer))
            {
                printfData("%s\r", applicationSettings.wlanIp);
                printAcknowledgement();
            }
            else if (compareExtendedCommand("mask",dataPointer))
            {
                printfData("%s\r", applicationSettings.wlanMask);
                printAcknowledgement();
            }
            else if (compareExtendedCommand("gateway",dataPointer))
            {
                printfData("%s\r", applicationSettings.wlanGateway);
                printAcknowledgement();
            }
            else if (compareExtendedCommand("dns",dataPointer))
            {
                printfData("%s\r", applicationSettings.wlanPrimaryDnsAddress);
                printAcknowledgement();
            }
            else if (compareExtendedCommand("backupDns",dataPointer))
            {
                printfData("%s\r", applicationSettings.wlanSecondaryDnsAddress);
                printAcknowledgement();
            }
            else
            {
                printUnknownCommand();
            }
        }
        else if (compareExtendedCommand("ir",dataPointer))
        {
            // set ir
            dataPointer = strtok_r(NULL, " ", &savePointer);
            if (dataPointer == NULL)
            {
                printUnknownCommand();
            }
            else if (compareExtendedCommand("receiveTimeout",dataPointer))
            {
                printfData("%u\r", (uint32)(applicationSettings.irReceiveTimeout/1000u));
                printAcknowledgement();
            }
            else if (compareExtendedCommand("sendTimeout",dataPointer))
            {
                printfData("%u\r", (uint32)(applicationSettings.irSendTimeout/1000u));
                printAcknowledgement();
            }
            else if (compareExtendedCommand("count",dataPointer))
            {
                printfData("%u\r", applicationSettings.irRepeatCount);
                printAcknowledgement();
            }
            else
            {
                printUnknownCommand();
            }
        }
        else if (compareExtendedCommand("433",dataPointer))
        {
            // set ir
            dataPointer = strtok_r(NULL, " ", &savePointer);
            if (dataPointer == NULL)
            {
                printUnknownCommand();
            }
            else if (compareExtendedCommand("receiveTimeout",dataPointer))
            {
                printfData("%u\r", applicationSettings.radio433ReceiveTimeout/1000u);
                printAcknowledgement();
            }
            else if (compareExtendedCommand("sendTimeout",dataPointer))
            {
                printfData("%u\r", applicationSettings.radio433SendTimeout/1000u);
                printAcknowledgement();
            }
            else if (compareExtendedCommand("count",dataPointer))
            {
                printfData("%u\r", applicationSettings.radio433RepeatCount);
                printAcknowledgement();
            }
            else
            {
                printUnknownCommand();
            }
        }
        else if (compareExtendedCommand("868",dataPointer))
        {
            // set ir
            dataPointer = strtok_r(NULL, " ", &savePointer);
            if (dataPointer == NULL)
            {
                printUnknownCommand();
            }
            else if (compareExtendedCommand("receiveTimeout",dataPointer))
            {
                printfData("%u\r", applicationSettings.radio868ReceiveTimeout/1000u);
                printAcknowledgement();
            }
            else if (compareExtendedCommand("sendTimeout",dataPointer))
            {
                printfData("%u\r", applicationSettings.radio868SendTimeout/1000u);
                printAcknowledgement();
            }
            else if (compareExtendedCommand("count",dataPointer))
            {
                printfData("%u\r", applicationSettings.radio868RepeatCount);
                printAcknowledgement();
            }
            else
            {
                printUnknownCommand();
            }
        }
        else if (compareExtendedCommand("debugMode",dataPointer))
        {
            printfData("%u\r", applicationSettings.debugMode);
            printAcknowledgement();
        }
        else
        {
            printUnknownCommand();
        }
    }
    else if (compareBaseCommand("start", dataPointer))
    {
        // starting a start command
        dataPointer = strtok_r(NULL, " ", &savePointer);
        if (dataPointer == NULL)
        {
            printUnknownCommand();
        }
        else if (compareExtendedCommand("wlan",dataPointer))
        {
            // set wlan
            dataPointer = strtok_r(NULL, " ", &savePointer);
            if (dataPointer == NULL)
            {
                printUnknownCommand();
            }
            else if (compareExtendedCommand("ap",dataPointer))
            {
                if (startWlanAccessPoint() == (int8)(0))
                {
                    printAcknowledgement();
                }
                else
                {
                    printError("entering adhoc mode failed");
                }
            }
            else if (compareExtendedCommand("infrastructure",dataPointer))
            {
                if (startWlanInfrastructureMode(&applicationSettings) == (int8)(0))
                {
                    printAcknowledgement();
                }
                else
                {
                    printError("entering infrastructure mode failed");
                }
            }
            else
            {
                printUnknownCommand();
            }
        }
        else if (compareExtendedCommand("flash", dataPointer))
        {
            startState(ApplicationState_FlashFirmware);
            printAcknowledgement();
        }
        else
        {
            printUnknownCommand();
        }
    }
    else if (compareBaseCommand("save", dataPointer))
    {
        // starting a save command
        dataPointer = strtok_r(NULL, " ", &savePointer);
        if (dataPointer == NULL)
        {
            printUnknownCommand();
        }
        else if (compareExtendedCommand("config",dataPointer))
        {
            // save config
            if (Iap_saveApplicationSettings(&applicationSettings, sizeof(ApplicationSettings)) == (int8)0)
            {
                printAcknowledgement();
            }
            else
            {
                printError("saving settings failed");
            }
        }
        else
        {
            printUnknownCommand();
        }
    }
    else if (compareBaseCommand("data", dataPointer))
    {
        dataPointer = strtok_r(NULL, " ", &savePointer);
        if (dataPointer == NULL)
        {
            printParameterMissing();
        }
        else if (compareExtendedCommand("add",dataPointer))
        {
            dataPointer = strtok_r(NULL, " ", &savePointer);
            if (dataPointer != NULL)
            {
                uint16 commandSize;
                StorageItem *storageItem;
                
                commandSize = strlen(dataPointer);
                storageItem = Storage_getStorageItem();
                if (Generic_base64Decode(dataPointer, commandSize, (uint8*)(storageItem)) == (int8)(0))
                {
                    int8 successful;
                    
                    RemoteControl_deinitializeRadio433();
                    Storage_initializeFlashMemory();
                    successful = Storage_addData(storageItem);
                    Storage_deinitializeFlashMemory();
                    RemoteControl_initializeRadio433();
                    
                    if (successful == (int8)0)
                    {
                        printAcknowledgement();
                    }
                    else
                    {
                        printError("adding data failed");
                    }
                }
                else
                {
                    printError("data format wrong");
                }
            }
            else
            {
                printParameterMissing();
            }
        }
        else if (compareExtendedCommand("get",dataPointer))
        {
            dataPointer = strtok_r(NULL, " ", &savePointer);
            if (dataPointer == NULL)
            {
                printParameterMissing();
            }
            else if (compareExtendedCommand("name",dataPointer))
            {
                dataPointer = strtok_r(NULL, " ", &savePointer);
                if ((dataPointer != NULL) && (xatoi(&dataPointer, &value) == 1u))
                {
                    int8 successful;
                    StorageItem *storageItem;
                    
                    storageItem = Storage_getStorageItem();
                    RemoteControl_deinitializeRadio433();
                    Storage_initializeFlashMemory();
                    successful = Storage_getData((uint16)value, storageItem);
                    Storage_deinitializeFlashMemory();
                    RemoteControl_initializeRadio433();
                    
                    if (successful == (int8)0)
                    {
                        printfData("%s\r", storageItem->commandHeader.commandName);
                        printAcknowledgement();
                    }
                    else
                    {
                        printError("data not existent");
                    }
                }
                else
                {
                    printUnknownCommand();
                }
            }
            else if (compareExtendedCommand("data",dataPointer))
            {
                dataPointer = strtok_r(NULL, " ", &savePointer);
                if ((dataPointer != NULL) && (xatoi(&dataPointer, &value) == 1u))
                {
                    int8 successful;
                    StorageItem *storageItem;
                    
                    storageItem = Storage_getStorageItem();
                    RemoteControl_deinitializeRadio433();
                    Storage_initializeFlashMemory();
                    successful = Storage_getData((uint16)value, storageItem);
                    Storage_deinitializeFlashMemory();
                    RemoteControl_initializeRadio433();
                    
                    if (successful == (int8)0)
                    {
                        outputStorageItem(storageItem);
                        printAcknowledgement();
                    }
                    else
                    {
                        printError("data not existent");
                    }
                }
                else
                {
                    printUnknownCommand();
                }
            }
            else if (compareExtendedCommand("count",dataPointer))
            {
                printfData("%u\r", Storage_getDataCount());
                printAcknowledgement();
            }
            else
            {
                printUnknownCommand();
            }
        }
        else if (compareExtendedCommand("swap", dataPointer))
        {
            dataPointer = strtok_r(NULL, " ", &savePointer);
            if ((dataPointer != NULL) && (xatoi(&dataPointer, &value) == 1u))
            {
                uint16 pos1;
                
                pos1 = (uint16)value;
                
                dataPointer = strtok_r(NULL, " ", &savePointer);
                if ((dataPointer != NULL) && (xatoi(&dataPointer, &value) == 1u))
                {
                    int8 successful;
                    uint16 pos2;
                    
                    pos2 = (uint16)value;
                    
                    RemoteControl_deinitializeRadio433();
                    Storage_initializeFlashMemory();
                    successful = Storage_swapData(pos1, pos2);
                    Storage_deinitializeFlashMemory();
                    RemoteControl_initializeRadio433();
                    
                    if (successful == (int8)0)
                    {
                        printAcknowledgement();
                    }
                    else
                    {
                        printError("data not existent");
                    }
                }
                else
                {
                    printUnknownCommand();
                }
            }
            else
            {
                printUnknownCommand();
            }
            
        }
        else if (compareExtendedCommand("remove",dataPointer))
        {
            dataPointer = strtok_r(NULL, " ", &savePointer);
            if ((dataPointer != NULL) && (xatoi(&dataPointer, &value) == 1u))
            {
                int8 successful;
                
                RemoteControl_deinitializeRadio433();
                Storage_initializeFlashMemory();
                successful = Storage_removeData((uint16)value);
                Storage_deinitializeFlashMemory();
                RemoteControl_initializeRadio433();
                
                if (successful == (int8)0)
                {
                    printAcknowledgement();
                }
                else
                {
                    printError("data not existent");
                }
            }
            else
            {
                printUnknownCommand();
            }
        }
        else if (compareExtendedCommand("eraseAll",dataPointer))
        {
            RemoteControl_deinitializeRadio433();
            Storage_initializeFlashMemory();
            Storage_clearAllData();
            Storage_deinitializeFlashMemory();
            RemoteControl_initializeRadio433();
            printAcknowledgement();
        }
        else
        {
            printUnknownCommand();
        }
    }
    else if (compareBaseCommand("factory", dataPointer))
    {
        dataPointer = strtok_r(NULL, " ", &savePointer);
        if (dataPointer == NULL)
        {
            printUnknownCommand();
        }
        else if (compareExtendedCommand("RESET",dataPointer))
        {
            RemoteControl_deinitializeRadio433();
            Storage_initializeFlashMemory();
            Storage_clearAllData();
            Storage_deinitializeFlashMemory();
            RemoteControl_initializeRadio433();
            loadDefaultApplicationSettings();
            printAcknowledgement();
        }
    }
    else if (compareBaseCommand("test", dataPointer))
    {
        dataPointer = strtok_r(NULL, " ", &savePointer);
        if (dataPointer == NULL)
        {
            printUnknownCommand();
        }
        else if (compareExtendedCommand("wifly",dataPointer))
        {
            startState(ApplicationState_WiFlyTest);
        }
        else
        {
            printUnknownCommand();
        }
    }
    else
    {
        printUnknownCommand();
    }
    
    Led_clear(LedYellow);
}

void startState(ApplicationState state)
{
    if (applicationState == state)              // If we are already in this state => ignore
    {
        return;
    }
    
    if ((state != ApplicationState_Idle) 
        && (applicationState != ApplicationState_Idle))  // only changes beetween idle and non idle are possible
    {
        return;
    }
    
    if (state == ApplicationState_Idle)
    {
        if ((applicationState == ApplicationState_CaptureIrCommand)
            || (applicationState == ApplicationState_CaptureRadio433MhzCommand)
            || (applicationState == ApplicationState_CaptureRadio868MhzCommand))
        {
            RemoteControl_stopCapture();
        }
        else if (applicationState == ApplicationState_RunCommand)
        {
            RemoteControl_stopCommand();
        }
        applicationState = ApplicationState_Idle;
        
        printfData("Idle\r");
    }
    else if (state == ApplicationState_CaptureIrCommand)
    {
        applicationState = ApplicationState_CaptureIrCommand;
        
        printfData("Capturing IR data\r");
        Led_blink2(LedYellow);
        RemoteControl_startCapture(RemoteCommand_Medium_Ir);
    }
    else if (state == ApplicationState_CaptureRadio433MhzCommand)
    {
        applicationState = ApplicationState_CaptureIrCommand;
        
        printfData("Capturing 433MHz data\r");
        Led_blink2(LedYellow);
        RemoteControl_startCapture(RemoteCommand_Medium_433Mhz);
    }
    else if (state == ApplicationState_CaptureRadio868MhzCommand)
    {
        applicationState = ApplicationState_CaptureIrCommand;
        
        printfData("Capturing 868MHz data\r");
        Led_blink2(LedYellow);
        RemoteControl_startCapture(RemoteCommand_Medium_868Mhz);
    }
    else if (state == ApplicationState_RunCommand)
    {
        applicationState = ApplicationState_RunCommand;
                
        printfData("Running command\r");
        Led_blink(LedYellow);
        RemoteControl_runCommand(&currentCommand);
    }
    else if (state == ApplicationState_FlashFirmware)
    {
        Led_setAll();
        
    }
    else if (state == ApplicationState_WiFlyTest)
    {
        applicationState = ApplicationState_WiFlyTest;
        
        printfData("Going into WiFly Test state, all uart in and outputs will redirected.\n");
    }
    
    return;
}

int8 MainApplication_initializeHardware(void)
{
    Crc_initialize();              // init crc function for firmware flashing
    Timeout_initialize(Timer1);    // initialize Timer1 for general timeout functions
    Gpio_initialize();
    Led_initialize();
    Switch_initialize();
    
    Led_initializeLed(Led1, Led1Port, Led1Pin, Led_LowActive_No);  // led 1
    Led_initializeLed(Led2, Led2Port, Led2Pin, Led_LowActive_No);  // led 2
    Led_initializeLed(Led3, Led3Port, Led3Pin, Led_LowActive_No);  // led 3
    Led_initializeLed(Led4, Led4Port, Led4Pin, Led_LowActive_No);  // led 4
    Led_initializeLed(Led5, LedDebugPort, LedDebugPin, Led_LowActive_No); // led 5 - debug led
    Switch_initializeSwitch(LedDebugEnableSwitch,
                      LedDebugEnablePort, LedDebugEnablePin,
                      LedDebugEnableType, LedDebugEnablePinMode, LedDebugEnableOpenDrain);
    Switch_initializeSwitch(LedLatchEnableSwitch,
                      LedLatchEnablePort, LedLatchEnablePin,
                      LedLatchEnableType, LedLatchEnablePinMode, LedLatchEnableOpenDrain);
    Switch_on(LedDebugEnableSwitch); // Disable Debug LEDs
    Switch_on(LedLatchEnableSwitch); // Enable LED Latch
    
    Led_clearAll();
    Led_set(LedYellow);     // Inidicate LED and switch initialization
    
    Button_initialize(ButtonClock, ButtonSampleInterval, ButtonTimeout, Timer2);  //1000kHz timer, 10ms sampling, 100ms timeout
    Button_initializeButton(Button1, Button1Port, Button1Pin, Button_Type_LowActive);
    Button_initializeButton(Button2, Button2Port, Button2Pin, Button_Type_LowActive);
    Button_initializeButton(Button3, Button3Port, Button3Pin, Button_Type_LowActive);
   
    //Indicate that buttons and leds are now initialized
    Led_set(LedYellow2);
    
    // intialize the remotecontrol peripherals
    RemoteControl_initialize(); 
    
    // initialize the flash memory, needs to be done after remote control initialization
    RemoteControl_deinitializeRadio433();
    Storage_initialize();
    
    // initialize Network Connections
    initializeSerialConnection();
    if (initializeNetworkConnection() == (int8)(-1))
    {
        printfData("ERR: Network initalization failed");
        Led_set(LedDebug);
        return (int8)(-1);
    }
    
    Led_clear(LedYellow2);   //onboard we came through the hardware initialization
    Led_clear(LedYellow);
    Led_set(LedRed);
    Led_set(LedGreen);
    
    return (int8)(0);
}

int8 MainApplication_initializeVariables(void )
{    
    // init libs
    Generic_buildBase64DecodingTable();

    // init variables
    activeConnections = 0u;
    applicationState = ApplicationState_Idle;
    RemoteCommand_initialize(&currentCommand);
    
    //load settings....
    Iap_loadApplicationSettings(&applicationSettings, sizeof(ApplicationSettings));
    
    if (applicationSettings.firstStartIdentificator != 42u) // load default settings
    {
        loadDefaultApplicationSettings();
    }
    
    RemoteControl_setReceiveTimeout(RemoteCommand_Medium_Ir, applicationSettings.irReceiveTimeout);
    RemoteControl_setSendTimeout(RemoteCommand_Medium_Ir, applicationSettings.irSendTimeout);
    RemoteControl_setRepeatCount(RemoteCommand_Medium_Ir, applicationSettings.irRepeatCount);
    RemoteControl_setReceiveTimeout(RemoteCommand_Medium_433Mhz, applicationSettings.radio433ReceiveTimeout);
    RemoteControl_setSendTimeout(RemoteCommand_Medium_433Mhz, applicationSettings.radio433SendTimeout);
    RemoteControl_setRepeatCount(RemoteCommand_Medium_433Mhz, applicationSettings.radio433RepeatCount);
    RemoteControl_setReceiveTimeout(RemoteCommand_Medium_868Mhz, applicationSettings.radio868ReceiveTimeout);
    RemoteControl_setSendTimeout(RemoteCommand_Medium_868Mhz, applicationSettings.radio868SendTimeout);
    RemoteControl_setRepeatCount(RemoteCommand_Medium_868Mhz, applicationSettings.radio868RepeatCount);
    
    Switch_set(LedDebugEnableSwitch, applicationSettings.debugMode);
    
    if (applicationSettings.networkMode == NetworkMode_Infrastructure)
    {
        startWlanInfrastructureMode(&applicationSettings); // Start the infrastructure mode
    }
    else
    {
        startWlanAccessPoint();   // Start the adhoc mode 
    }
    
    return (int8)(0);
}

void MainApplication_mainTask(void)
{
    //Uart_processTask0();     // serial task
    WiFly_processTask();     // wifly task
    
    if (WiFly_isConnected()) // keeps track of the connectedness
    {
        activeConnections |= NetworkConnection;
    }
    else
    {
        activeConnections &= ~NetworkConnection;
        startState(ApplicationState_Idle);
    }
            
    if ((applicationState == ApplicationState_CaptureIrCommand)
            || (applicationState == ApplicationState_CaptureRadio433MhzCommand)
            || (applicationState == ApplicationState_CaptureRadio868MhzCommand)
    )
    {
        if (RemoteControl_isCaptureFinished() == true)
        {
            RemoteCommand *receivedCommand;
            receivedCommand = RemoteControl_command();
            if (receivedCommand != NULL)    // We finally received something
            {
                outputCommand(receivedCommand);
                startState(ApplicationState_Idle);
            }
        }
    }
    else if (applicationState == ApplicationState_RunCommand)
    {
        if (!RemoteControl_isCommandRunning())    // Command has finished
        {
            startState(ApplicationState_Idle);
        }
    }
    else if (applicationState == ApplicationState_WiFlyTest)
    {
        static char receivedData;
        
        while (Uart_getchar(Uart0, &receivedData) == (int8)(0))
        {
            WiFly_putchar(receivedData);
        }
        while (WiFly_getchar(&receivedData) == (int8)(0))
        {
            Uart_putchar(Uart0, receivedData);
        }
    }
}

void MainApplication_ledTask(void)
{
    WiFly_Status wiflyStatus;
    
    if ((activeConnections & NetworkConnection))
    {
        Led_set(LedGreen);  // Green LED
        Led_clear(LedYellow);
        Led_clear(LedYellow2);
        Led_clear(LedRed);
        //Led_clear(LedRed);
    }
    else
    {
        Led_clear(LedGreen);
        
        if (applicationSettings.networkMode == NetworkMode_AccessPoint)
        {
            Led_toggle(LedYellow2);   // Green LED
            //Led_clear(LedRed);
        }
        else if (applicationSettings.networkMode == NetworkMode_Infrastructure)
        {
            Led_set(LedYellow2);
        }
        else if (applicationSettings.networkMode == NetworkMode_None)
        {
            Led_clear(LedYellow2);
            Led_set(LedRed);
        }
        
        if (applicationSettings.networkMode != NetworkMode_None)
        {
            wiflyStatus = WiFly_getStatus();
            
            if (wiflyStatus.wlanAssociationState == WiFly_WlanAssociationState_NotOk)
            {
                Led_set(LedRed);
                Led_clear(LedYellow);
            }
            else 
            {
                Led_clear(LedRed);
                if (wiflyStatus.wlanAuthenticationState == WiFly_WlanAuthenticationState_NotOk)
                {
                    Led_toggle(LedYellow);
                }
                else 
                {
                    Led_set(LedYellow);
                }
            }
        }
        
    }
    
    //Led_clear(LedYellow);    // clear the yellow led in case it is still running
}

void MainApplication_buttonTask(void )
{
    ButtonValue buttonValue;
    if (Button_getPress(&buttonValue) == (int8)(0))
    {
        //printfData("pressed %u, %u\r", buttonValue.id, buttonValue.count);
        if ((buttonValue.id == Button1) && (buttonValue.count == 1u))
        {
            Led_clearAll();
            Led_set(LedYellow);
            Led_set(LedRed);
            if (applicationSettings.networkMode == NetworkMode_AccessPoint)
            {
                startWlanInfrastructureMode(&applicationSettings);
            }
            else 
            {
                startWlanAccessPoint();
            }
        }
        else if ((buttonValue.id == Button2) && (buttonValue.count == 1u))
        {
            // TODO: do something
        }
        else if ((buttonValue.id == Button3) && (buttonValue.count == 1u))
        {
            // TODO: do something
        }
    }
}
