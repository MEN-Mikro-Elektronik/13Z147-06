/****************************************************************************
 ************                                                    ************
 ************                   Z147_RATE_TEST_RX                ************
 ************                                                    ************
 ****************************************************************************/
/*!
 *         \file rate_test_rx_part.c
 *       \author Apatil
 *        $Date: 2016/04/27 13:47:39 $
 *    $Revision: 1.1 $
 *
 *       \brief  Z147 test tool for rate (rx part)
 *
 *     Required: libraries: mdis_api, usr_oss
 *     \switches (none)
 */
/*-------------------------------[ History ]--------------------------------
 *
 * $Log: rate_test_rx_part.c,v $
 * Revision 1.1  2016/04/27 13:47:39  Apatil
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
#define Z147_FRAME_TIME		4000
#define MAX_DATA_LEN 		32768

/*--------------------------------------+
|   GLOBALS                             |
+--------------------------------------*/
static int G_sigCount = 0;
MDIS_PATH G_rxPath;
int G_errors = 0;
int G_timingError = 0;
u_int32 G_dataLen = 0;
u_int8 G_dataByte = 0;
u_int32 G_rxFrameCnt = 0;
u_int32 G_currentTime = 0;
u_int32 G_prevTime = 0;

/*--------------------------------------+
|   PROTOTYPES                          |
+--------------------------------------*/
static void PrintError(char *info);
static void __MAPILIB SignalHandler( u_int32 sig );

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
	int32 irqCount;
	char *rxDevice;
	int32 result = 0;
	int32 isSync = 0;
	int i = 0;
	int j = 0;
	int k = 0;
	int dataRate =0;
	u_int32 data = 0;

	if (argc < 1 || strcmp(argv[1],"-?")==0) {
		printf("Syntax: z147_rate_test_rx <rxDevice> \n");
		printf("Function: Z147 test tool for rate (rx part)\n");
		return(1);
	}

	rxDevice = argv[1];

	for(dataRate = 0; dataRate <= Z147_RX_DATA_RATE_8192; dataRate++){
		/*--------------------+
	    |  open               |
	    +--------------------*/
		if ((G_rxPath = M_open(rxDevice)) < 0) {
			PrintError("open");
			return(1);
		}
		/*--------------------+
	    |  config             |
	    +--------------------*/
		UOS_SigInit( SignalHandler );
		UOS_SigInstall( UOS_SIG_USR1 );
		
		result = M_setstat(G_rxPath, Z147_SET_SIGNAL, UOS_SIG_USR1);
		if(result != 0){
			G_errors++;
		}
		switch(dataRate){
		case Z147_RX_DATA_RATE_64:
			G_dataLen = 256;
			break;
		case Z147_RX_DATA_RATE_128:
			G_dataLen = 512;
			break;
		case Z147_RX_DATA_RATE_256:
			G_dataLen = 1024;
			break;
		case Z147_RX_DATA_RATE_512:
			G_dataLen = 2048;
			break;
		case Z147_RX_DATA_RATE_1024:
			G_dataLen = 4096;
			break;
		case Z147_RX_DATA_RATE_2048:
			G_dataLen = 8192;
			break;
		case Z147_RX_DATA_RATE_4096:
			G_dataLen = 16384;
			break;
		case Z147_RX_DATA_RATE_8192:
			G_dataLen = 32768;
			break;
		default:
			G_dataLen = 256;
			printf("Unknown dataRate = %d, setting default rate to 64.\n", dataRate);
		}

		result = M_setstat(G_rxPath, Z147_RX_DATA_RATE, dataRate);
		if(result != 0){
			printf("Setting receive data rate failed.\n");
			G_errors++;
		}
		G_dataByte = 0;
		G_prevTime = 0;
		G_sigCount = 0;

		for(k=0;k<20; ){
			if(G_sigCount == ( k + 1)){
				printf("\n");
				printf("################### Test Run-%d (Data Rate-%d) ################\n",
					k+1, dataRate);
				k++;
				printf("G_sigCount = %d\n", G_sigCount);
			}else{
				result = M_getstat(G_rxPath, Z147_RX_IN_SYNC, &isSync);
				if((k!=0) && (isSync != 1)){
					printf("Sync Lost\n");
					goto Z147_EXIT;
				}
				UOS_Delay(100);
			}
		}

		result = M_setstat(G_rxPath, Z147_DISABLE_RX, 1);
		if(result != 0){
			G_errors++;
		}
		if (M_close(G_rxPath) < 0){
			PrintError("close");
		}
	}

	printf("-------------------------------------------\n");
	printf("-------------------------------------------\n\n");
	printf("Receive frame count : %d\n", G_rxFrameCnt);
	printf("Test Result : ");
	if((G_errors != 0) || (G_timingError != 0)){
		printf("FAILED\n");
		if(G_timingError != 0){
			printf("Timing error detected.");
		}
	}else{
		printf("PASSED\n");
	}
	printf("\n-------------------------------------------\n");
	printf("-------------------------------------------\n");

	return(0);

Z147_EXIT:
	result = M_setstat(G_rxPath, Z147_DISABLE_RX, 1);
	if(result != 0){
		G_errors++;
	}
	if (M_close(G_rxPath) < 0){
		PrintError("close");
	}

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

/****************************** SignalHandler ******************************/
/** Signal handler
 *
 *  \param  sig    \IN   received signal
 */
static void __MAPILIB SignalHandler( u_int32 sig )
{
	int32 result = 0;
	int i = 0;
	int j=0;
	u_int32 timeDiff = 0;
	u_int16 rxDataArray[MAX_DATA_LEN];
	
	if( sig == UOS_SIG_USR1 ) {
		++G_sigCount;
		G_currentTime = UOS_MsecTimerGet();
		
		if(G_prevTime != 0){
			
			if(G_prevTime <G_currentTime){
				timeDiff = (G_currentTime - G_prevTime);
			}else{
				timeDiff = (G_prevTime - G_currentTime);
			}
			
			if((timeDiff > (Z147_FRAME_TIME + 5)) || (timeDiff < (Z147_FRAME_TIME - 5))){
				printf("Time difference is expected 4000ms but received %d\n", timeDiff);
				G_timingError++;
			}else{
				printf(" Time difference is 4000ms as expected.\n");
			}
			
			G_prevTime = G_currentTime;
		}else{
			G_prevTime = G_currentTime;
		}
		
		result = M_getblock( G_rxPath, (u_int8*)rxDataArray, (G_dataLen *2) );
		if(result < 0){
			G_errors++;
		}
		if(result > 0){
			printf("----------------- Receive -----------------\n");
			printf("Receiving = %d bytes data\n", result);
			G_dataByte = 0;
			/* Add 4 sync words and ignore them in check. */
			for(j=0, i=0;j<(result/2);j++){
				if((rxDataArray[j] != Z147_ARINC717_SUB_1_SYNC) &&
						(rxDataArray[j] != Z147_ARINC717_SUB_2_SYNC) &&
						(rxDataArray[j] != Z147_ARINC717_SUB_3_SYNC) &&
						(rxDataArray[j] != Z147_ARINC717_SUB_4_SYNC))
				{
					if(rxDataArray[j] != (G_dataByte)){
						printf("Expected 0x%x but received rxDataArray[%d] = 0x%x\n",
							( G_dataByte), j, rxDataArray[j]);
						G_errors++;
					}
					if((j<10) || (j>((result/2) - 5))){
						printf(" [%d]0x%x",j, rxDataArray[j]);
					}
					G_dataByte++;
				}
				/* Reset the buffer. */
				rxDataArray[j] = 0;
			}
			printf("\n");
			printf("Received %d bytes successfully\n", result);
			printf("--------------------------------\n");
			G_rxFrameCnt++;
		}else{
			printf("Read failed with the result = %d\n", result);
		}
	}
}
