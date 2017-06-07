/****************************************************************************
 ************                                                    ************
 ************                   Z147_TIMING_TEST_TX              ************
 ************                                                    ************
 ****************************************************************************/
/*!
 *         \file timing_test_tx_part.c
 *       \author Apatil
 *        $Date: 2016/04/27 13:47:50 $
 *    $Revision: 1.1 $
 *
 *       \brief  Z147 test tool for timing (tx part)
 *
 *     Required: libraries: mdis_api, usr_oss
 *     \switches (none)
 */
/*-------------------------------[ History ]--------------------------------
 *
 * $Log: timing_test_tx_part.c,v $
 * Revision 1.1  2016/04/27 13:47:50  Apatil
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2003 by MEN mikro elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <MEN/men_typs.h>
#include <MEN/mdis_api.h>
#include <MEN/usr_oss.h>
#include <MEN/z147_drv.h>
#include <MEN/z247_drv.h>

/*--------------------------------------+
|   DEFINES                             |
+--------------------------------------*/
#define MAX_DATA_LEN 		32768

/*--------------------------------------+
|   PROTOTYPES                          |
+--------------------------------------*/
static void PrintError(char *info);

/********************************* main ************************************/
/** Program main function
 *
 *  \param argc       \IN  argument counter
 *  \param argv       \IN  argument vector
 *
 *  \return	          success (0) or error (1)
 */
int main(int argc, char *argv[])
{
	char *txDevice;
	int32 result = 0;
	u_int32 i = 0;
	int j = 0;
	int k = 0;
	int dataRate = 0;
	u_int32 data = 0;
	u_int16 *rxBuf = NULL;
	MDIS_PATH txPath;
	u_int16 txDataArray[MAX_DATA_LEN];
	int errors = 0;
	u_int32 dataLen = 0;
	u_int8 dataByte = 0;
	u_int32 txFrameCnt = 0;

	if (argc < 1 || strcmp(argv[1],"-?")==0) {
		printf("Syntax: z147_timing_test_tx <txDevice>\n");
		printf("Function: Z147 test tool for timing (tx part)\n");
		return(1);
	}

	txDevice = argv[1];

	for(dataRate = 0; dataRate <= Z147_RX_DATA_RATE_8192; dataRate++){
		/*--------------------+
	    |  open               |
	    +--------------------*/
		if ((txPath = M_open(txDevice)) < 0) {
			PrintError("open");
			return(1);
		}
		switch(dataRate){
		case Z147_RX_DATA_RATE_64:
			dataLen = 256;
			break;
		case Z147_RX_DATA_RATE_128:
			dataLen = 512;
			break;
		case Z147_RX_DATA_RATE_256:
			dataLen = 1024;
			break;
		case Z147_RX_DATA_RATE_512:
			dataLen = 2048;
			break;
		case Z147_RX_DATA_RATE_1024:
			dataLen = 4096;
			break;
		case Z147_RX_DATA_RATE_2048:
			dataLen = 8192;
			break;
		case Z147_RX_DATA_RATE_4096:
			dataLen = 16384;
			break;
		case Z147_RX_DATA_RATE_8192:
			dataLen = 32768;
			break;
		default:
			dataLen = 256;
			printf("Unknown dataRate = %d, setting default rate to 64.\n", dataRate);
		}
		result = M_setstat(txPath, Z247_TX_DATA_RATE, dataRate);
		if(result != 0){
			printf("Setting transmit data rate failed.\n");
			errors++;
		}

		for(k=0;k<20;k++){
			printf("\n");
			printf("################### Test Run-%d (Data Rate-%d) ################\n",
				k+1, dataRate);
			printf("----------------- Transmit -----------------\n");
			dataByte = 0;
			
			for(i=0;i<(dataLen - 4);i++){
				txDataArray[i] = (dataByte++);
				if((i<10) || (i>((dataLen - 4) - 5))){
					printf(" 0x%x", txDataArray[i]);
				}
			}

			printf("\n");
			printf("\n");
			
			result = M_setblock(txPath, (u_int8*)txDataArray, (dataLen *2) - 8);
			if(result > 0){
				printf("Transmitted %d bytes successfully\n", dataLen *2);
			}else{
				printf("Write failed with result %d\n", result);
				errors++;
			}
			
			printf("--------------------------------\n");
			printf("\n");

			txFrameCnt++;
			UOS_Delay( 4000);
		}
		
		UOS_Delay( 4000);
		
		result = M_setstat(txPath, Z247_DISABLE_TX, 1);
		if(result != 0){
			errors++;
		}
		if (M_close(txPath) < 0){
			PrintError("close");
		}
	}

	printf("-------------------------------------------\n");
	printf("-------------------------------------------\n\n");
	printf("Transmit frame count : %d\n", txFrameCnt);

	printf("Test Result : ");
	if(errors != 0){
		printf("FAILED\n");
	}else{
		printf("PASSED\n");
	}
	printf("\n-------------------------------------------\n");
	printf("-------------------------------------------\n");

	return(0);
}

/********************************* PrintError ******************************/
/** Print MDIS error message
 *
 *  \param info       \IN  info string
 */
static void PrintError(char *info)
{
	printf("*** can't %s: %s\n", info, M_errstring(UOS_ErrnoGet()));
}
