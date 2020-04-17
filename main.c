#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "./ftd2xx.h"

void printBin(char num)
{
    int len = 8;
    int bit[8];
    int x;
    int i;

    for (i = 0; i < len; i++)
    {
        x = 1 << i;
        x = num & x;
        bit[len - i - 1] = x >> i;
    }

    printf("0b");
    for (i = 0; i < len; i++)
    {
        printf("%d", bit[i]);
    }
}

int main(int argc, char *argv[])
{
    int retCode = -1; // Assume failure
    int port = 0;
    char buff;
    int i;
    FT_STATUS ftStatus = FT_OK;
    FT_HANDLE ftHandle = NULL;
    DWORD bytesWritten = 0;
    DWORD lpdwBytesReturned = 0;

    int bit[4] = {0, 0, 0, 0};

    // Argument check

    if (argc == 1)
    {
        printf("Usage: [-port (Port Number)] [-on (Channel)...] [-off (Channel)...]\n");
        printf("ex: ./relay_control -port 0 -on 1 -on 3 -off 2\n");
    }

    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-port") == 0)
        {
            if (i + 1 >= argc)
            {
                printf("Failure. Check parameter \n");
                goto exit;
            }
            int c_val = argv[i + 1][0] - '0';
            if (0 > c_val || 16 < c_val)
            {
                printf("Failure. Port range: %s\n", argv[i + 1]);
                goto exit;
            }
            port = c_val;
        }
        if (strcmp(argv[i], "-on") == 0)
        {
            if (i + 1 >= argc)
            {
                printf("Failure. Check parameter \n");
                goto exit;
            }
            int c_val = argv[i + 1][0] - '0';
            if (1 > c_val || 4 < c_val)
            {
                printf("Failure. Channel range: %s\n", argv[i + 1]);
                goto exit;
            }
            bit[c_val - 1] = 1;
        }
        if (strcmp(argv[i], "-off") == 0)
        {
            if (i + 1 >= argc)
            {
                printf("Failure. Check parameter \n");
                goto exit;
            }
            int c_val = argv[i + 1][0] - '0';
            if (1 > c_val || 4 < c_val)
            {
                printf("Failure. Channel range: %s\n", argv[i + 1]);
                goto exit;
            }
            bit[c_val - 1] = -1;
        }
    }

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

    // Read

    ftStatus = FT_Read(ftHandle, &buff, 1, &lpdwBytesReturned);
    if (ftStatus != FT_OK)
    {
        printf("Failure.  FT_Read returned %d\n", (int)ftStatus);
        goto exit;
    }

    // Bit Change

    printf("From: ");
    printBin(buff);
    printf("\n");

    if (bit[0] != 0)
    {
        buff = buff | 1;
        if (bit[0] < 0)
        {
            buff = buff ^ 1;
        }
    }

    if (bit[1] != 0)
    {
        buff = buff | (1 << 1);
        if (bit[1] < 0)
        {
            buff = buff ^ (1 << 1);
        }
    }

    if (bit[2] != 0)
    {
        buff = buff | (1 << 2);
        if (bit[2] < 0)
        {
            buff = buff ^ (1 << 2);
        }
    }

    if (bit[3] != 0)
    {
        buff = buff | (1 << 3);
        if (bit[3] < 0)
        {
            buff = buff ^ (1 << 3);
        }
    }

    // Write

    ftStatus = FT_Write(ftHandle, &buff, 1, &bytesWritten);
    if (ftStatus != FT_OK)
    {
        printf("Failure.  FT_Write returned %d\n", (int)ftStatus);
        goto exit;
    }

    printf("To  : ");
    printBin(buff);
    printf("\n");

    retCode = 0;

exit:
    if (ftHandle != NULL)
        FT_Close(ftHandle);

    return retCode;
}
