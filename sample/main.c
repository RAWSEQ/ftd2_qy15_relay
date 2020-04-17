#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "./ftd2xx.h"

int main()
{
    // Settings

    int port = 0;
    char buff = 0b00001010; //  channel 1 2 3 4 => 0 1 0 1

    int retCode = -1; // Assume failure
    FT_STATUS ftStatus = FT_OK;
    FT_HANDLE ftHandle = NULL;
    DWORD bytesWritten = 0;

    // Open

    ftStatus = FT_Open(port, &ftHandle);
    if (ftStatus != FT_OK)
    {
        printf("FT_Open(%d) failed, with error %d.\n", port, (int)ftStatus);
        printf("- Use lsmod to check if ftdi_sio (and usbserial) are present.\n");
        printf("  If so, unload them using rmmod, as they conflict with ftd2xx.\n");
        printf("- Check the port number of the device you want to use.\n");
        printf("- Try run by Super User Mode.\n");
        goto exit;
    }

    // Setup (BIT BANG MODE)

    assert(ftHandle != NULL);
    FT_ResetDevice(ftHandle);
    FT_SetBaudRate(ftHandle, 9600);
    FT_SetDataCharacteristics(ftHandle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);
    FT_SetDtr(ftHandle);
    FT_SetFlowControl(ftHandle, FT_FLOW_RTS_CTS, 0, 0);
    FT_SetRts(ftHandle);
    FT_SetTimeouts(ftHandle, 3000, 3000); // 3 seconds
    FT_SetBitMode(ftHandle, 0xff, 1);

    // Write

    FT_Write(ftHandle, &buff, 1, &bytesWritten);

    printf("Sended.\n");

    retCode = 0;

exit:
    if (ftHandle != NULL)
        FT_Close(ftHandle);

    return retCode;
}
