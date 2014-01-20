/**
 * @file remotecommand.h
 * @author Alexander Rössler
 * @brief Contains anything having to do with the data handling of IRemote
 * @date 13-12-2012
 */

#pragma once

#define REMOTE_COMMAND_MAX_TRANSITIONS 100u
#define REMOTE_COMMAND_MAX_TOLERANCE   0.1

#include <stdio.h>
#include <stdlib.h>
#include <types.h>

typedef enum {
    RemoteCommand_Medium_Ir = 0u,
    RemoteCommand_Medium_433Mhz = 1u,
    RemoteCommand_Medium_868Mhz = 2u
} RemoteCommand_Medium;

typedef struct {
    uint8   version;
    uint16  id;
    uint8   length;
    uint8   medium;
    uint32  frequency;
    uint16  data[REMOTE_COMMAND_MAX_TRANSITIONS];
} RemoteCommand;

/** Creates an empty IR-Command
 *  @return Pointer to the new created command, return NULL on failure.
 */
void RemoteCommand_initialize(RemoteCommand* command);
/** Compares 2 IR-Commands, it is not necessare that the commands are 100% equal (see MAX_TOLERANCE).
 *  @param command1 Command 1
 *  @param command2 Command 2
 *  @return 1 = euqal, 0 unequal
 */
bool RemoteCommand_compare(RemoteCommand *command1, RemoteCommand *command2);
