/******************************************************************************
 *
 * File Name: APP_UTIL.c
 *
 * Description: Source file for helpful functions and global variables used by application
 *
 * Author: Abdelrahman Elsayed
 *
 *******************************************************************************/


/*******************************************************************************
 *                              Includes                       					*
 *******************************************************************************/
#include"App_UTIL.h"
#include "usart.h"
#include "rtc.h"

#if DELTA_PATCH_ENABLED == ENABLED
	#include "fatfs.h"
	#include "fatfs_sd.h"
	#include "janpatch.h"
#endif

/*******************************************************************************
 *                      Static global Variables		                            *
 *******************************************************************************/
//0: No application/BL Updater valid
//1: Application is last written in shared memory
//2: BL Updater is last written in shared memory
static uint8_t Last_written_image = 0 ;

static uint8_t Bootloader_Rx_Buffer[BOOTLOADER_RX_BUFFER_LENGTH];

/*******************************************************************************
 *                      Static user define types		                        *
 *******************************************************************************/
static enum Bootloader_Supported_Commands{
	BOOTLOADER_GET_VERION_COMMAND,
	BOOTLOADER_MEM_WRITE_APP_COMMAND,
	BOOTLOADER_MEM_ERASE_APP_COMMAND,
	BOOTLOADER_MEM_WRITE_BOOTLOADER_UPDATER_COMMAND,
	BOOTLOADER_MEM_ERASE_BOOTLOADER_UPDATER_COMMAND,
	BOOTLOADER_LEAVING_TO_BOOT_MANAGER_COMMAND,
	BOOTLOADER_DELTA_PATCHING_COMMAND

};


/*******************************************************************************
 *                      Static Functions Definitions                            *
 *******************************************************************************/
static void Write_RTC_backup_reg(uint32_t reg ,uint32_t data){
    HAL_PWR_EnableBkUpAccess();
    HAL_RTCEx_BKUPWrite(&hrtc, reg, data);
    HAL_PWR_DisableBkUpAccess();

}

/*
 * jump to specific address
 */
static void jump_to_Image_Address(uint32_t start_addr){

	/* Set the main stack pointer to to the application start address */
	__set_MSP(*(uint32_t *)start_addr);
	//__set_PSP(*(uint32_t *)start_addr);

	/* Get the main application start address */
	uint32_t jump_address = *(uint32_t *)(start_addr + 4);

	// Create function pointer for the main application
	void (*app_ptr)(void);
	app_ptr = (void *)(jump_address);

	// Now jump to the main application
	app_ptr();
}

/*
 * get flash sector number based on passed address
 */
static uint32_t GetSector(uint32_t Address)
{
	uint32_t sector = 0;

	/* BANK 1 */
	if((Address >= 0x08000000 ) && (Address < 0x08003FFF))
	{
		sector = FLASH_SECTOR_0;
	}

	else if((Address >= 0x08004000 ) && (Address <  0x08007FFF))
	{
		sector = FLASH_SECTOR_1;
	}

	else if((Address >= 0x08008000) && (Address <  0x0800BFFF))
	{
		sector = FLASH_SECTOR_2;
	}

	else if((Address >= 0x0800C000) && (Address < 0x0800FFFF))
	{
		sector = FLASH_SECTOR_3;
	}

	else if((Address >=  0x08010000) && (Address <  0x0801FFFF))
	{
		sector = FLASH_SECTOR_4;
	}

	else if((Address >=  0x08020000) && (Address < 0x0803FFFF))
	{
		sector = FLASH_SECTOR_5;
	}

	else if((Address >= 0x08040000) && (Address < 0x0805FFFF))
	{
		sector = FLASH_SECTOR_6;
	}

	else if((Address >= 0x08060000) && (Address < 0x0807FFFF))
	{
		sector = FLASH_SECTOR_7;
	}

	else if((Address >= 0x08080000) && (Address < 0x0809FFFF))
	{
		sector = FLASH_SECTOR_8;
	}

	else if((Address >= 0x080A0000) && (Address < 0x080BFFFF))
	{
		sector = FLASH_SECTOR_9;
	}

	else if((Address >= 0x080C0000) && (Address < 0x080DFFFF))
	{
		sector = FLASH_SECTOR_10;
	}

	else if((Address >= 0x080E0000) && (Address < 0x080FFFFF))
	{
		sector = FLASH_SECTOR_11;
	}

	/* BANK 2 */
	if((Address >= 0x08100000 ) && (Address < 0x08103FFF))
	{
		sector = FLASH_SECTOR_12;
	}

	else if((Address >= 0x08104000 ) && (Address <  0x08107FFF))
	{
		sector = FLASH_SECTOR_13;
	}

	else if((Address >= 0x08108000) && (Address <  0x0810BFFF))
	{
		sector = FLASH_SECTOR_14;
	}

	else if((Address >= 0x0810C000) && (Address < 0x0810FFFF))
	{
		sector = FLASH_SECTOR_15;
	}

	else if((Address >=  0x08110000) && (Address <  0x0811FFFF))
	{
		sector = FLASH_SECTOR_16;
	}

	else if((Address >=  0x08120000) && (Address < 0x0813FFFF))
	{
		sector = FLASH_SECTOR_17;
	}

	else if((Address >= 0x08140000) && (Address < 0x0815FFFF))
	{
		sector = FLASH_SECTOR_18;
	}

	else if((Address >= 0x08160000) && (Address < 0x0817FFFF))
	{
		sector = FLASH_SECTOR_19;
	}

	else if((Address >= 0x08180000) && (Address < 0x0819FFFF))
	{
		sector = FLASH_SECTOR_20;
	}

	else if((Address >= 0x081A0000) && (Address < 0x081BFFFF))
	{
		sector = FLASH_SECTOR_21;
	}

	else if((Address >= 0x081C0000) && (Address < 0x081DFFFF))
	{
		sector = FLASH_SECTOR_22;
	}

	else if((Address >= 0x081E0000) && (Address < 0x081FFFFF))
	{
		sector = FLASH_SECTOR_23;
	}
	return sector;
}

/*
 * erase flash
 */
static uint8_t Flash_Memory_Erase(uint32_t StartSectorAddress , uint32_t dataSizeInBytes){
	static FLASH_EraseInitTypeDef EraseInitStruct;   /* Structure to erase the flash area */
	uint32_t SECTORError;

	/* Getting the number of sector to erase from the first sector */
	uint32_t StartSector = GetSector(StartSectorAddress);                /*getting the start sector number*/
	uint32_t EndSectorAddress = StartSectorAddress + dataSizeInBytes;    /*getting the end sector address*/
	uint32_t EndSector = GetSector(EndSectorAddress);                    /*getting the end sector number*/

	/* Filling the erasing structure */
	EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
	EraseInitStruct.Sector        = StartSector;
	EraseInitStruct.NbSectors     = (EndSector - StartSector) + 1;

	/* Unlocking the Flash control register */
	HAL_FLASH_Unlock();

	/* check if the erasing process is done correctly */
	if (HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError) != HAL_OK)
	{
		/*Error occurred while page erase*/
		return ERROR;
	}

	/* Locking the Flash control register */
	HAL_FLASH_Lock();

	return SUCCESS;
}

/*
 * write flash
 */
static uint8_t Flash_Memory_Write(uint32_t StartSectorAddress ,uint32_t *data, uint32_t dataSizeInBytes){
	uint32_t numofWords=dataSizeInBytes/4;     /*getting number of words to write*/
	uint32_t numofWordsWritten=0;

	/* Unlocking the Flash control register */
	HAL_FLASH_Unlock();

	/* looping on the data word by word to write it in the flash */
	while(numofWordsWritten < numofWords){

		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, StartSectorAddress, data[numofWordsWritten]) == HAL_OK)
		{
			StartSectorAddress += 4;
			numofWordsWritten++;
		}
		else
		{
			/* Error occurred while writing data in Flash memory*/
			return ERROR;
		}

	}

	/* Locking the Flash control register */
	HAL_FLASH_Lock();

	return SUCCESS;

}

/*
 * get bootloader version
 */
static void Get_Version_Command_Handler(){
	uint8_t bootloader_version[3]={BOOTLOADER_MAJOR_VERSION,BOOTLOADER_MINOR_VERSION,BOOTLOADER_PATCH_VERSION};
	HAL_UART_Transmit(&huart4, bootloader_version, 3, HAL_MAX_DELAY);
}

/*
 * receiving application and write it on the flash
 */
static void Mem_Write_APP_Command_Handler(){
	uint32_t app_size_length = atoi(&Bootloader_Rx_Buffer[2]);

	//receiving application
	//for now maximum received app will be 150000 bytes(same size of BOOTLOADER_RX_BUFFER_LENGTH)
	if(app_size_length <(65535-42)){
		HAL_UART_Receive(&huart4, &Bootloader_Rx_Buffer[42], app_size_length, HAL_MAX_DELAY);
	}else{
		uint32_t temp_app_size_length = app_size_length - 65535;

		HAL_UART_Receive(&huart4, &Bootloader_Rx_Buffer[42], 65535, HAL_MAX_DELAY);
		HAL_UART_Receive(&huart4, &Bootloader_Rx_Buffer[42+65535], temp_app_size_length, HAL_MAX_DELAY);
	}

	//writing app length
	uint8_t result = Flash_Memory_Write(APP_NO_OF_BYTES_START_ADDRESS, (uint32_t *)&Bootloader_Rx_Buffer[2], 8);
	//writing app digest
	result = Flash_Memory_Write(APP_Digest_START_ADDRESS, (uint32_t *)&Bootloader_Rx_Buffer[10], 32);
	//writing app itself
	result = Flash_Memory_Write(APP_BINARY_START_ADDRESS, (uint32_t *)&Bootloader_Rx_Buffer[42], app_size_length);

	//writing app in file system in case using delta patching technique
	FATFS FatFs; 		//Fatfs handle
	FRESULT fres; 		//Result after operations
	FILINFO fileInfo;
	UINT bytesWrote;
	FIL fil; 		//File handle
	fres = f_mount(&FatFs, "", 1); //1=mount now

	// Check if a file exists
	fres = f_stat("app.bin", &fileInfo);
	if (fres == FR_OK) {
	    // File exists
		fres =  f_unlink("app.bin");//delete old app
	} else if (result == FR_NO_FILE) {
	    // File doesn't exist
	} else {
	    // Error occurred
	}

	fres = f_open(&fil,"app.bin", FA_WRITE | FA_OPEN_ALWAYS | FA_CREATE_ALWAYS | FA_READ);
	fres = f_write(&fil, &Bootloader_Rx_Buffer[42], app_size_length, &bytesWrote);

	fres = f_close(&fil);
	fres =f_mount(NULL, "", 0);

	HAL_UART_Transmit(&huart4, &result, 1, HAL_MAX_DELAY);

	if(result ==SUCCESS){
		Last_written_image = 1;
	}else{
		Last_written_image = 0;
	}
}

/*
 * erase application flash region
 */
static void Mem_Erase_APP_Command_Handler(){
	uint32_t app_size_length = atoi(&Bootloader_Rx_Buffer[2]);

	uint8_t result = Flash_Memory_Erase(APP_START_ADDRESS ,app_size_length+0x200 );
	HAL_UART_Transmit(&huart4, &result, 1, HAL_MAX_DELAY);
	if(result ==SUCCESS){
		Last_written_image = 0;//desired
	}else{
		Last_written_image = 0;//not desired but for safety
	}
}

/*
 * receiving Bootloader updater and write it on the flash
 */
static void Mem_Write_Bootloader_updater_Command_Handler(){

	uint32_t Bootloader_updater_size_length = atoi(&Bootloader_Rx_Buffer[2]);

	HAL_UART_Receive(&huart4, &Bootloader_Rx_Buffer[10], Bootloader_updater_size_length, HAL_MAX_DELAY);

	//writing  Bootloader updater binary
	uint8_t result = Flash_Memory_Write(BOOTLOADER_UPDATER_BINARY_START_ADDRESS, (uint32_t *)&Bootloader_Rx_Buffer[10], Bootloader_updater_size_length);

	HAL_UART_Transmit(&huart4, &result, 1, HAL_MAX_DELAY);

	if(result ==SUCCESS){
			Last_written_image = 2;
	}else{
			Last_written_image = 0;
	}


}

/*
 * erase Bootloader Updater flash region
 */
static void Mem_Erase_Bootloader_updater_Command_Handler(){
	uint32_t Bootloader_updater_size_length = atoi(&Bootloader_Rx_Buffer[2]);

	uint8_t result = Flash_Memory_Erase(BOOTLOADER_UPDATER_BINARY_START_ADDRESS ,Bootloader_updater_size_length );
	HAL_UART_Transmit(&huart4, &result, 1, HAL_MAX_DELAY);

	if(result ==SUCCESS){
			Last_written_image = 0;//desired
	}else{
			Last_written_image = 0;//not desired but for safety
	}
}

/*
 * leaving bootloader
 */
static void Leaving_To_Boot_Manager_Command_Handler(){

	/*
	 * update Control flags
	 */
	if(Last_written_image == 0){
		Write_RTC_backup_reg(APPLICATION_ENTER_FLAG_ADDRESS,N_ENTER);
		Write_RTC_backup_reg(BOOTLOADER_UPDATER_ENTER_FLAG_ADDRESS, N_ENTER);
	}else if(Last_written_image == 1){
		Write_RTC_backup_reg(APPLICATION_ENTER_FLAG_ADDRESS,ENTER);
		Write_RTC_backup_reg(BOOTLOADER_UPDATER_ENTER_FLAG_ADDRESS, N_ENTER);
	}else if(Last_written_image == 2){
		Write_RTC_backup_reg(APPLICATION_ENTER_FLAG_ADDRESS,N_ENTER);
		Write_RTC_backup_reg(BOOTLOADER_UPDATER_ENTER_FLAG_ADDRESS, ENTER);
	}
	//sw reset
	NVIC_SystemReset();
}



static uint8_t Reconstruct_Image_From_Delta(const char *original_image_path,const char *delta_image_path,const char *reconstruct_image_path, uint8_t * delta_image,uint32_t delta_image_size){

	uint8_t total_result=0;
	//some variables for FatFs
	FIL fil_old; 		//File handle
	FIL fil_patch; 		//File handle
	FIL fil_new; 		//File handle
	FRESULT fres; 		//Result after operations

	fres = f_open(&fil_patch,delta_image_path, FA_WRITE | FA_OPEN_ALWAYS | FA_CREATE_ALWAYS | FA_READ);
	if(fres == FR_OK) {

	} else {
		total_result+=1;
	}

	UINT bytesWrote;
	f_write(&fil_patch, delta_image, delta_image_size, &bytesWrote);
	if(fres == FR_OK) {

	} else {
		total_result+=1;
	}

	fres = f_open(&fil_old, original_image_path, FA_READ);
	if(fres == FR_OK) {
	} else {
		total_result+=1;
	}

	fres = f_open(&fil_new, reconstruct_image_path, FA_WRITE| FA_OPEN_ALWAYS | FA_CREATE_ALWAYS );
	if(fres == FR_OK) {

	} else {
		total_result+=1;
	}
	// janpatch_ctx contains buffers, and references to the file system functions
	janpatch_ctx ctx = {
			{ (unsigned char*)malloc(1024), 1024 }, // source buffer
			{ (unsigned char*)malloc(1024), 1024 }, // patch buffer
			{ (unsigned char*)malloc(1024), 1024 }, // target buffer
			&f_read,
			&f_write,
			&f_lseek
	};

	//patching
	int res = janpatch(ctx, &fil_old, &fil_patch, &fil_new);
	if(res == 0) {

	} else {
		total_result+=1;
	}
	fres = f_close(&fil_old);
	if(fres == FR_OK) {

	} else {
		total_result+=1;
	}
	fres = f_close(&fil_patch);
	if(fres == FR_OK) {

	} else {
		total_result+=1;
	}
	fres = f_close(&fil_new);
	if(fres == FR_OK) {

	} else {
		total_result+=1;
	}
	return total_result;
}

static uint8_t Flashing_Reconstructed_Image(const char *reconstruct_image_path) {

	uint8_t total_result=0;

	FRESULT fres;
	uint8_t res;
	FIL fil_new; 		//File handle

	fres = f_open(&fil_new, reconstruct_image_path, FA_READ );

	if(fres == FR_OK){
		uint64_t size;
		FILINFO fno;
		fres = f_stat (reconstruct_image_path,&fno );
		if(fres == FR_OK) {

		} else {
			total_result+=1;
		}
		res = Flash_Memory_Erase(APP_BINARY_START_ADDRESS ,fno.fsize );
		if(res == 0) {

		} else {
			total_result+=1;
		}
		memset(Bootloader_Rx_Buffer,0,fno.fsize);
		fres = f_read(&fil_new, Bootloader_Rx_Buffer, fno.fsize, &size);
		if(fres == FR_OK) {

		} else {
			total_result+=1;
		}
		// buffer now has the data ,can use and flash it
		res = Flash_Memory_Write(APP_BINARY_START_ADDRESS, Bootloader_Rx_Buffer, fno.fsize);
		if(res == 0) {

		} else {
			total_result+=1;
		}

	}else {
		total_result+=1;
	}
	fres = f_close(&fil_new);
	if(fres == 0) {

	} else {
		total_result+=1;
	}
	return total_result;
}


#if (DELTA_PATCH_ENABLED == ENABLED)
static void Bootloader_Delta_Patching_Handler(){

	FATFS FatFs; 		//Fatfs handle
	FRESULT fres; 		//Result after operations
	//Open the file system
	fres = f_mount(&FatFs, "", 1); //1=mount now

	uint32_t delta_image_length = atoi(&Bootloader_Rx_Buffer[2]);
	uint32_t reconstructed_image_length = atoi(&Bootloader_Rx_Buffer[10]);


	HAL_UART_Receive(&huart4, &Bootloader_Rx_Buffer[50], delta_image_length, HAL_MAX_DELAY);

	//writing reconstructed image length
	uint8_t re_length[8]={0};
	memcpy(re_length,&Bootloader_Rx_Buffer[10],8);
	//writing app digest
	uint8_t digest_[32]={0};
	memcpy(digest_,&Bootloader_Rx_Buffer[18],32);


	uint8_t res =0;
	//reconstruct image
	res += Reconstruct_Image_From_Delta("app.bin", "diff.bin", "reconstruct.bin", &Bootloader_Rx_Buffer[50], delta_image_length);
	if(res == 0){
		FILINFO fno;
		fres = f_stat ("reconstruct.bin",&fno );
		//in case of success Reconstruction
		if(fres == 0 && fno.fsize == reconstructed_image_length ){
			//flashing reconstruceted Image
			res = Flashing_Reconstructed_Image("reconstruct.bin");
			if(res == 0) {
				//delete old data files
				fres =  f_unlink("app.bin");
				fres +=  f_unlink("diff.bin");
				//rename reconstruct to app to use it in next times
				fres += f_rename("reconstruct.bin","app.bin");



				if(fres ==SUCCESS){
					Last_written_image = 1;
					res = Flash_Memory_Write(APP_NO_OF_BYTES_START_ADDRESS, (uint32_t *)re_length, 8);
					res += Flash_Memory_Write(APP_Digest_START_ADDRESS, (uint32_t *)digest_, 32);
				}else{
					Last_written_image = 0;
				}
				HAL_UART_Transmit(&huart4, &fres, 1, HAL_MAX_DELAY);
				fres+=5;
			} else {
				HAL_UART_Transmit(&huart4, &res, 1, HAL_MAX_DELAY);
			}
		}
		else{
			res =1;
			HAL_UART_Transmit(&huart4, &res, 1, HAL_MAX_DELAY);
		}
	}else{
		res =1;
		HAL_UART_Transmit(&huart4, &res, 1, HAL_MAX_DELAY);
	}

	//We're done, so de-mount the drive
	fres =f_mount(NULL, "", 0);

}

#endif
/*
 * Receiving Commands from BCM and handle it
 */
static void Bootloader_Receive_Command(void){
	uint8_t command_Length = 0;
	/*clear receiving buffer*/
	memset(Bootloader_Rx_Buffer, 0, BOOTLOADER_RX_BUFFER_LENGTH);
	/* Read the length of the command received from the BCM */
	HAL_UART_Receive(&huart4, Bootloader_Rx_Buffer, 1, HAL_MAX_DELAY);

	command_Length = Bootloader_Rx_Buffer[0];
	/* Read the command*/
	HAL_UART_Receive(&huart4, &Bootloader_Rx_Buffer[1], command_Length, HAL_MAX_DELAY);

	/*
	 * Directing to specific Command Handler
	 */
	switch(Bootloader_Rx_Buffer[1]){
	case BOOTLOADER_GET_VERION_COMMAND:
		Get_Version_Command_Handler();
		break;
	case BOOTLOADER_MEM_WRITE_APP_COMMAND:
		Mem_Write_APP_Command_Handler();
		break;
	case BOOTLOADER_MEM_ERASE_APP_COMMAND:
		Mem_Erase_APP_Command_Handler();
		break;
	case BOOTLOADER_MEM_WRITE_BOOTLOADER_UPDATER_COMMAND:
		Mem_Write_Bootloader_updater_Command_Handler();
		break;
	case BOOTLOADER_MEM_ERASE_BOOTLOADER_UPDATER_COMMAND:
		Mem_Erase_Bootloader_updater_Command_Handler();
		break;
	case BOOTLOADER_LEAVING_TO_BOOT_MANAGER_COMMAND:
		Leaving_To_Boot_Manager_Command_Handler();
		break;
	case BOOTLOADER_DELTA_PATCHING_COMMAND:
		Bootloader_Delta_Patching_Handler();
		break;
	default:
		//do no thing
		break;
	}
}


/*******************************************************************************
 *                      Global Functions Definitions                            *
 *******************************************************************************/

/***************************************************************************************************
 * [Function Name]: App_Logic
 *
 * [Description]:  App logic and behaviour
 *
 * [Args]:         void
 *
 * [Returns]:      void
 *
 ***************************************************************************************************/
void APP_Logic(void){

	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin,GPIO_PIN_SET);

	Bootloader_Receive_Command();

}



