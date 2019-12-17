
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <asm/ioctl.h>
#include <linux/spi/spidev.h>

int SPIDataRW(int channel, unsigned char *tx_data, unsigned char *rx_data, int len);
int SPISetupMode(int channel, int speed, int mode);
int SPISetup(int channel, int speed);

static const char   *spiDev0 = "/dev/spidev0.0";
static const char   *spiDev1 = "/dev/spidev0.1";
static  uint8_t     spiBPW = 8;
static  uint16_t    spiDelay = 0;

static uint32_t     spiSpeeds[2];
static int          spiFds[2];

/*
* SPIDataRW:
*    Write and Read a block of data over the SPI bus.
*    Note the data ia being read into the transmit buffer, so will
*    overwrite it!
*    This is also a full-duplex operation.
*********************************************************************************
*********************************************************************************/
int SPIDataRW(int channel, unsigned char *tx_data, unsigned char *rx_data, int len)
{
	int i = 0;
	struct spi_ioc_transfer spi;

	channel &= 1;

	memset(&spi, 0, sizeof(spi));

	spi.tx_buf = (unsigned long)tx_data;
	spi.rx_buf = (unsigned long)rx_data;
	spi.len = len;
	spi.delay_usecs = spiDelay;
	spi.speed_hz = spiSpeeds[channel];
	spi.bits_per_word = spiBPW;

	return ioctl(spiFds[channel], SPI_IOC_MESSAGE(1), &spi);
}


/*
* SPISetupMode:
*    Open the SPI device, and set it up, with the mode, etc.
*********************************************************************************
*********************************************************************************/

int SPISetupMode(int channel, int speed, int mode)
{
	int fd;

	if ((fd = open(channel == 0 ? spiDev0 : spiDev1, O_RDWR)) < 0)
	{
		printf("Unable to open SPI device: %s\n", strerror(errno));
		return -1;
	}

	spiSpeeds[channel] = speed;
	spiFds[channel] = fd;

	//  Mode CPOL=0, CPHA=0
	//  Mode CPOL=0, CPHA=1
	//  Mode CPOL=1, CPHA=0
	//  Mode CPOL=1, CPHA=1
	if (ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0)
	{
		printf("Can't set spi mode: %s\n", strerror(errno));
		return -1;
	}

	if (ioctl(fd, SPI_IOC_RD_MODE, &mode) < 0)
	{
		printf("Can't get spi mode: %s\n", strerror(errno));
		return -1;
	}


	if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &spiBPW) < 0)
	{
		printf("Can't set bits per word: %s\n", strerror(errno));
		return -1;
	}

	if (ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &spiBPW) < 0)
	{
		printf("Can't get bits per word: %s\n", strerror(errno));
		return -1;
	}

	if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0)
	{
		printf("Can't set max speed hz: %s\n", strerror(errno));
		return -1;
	}

	if (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed) < 0)
	{
		printf("Can't get max speed hz: %s\n", strerror(errno));
		return -1;
	}

	return fd;
}


/*
* SPISetup:
*    Open the SPI device, and set it up, etc. in the default MODE 0
*********************************************************************************
*********************************************************************************/

int SPISetup(int channel, int speed)
{
	return SPISetupMode(channel, speed, 1);
}


int initSPI()
{
	int spiFd;
	spiFd = SPISetup(0, 500000);
	if (spiFd == -1)
	{
		printf("init spi failed!\n");
	}
}

int main()
{
	printf( "############################################################\n");
	printf( "This application is designed to test SPI communication\n");
	printf( "between Jetson Nano and the on board FPGA.\n");
	printf( "With spidev0.0, Jetson Nano sends 8 bytes to FPGA, and FPGA\n");
	printf( "  performs bitwise NOT operation, then sends back to Jetson Nano.\n");
	printf( "By comparing TX and RX data, we can verify the SPI interface.\n");
	printf( "\n");
	
	unsigned char tx_Data[8] = { 0xDE,0xED,0xBE,0xEF,0x55,0xAA,0x5A,0xA5 };
	unsigned char rx_Data[8] = { 0,0,0,0,0,0,0,0 };
	bool result_ok = true;
	int i = 0;

        printf( "\n");
        printf( "Test with spidev0.0:\n");
	initSPI();

	SPIDataRW(0, tx_Data, rx_Data, 8);
	printf("Send data is : ");
	for (i = 0; i < 8; i++)
	{
		printf("%02X ", tx_Data[i] );
	}
	printf("\n");
	printf("Read data is : ");
	for (i = 0; i < 8; i++)
	{
		printf("%02X ", rx_Data[i]);
	}
	printf("\n");

	for (i = 0; i < 8; i++)
	{
		unsigned char tx_inv = ~tx_Data[i];
		unsigned char rx_element = rx_Data[i];
		if (tx_inv != rx_element)
		{
			result_ok = false;
			break;
		}
	}

	if (result_ok)
	{
		printf("Read data is bit-wise invertion of sent data.\n");
		printf("SPI test finished successfully!\n");
	}
	else
	{
		printf("Error: Read data is NOT bit-wise invertion of sent data!\n");
		printf("SPI test failed!\n");
	}

	return 0;
}
