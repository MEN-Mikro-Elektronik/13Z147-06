/****************************************************************************
 ************                                                    ************
 ************                   Z147_EXAMPLE                     ************
 ************                                                    ************
 ****************************************************************************/
/*!
 *         \file timing_test_rx_part.c
 *       \author Apatil
 *        $Date: 2009/07/10 13:40:12 $
 *    $Revision: 1.5 $
 *
 *       \brief  Simple example program for the Z147 driver
 *
 *               Reads and writes some values from/to GPIO ports,
 *               generating interrupts.
 *               Interrupts will be generated only on inputs. Thatsway
 *               normaly an external loopback from the outputs gpio[0]..[4]
 *               to gpio[5]..[7] is required.
 *
 *     Required: libraries: mdis_api, usr_oss
 *     \switches (none)
 */
/*-------------------------------[ History ]--------------------------------
 *
 * $Log: z17_simp.c,v $
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2003 by MEN mikro elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/

static const char RCSid[]="$Id: timing_test_rx_part.c,v 1.5 2009/07/10 13:40:12 CRuff Exp $";

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
/* SYNC words */
#define ARINC717_SUB_1_SYNC      0x247
#define ARINC717_SUB_2_SYNC      0x5B8
#define ARINC717_SUB_3_SYNC      0xA47
#define ARINC717_SUB_4_SYNC      0xDB8

#define Z147_RX_DATA_RATE_64     0    /**< S  : Set data rate of 64 words/sec. */
#define Z147_RX_DATA_RATE_128    1    /**< S  : Set data rate of 128 words/sec. */
#define Z147_RX_DATA_RATE_256    2    /**< S  : Set data rate of 256 words/sec. */
#define Z147_RX_DATA_RATE_512    3    /**< S  : Set data rate of 512 words/sec. */
#define Z147_RX_DATA_RATE_1024   4    /**< S  : Set data rate of 1024 words/sec. */
#define Z147_RX_DATA_RATE_2048   5    /**< S  : Set data rate of 2048 words/sec. */
#define Z147_RX_DATA_RATE_4096   6    /**< S  : Set data rate of 4096 words/sec. */
#define Z147_RX_DATA_RATE_8192   7    /**< S  : Set data rate of 8192 words/sec. */

#define Z147_FRAME_TIME    4000
/*--------------------------------------+
|   TYPDEFS                             |
+--------------------------------------*/
/* none */

/*--------------------------------------+
|   EXTERNALS                           |
+--------------------------------------*/
/* none */

/*--------------------------------------+
|   GLOBALS                             |
+--------------------------------------*/
static int G_sigCount = 0;

/*--------------------------------------+
|   PROTOTYPES                          |
+--------------------------------------*/
static void PrintError(char *info);
static void __MAPILIB SignalHandler( u_int32 sig );
MDIS_PATH rxPath;
u_int16 rxDataArray[32768];
int G_Errors = 0;
int G_TimingError = 0;
u_int32 G_DataLen = 0;
u_int8 G_DataByte = 0;
u_int32 G_TxFrameCnt = 0;
u_int32 G_RxFrameCnt = 0;
u_int32 G_CurrentTime = 0;
u_int32 G_PrevTime = 0;
u_int32 G_StartParam = 7;
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
	char	*rxDevice;
	int32 result = 0;
	int32 isSync = 0;

	int i = 0;
	int j = 0;
	int k = 0;
	int dataRate =0;
	u_int32 data = 0;
	printf("argc = %d\n",argc );
	if (argc < 1 || strcmp(argv[1],"-?")==0) {
		printf("Syntax: timing_test_rx_part <rx_rxDevice> \n");
		printf("Function: Test tool for timing test\n");
		printf("%s \n",RCSid );
		return(1);
	}

	printf("%s \n",RCSid );

	rxDevice = argv[1];

	for(dataRate = 0; dataRate <= Z147_RX_DATA_RATE_8192; dataRate++){
		/*--------------------+
	    |  open rxPath          |
	    +--------------------*/
		if ((rxPath = M_open(rxDevice)) < 0) {
			PrintError("open");
			return(1);
		}
		/*--------------------+
	    |  config             |
	    +--------------------*/
		/* install signal which will be received at change of input ports */
		UOS_SigInit( SignalHandler );
		UOS_SigInstall( UOS_SIG_USR1 );
		result = M_setstat(rxPath, Z147_SET_SIGNAL, UOS_SIG_USR1);
		if(result != 0){
			G_Errors++;
		}
		switch(dataRate){
		case Z147_RX_DATA_RATE_64:
			G_DataLen = 256;
			break;
		case Z147_RX_DATA_RATE_128:
			G_DataLen = 512;
			break;
		case Z147_RX_DATA_RATE_256:
			G_DataLen = 1024;
			break;
		case Z147_RX_DATA_RATE_512:
			G_DataLen = 2048;
			break;
		case Z147_RX_DATA_RATE_1024:
			G_DataLen = 4096;
			break;
		case Z147_RX_DATA_RATE_2048:
			G_DataLen = 8192;
			break;
		case Z147_RX_DATA_RATE_4096:
			G_DataLen = 16384;
			break;
		case Z147_RX_DATA_RATE_8192:
			G_DataLen = 32768;
			break;
		default:
			G_DataLen = 256;
			printf("Unknown dataRate = %d, setting default rate to 64.\n", dataRate);
		}

		result = M_setstat(rxPath, Z147_RX_DATA_RATE, dataRate);
		if(result != 0){
			printf("Setting receive data rate failed.\n");
			G_Errors++;
		}
		G_DataByte = 0;
		G_PrevTime = 0;
		G_sigCount = 0;

		for(k=0;k<20; ){
			if(G_sigCount == ( k + 1)){
				printf("\n");
				printf("################### Test Run-%d (Data Rate-%d) ################\n", k+1, dataRate);
				k++;
				printf("G_sigCount = %d\n", G_sigCount);
			}else{
				result = M_getstat(rxPath, Z147_RX_IN_SYNC, &isSync);
				if((k!=0) && (isSync != 1)){
					printf("Sync Lost\n");
					goto Z147_EXIT;
				}
				UOS_Delay(100);
			}
//
//			if((isSync == 0) && (k==0)){
//				while(1){
//					result = M_getstat(rxPath, Z147_RX_IN_SYNC, &isSync);
//
//					if(result != 0){
//						G_Errors++;
//					}
//					if(isSync){
//						break;
//					}else{
//						UOS_Delay(100);
//					}
//				}
//			}else if((isSync == 0) && (k!=0)){
//				printf("Sync failed\n");
//				goto Z147_EXIT;
//			}

		}
		result = M_setstat(rxPath, Z147_DISABLE_RX, 1);
		if(result != 0){
			G_Errors++;
		}
		if (M_close(rxPath) < 0){
			PrintError("close");
		}
	}

	printf("-------------------------------------------\n");
	printf("-------------------------------------------\n\n");
	printf("Receive frame count : %d\n", G_RxFrameCnt);
	printf("Test Result : ");
	if((G_Errors != 0) || (G_TimingError != 0)){
		printf("FAILED\n");
		if(G_TimingError != 0){
			printf("Timing error detected.");
		}
	}else{
		printf("PASSED\n");
	}
	printf("\n-------------------------------------------\n");
	printf("-------------------------------------------\n");

	return(0);

Z147_EXIT:
	result = M_setstat(rxPath, Z147_DISABLE_RX, 1);
	if(result != 0){
		G_Errors++;
	}
	if (M_close(rxPath) < 0){
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

/**********************************************************************/
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
	if( sig == UOS_SIG_USR1 ) {
		++G_sigCount;
		G_CurrentTime = UOS_MsecTimerGet();
		if(G_PrevTime != 0){
			if(G_PrevTime <G_CurrentTime){
				timeDiff = (G_CurrentTime - G_PrevTime);
			}else{
				timeDiff = (G_PrevTime - G_CurrentTime);
			}
			if((timeDiff > (Z147_FRAME_TIME + 5)) || (timeDiff < (Z147_FRAME_TIME - 5))){
				printf("Time difference is expected 4000ms but received %d\n", timeDiff);
				G_TimingError++;
			}else{
				printf(" Time difference is 4000ms as expected.\n");
			}
			G_PrevTime = G_CurrentTime;
		}else{
			G_PrevTime = G_CurrentTime;
		}
		result = M_getblock( rxPath, (u_int8*)rxDataArray, (G_DataLen *2) );
		if(result < 0){
			G_Errors++;
		}
		/* program lower five ports as outputs, others as inputs */
		if(result > 0){
			printf("----------------- Receive -----------------\n");
			printf("Receiving = %d bytes data\n", result);
			G_DataByte = 0;
			/* Add 4 sync words and ignore them in check. */
			for(j=0, i=0;j<(result/2);j++){
//			for(j=0, i=0;j<(10);i++,j++){
				if((rxDataArray[j] != ARINC717_SUB_1_SYNC) &&
						(rxDataArray[j] != ARINC717_SUB_2_SYNC) &&
						(rxDataArray[j] != ARINC717_SUB_3_SYNC) &&
						(rxDataArray[j] != ARINC717_SUB_4_SYNC))
				{
					if(rxDataArray[j] != (G_DataByte)){
						printf("Expected 0x%x but received rxDataArray[%d] = 0x%x\n",( G_DataByte), j, rxDataArray[j]);
						G_Errors++;
					}
					if((j<10) || (j>((result/2) - 5))){
						printf(" [%d]0x%x",j, rxDataArray[j]);
					}
					G_DataByte++;
				}
				/* Reset the buffer. */
				rxDataArray[j] = 0;
			}
			printf("\n");
			printf("Received %d bytes successfully\n", result);
			printf("--------------------------------\n");
			G_RxFrameCnt++;
			G_StartParam++;
		}else{
			printf("Read failed with the result = %d\n", result);
		}
	}
}
