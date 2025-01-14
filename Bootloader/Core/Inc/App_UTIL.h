/******************************************************************************
 *
 * File Name: APP_UTIL.h For Bootloader
 *
 * Description: header file for helpful functions and global variables used by Bootloader
 *
 * Author: Abdelrahman Elsayed
 *
 *******************************************************************************/
#ifndef INC_APP_UTIL_H_
#define INC_APP_UTIL_H_

/*******************************************************************************
 *                                Includes                                  *
 *******************************************************************************/
#include "stm32f4xx_hal.h"
#include "General_Config.h"
/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/
#define BOOTLOADER_RX_BUFFER_LENGTH     150000


#define BOOTLOADER_MAJOR_VERSION 3
#define BOOTLOADER_MINOR_VERSION 4
#define BOOTLOADER_PATCH_VERSION 5

#define N_ENTER 0
#define ENTER 1
#define APPLICATION_ENTER_FLAG_ADDRESS 0x00
#define BOOTLOADER_UPDATER_ENTER_FLAG_ADDRESS (APPLICATION_ENTER_FLAG_ADDRESS+0x01)

#define BOOTLOADER_START_ADDRESS 0x08040000
#define BOOTLOADER_BINARY_START_ADDRESS BOOTLOADER_START_ADDRESS

#define APP_START_ADDRESS 0x080A0000
#define APP_NO_OF_BYTES_START_ADDRESS APP_START_ADDRESS
#define APP_Digest_START_ADDRESS (APP_START_ADDRESS + 32)
#define APP_BINARY_START_ADDRESS (APP_START_ADDRESS + 0x200)

#define BOOTLOADER_UPDATER_START_ADDRESS 0x080A0000
#define BOOTLOADER_UPDATER_BINARY_START_ADDRESS BOOTLOADER_UPDATER_START_ADDRESS

/*
 * used for Delta Patching
 */
#if DELTA_PATCH_ENABLED == ENABLED
	#define JANPATCH_STREAM     FIL
	#define SEEK_CUR 1
	#define SEEK_END 2
	#define SEEK_SET 0
#endif

/*******************************************************************************
 *                              Functions Prototypes                           *
 *******************************************************************************/
void APP_Logic(void);

#endif /* INC_APP_UTIL_H_ */
