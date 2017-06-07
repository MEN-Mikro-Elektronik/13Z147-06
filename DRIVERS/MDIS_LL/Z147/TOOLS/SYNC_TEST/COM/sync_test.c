/****************************************************************************
 ************                                                    ************
 ************                   Z147_SYNC_TEST                   ************
 ************                                                    ************
 ****************************************************************************/
/*!
 *         \file z146_example.c
 *       \author Apatil
 *        $Date: 2016/04/27 13:47:44 $
 *    $Revision: 1.1 $
 *
 *       \brief  Z147 test tool for sync
 *
 *     Required: libraries: mdis_api, usr_oss
 *     \switches (none)
 */
/*-------------------------------[ History ]--------------------------------
 *
 * $Log: sync_test.c,v $
 * Revision 1.1  2016/04/27 13:47:44  Apatil
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
#define MAX_DATA_LEN 	32768

/*--------------------------------------+
|   GLOBALS                             |
+--------------------------------------*/
static int G_sigCount = 0;
MDIS_PATH G_rxPath;
int G_errors = 0;
u_int32 G_dataLen = 0;
u_int32 G_rxFrameCnt = 0;

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
	char *txDevice;
	int32 result = 0;
	int32 isSync = 0;
	u_int32 i = 0;
	int j = 0;
	int k = 0;
	int syncCfg =0;
	int testType = 0;
	u_int32 data = 0;
	u_int16 *txBuf = NULL;
	u_int16 *rxBuf = NULL;
	MDIS_PATH txPath;
	u_int16 txDataArray[MAX_DATA_LEN];
	u_int8 txDataByte = 0;
	u_int32 txFrameCnt = 0;

	if (argc < 3 || strcmp(argv[1],"-?")==0) {
		printf("Syntax: z147_sync_test <rxDevice> <txDevice>\n");
		printf("Function: Z147 test tool for sync\n");
		return(1);
	}

	rxDevice = argv[1];
	txDevice = argv[2];
	
	/*--------------------+
    |  open               |
    +--------------------*/
	if ((G_rxPath = M_open(rxDevice)) < 0) {
		PrintError("open");
		return(1);
	}
	if ((txPath = M_open(txDevice)) < 0) {
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
	
	G_dataLen = 256;

	for(syncCfg = 2; syncCfg >= 0; syncCfg--){

		result = M_setstat(G_rxPath, Z147_RX_SYNC_CFG, syncCfg);
		if(result != 0){
			printf("Setting receive sync failed.\n");
			G_errors++;
		}
		result = M_setstat(txPath, Z247_TX_DATA_RATE, 0);
		if(result != 0){
			printf("Setting transmit data rate failed.\n");
			G_errors++;
		}
		result = M_setstat(G_rxPath, Z147_RX_DATA_RATE, 0);
		if(result != 0){
			printf("Setting receive data rate failed.\n");
			G_errors++;
		}
		txDataByte = 0;
		for(k=0;k<10;k++){
			printf("\n");
			printf("################### Test Run-%d (Sync Cfg-%d) ################\n",
				k+1, syncCfg);
			printf("----------------- Transmit -----------------\n");
			printf("User data length = %d and dataptr = 0x%x\n", G_dataLen, txDataArray);
			txDataByte = 0;
			for(i=0;i<(G_dataLen-4);i++){
				if(i==txDataByte){
					txDataArray[i] = txDataByte++;
				}

			}
			printf("\n");
			printf("\n");
			result = M_setblock(txPath, (u_int8*)txDataArray, (G_dataLen *2) - 8);
			if(result > 0){
				printf("Transmitted %d bytes successfully\n", G_dataLen *2);
			}else{
				printf("Write failed with result %d\n", result);
				G_errors++;
			}
			printf("--------------------------------\n");
			printf("\n");

			UOS_Delay( 4000);
			if(syncCfg == 2){
				while(1){
					result = M_getstat(G_rxPath, Z147_RX_IN_SYNC, &isSync);

					if((k!=0) && result != 0){
						printf("Sync failed\n");
						return -1;
					}

					if(result != 0){
						G_errors++;
					}
					if(isSync){
						printf("Is rx in sync = %d\n", isSync);
						break;
					}else{
						UOS_Delay(100);
					}
				}
			}
			printf("G_sigCount = %d\n", G_sigCount);
			txFrameCnt++;
		}
	}

	result = M_setstat(txPath, Z247_DISABLE_TX, 1);
	if(result != 0){
		G_errors++;
	}
	result = M_setstat(G_rxPath, Z147_DISABLE_RX, 1);
	if(result != 0){
		G_errors++;
	}
	printf("-------------------------------------------\n");
	printf("-------------------------------------------\n\n");
	printf("Transmit frame count : %d\n", txFrameCnt);
	printf("Receive frame count : %d\n", G_rxFrameCnt);
	printf("Test Result : ");
	if(G_errors != 0){
		printf("FAILED\n");
	}else{
		printf("PASSED\n");
	}
	printf("\n-------------------------------------------\n");
	printf("-------------------------------------------\n");


	if (M_close(G_rxPath) < 0){
		PrintError("close");
	}
	if (M_close(txPath) < 0){
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
	int32 i=0;
	int32 j=0;
	u_int16 rxDataArray[MAX_DATA_LEN];
	u_int8 rxDataByte = 0;

	if( sig == UOS_SIG_USR1 ) {
		++G_sigCount;
		
		result = M_getblock( G_rxPath, (u_int8*)rxDataArray, (G_dataLen *2) );
		if(result < 0){
			G_errors++;
		}
		if(result > 0){
			printf("----------------- Receive -----------------\n");
			printf("Receiving = %d bytes data\n", result);
			rxDataByte = 0;
			
			/* Add 4 sync words and ignore them in check. */
			for(j=0;j<(result/2);j++){
				if((rxDataArray[j] != Z147_ARINC717_SUB_1_SYNC) &&
						(rxDataArray[j] != Z147_ARINC717_SUB_2_SYNC) &&
						(rxDataArray[j] != Z147_ARINC717_SUB_3_SYNC) &&
						(rxDataArray[j] != Z147_ARINC717_SUB_4_SYNC))
				{
					if(rxDataArray[j] != (rxDataByte)){
						printf("Expected 0x%x but received rxDataArray[%d] = 0x%x\n",
							(rxDataByte), j, rxDataArray[j]);
						G_errors++;
					}
					if((j<10) || (j>((result/2) - 5))){
						printf(" [%d]0x%x",j, rxDataArray[j]);
					}
					rxDataByte++;
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

