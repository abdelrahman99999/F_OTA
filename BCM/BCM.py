from supported_functions import *


init()

while True:
    print("\n---------The Supported Commands For Bootloader---------")
    print("\tBOOTLOADER_GET_VERION_COMMAND                           - CODE: {}".format(BOOTLOADER_GET_VERION_COMMAND))
    print("\tBOOTLOADER_MEM_WRITE_FOR_APP_COMMAND                    - CODE: {}".format(BOOTLOADER_MEM_WRITE_FOR_APP_COMMAND))
    print("\tBOOTLOADER_MEM_ERASE_FOR_APP_COMMAND                    - CODE: {}".format(BOOTLOADER_MEM_ERASE_FOR_APP_COMMAND))
    print("\tBOOTLOADER_MEM_WRITE_FOR_BOOTLOADER_UPDATER_COMMAND     - CODE: {}".format(BOOTLOADER_MEM_WRITE_FOR_BOOTLOADER_UPDATER_COMMAND))
    print("\tBOOTLOADER_MEM_ERASE_FOR_BOOTLOADER_UPDATER_COMMAND     - CODE: {}".format(BOOTLOADER_MEM_ERASE_FOR_BOOTLOADER_UPDATER_COMMAND))
    print("\tBOOTLOADER_LEAVING_TO_BOOT_MANAGER_COMMAND              - CODE: {}".format(BOOTLOADER_LEAVING_TO_BOOT_MANAGER_COMMAND))
    print("\tBOOTLOADER_DELTA_PATCHING_APP_COMMAND                   - CODE: {}".format(BOOTLOADER_DELTA_PATCHING_APP_COMMAND))

    print("\n---------The Supported Commands For Application---------")
    print("\tAPP_LEAVING_TO_BOOT_MANAGER_COMMAND                     - CODE: {}".format(APP_LEAVING_TO_BOOT_MANAGER_COMMAND))

    print("\n---------The Supported Commands For Communicative Bootloader Updater---------")
    print("\tBOOTLOADER_UPDATER_GET_VERION_COMMAND                   - CODE: {}".format(BOOTLOADER_UPDATER_GET_VERION_COMMAND))
    print("\tBOOTLOADER_UPDATER_MEM_WRITE_FOR_BOOTLOADER_COMMAND     - CODE: {}".format(BOOTLOADER_UPDATER_MEM_WRITE_FOR_BOOTLOADER_COMMAND))
    print("\tBOOTLOADER_UPDATER_MEM_ERASE_FOR_BOOTLOADER_COMMAND     - CODE: {}".format(BOOTLOADER_UPDATER_MEM_ERASE_FOR_BOOTLOADER_COMMAND))
    print("\tBOOTLOADER_UPDATER_LEAVING_TO_BOOT_MANAGER_COMMAND      - CODE: {}".format(BOOTLOADER_UPDATER_LEAVING_TO_BOOT_MANAGER_COMMAND))


    print("\n\n>>>> Enter code 99 if you want to Exit")

    Command_To_Send = input("Enter Command code : ")

    start_time = time.time()
    Execute_Command(int(Command_To_Send))
    end_time = time.time()
    elapsed_time = end_time - start_time
    print("Elapsed time for the command: ", elapsed_time) 


   


