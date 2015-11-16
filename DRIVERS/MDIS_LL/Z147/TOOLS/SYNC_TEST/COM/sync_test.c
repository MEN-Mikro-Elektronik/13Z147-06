/****************************************************************************
 ************                                                    ************
 ************                   Z147_SYNC_TEST                    ************
 ************                                                    ************
 ****************************************************************************/
/*!
 *         \file z146_example.c
 *       \author Apatil
 *        $Date: 2009/07/10 13:40:12 $
 *    $Revision: 1.5 $
 *
 *       \brief  Program for the Z147 driver sync test
 *
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

static const char RCSid[]="$Id: z147_simp.c,v 1.5 2009/07/10 13:40:12 CRuff Exp $";

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
MDIS_PATH txPath;
u_int16 txDataArray[32768];
u_int16 rxDataArray[32768];
int G_Errors = 0;
u_int32 G_DataLen = 0;
u_int8 G_TxDataByte = 0;
u_int8 G_RxDataByte = 0;
u_int32 G_TxFrameCnt = 0;
u_int32 G_RxFrameCnt = 0;

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
	char	*txDevice;
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
	printf("argc = %d\n",argc );
	if (argc < 3 || strcmp(argv[1],"-?")==0) {
		printf("Syntax: sync_test <rxDevice> <txDevice> <Data Length>\n");
		printf("Function: Test program for sync test of the Z147 driver\n");
		printf("%s \n",RCSid );
		return(1);
	}

	printf("%s \n",RCSid );

	rxDevice = argv[1];
	txDevice = argv[2];
	/*--------------------+
    |  open rxPath          |
    +--------------------*/
	if ((rxPath = M_open(rxDevice)) < 0) {
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
	/* install signal which will be received at change of input ports */
	UOS_SigInit( SignalHandler );
	UOS_SigInstall( UOS_SIG_USR1 );
	result = M_setstat(rxPath, Z147_SET_SIGNAL, UOS_SIG_USR1);
	if(result != 0){
		G_Errors++;
	}
	G_DataLen = 256;

	for(syncCfg = 2; syncCfg >= 0; syncCfg--){

		result = M_setstat(rxPath, Z147_RX_SYNC_CFG, syncCfg);
		if(result != 0){
			printf("Setting receive sync failed.\n");
			G_Errors++;
		}
		result = M_setstat(txPath, Z247_TX_DATA_RATE, 0);
		if(result != 0){
			printf("Setting transmit data rate failed.\n");
			G_Errors++;
		}
		result = M_setstat(rxPath, Z147_RX_DATA_RATE, 0);
		if(result != 0){
			printf("Setting receive data rate failed.\n");
			G_Errors++;
		}
		G_TxDataByte = 0;
		for(k=0;k<10;k++){
			printf("\n");
			printf("################### Test Run-%d (Sync Cfg-%d) ################\n", k+1, syncCfg);
			printf("----------------- Transmit -----------------\n");
			printf("User data length = %d and dataptr = 0x%x\n", G_DataLen, txDataArray);
			G_TxDataByte = 0;
			for(i=0;i<(G_DataLen-4);i++){
				if(i==G_TxDataByte){
					txDataArray[i] = G_TxDataByte++;
				}

			}
			printf("\n");
			printf("\n");
			result = M_setblock(txPath, (u_int8*)txDataArray, (G_DataLen *2) - 8);
			if(result > 0){
				printf("Transmitted %d bytes successfully\n", G_DataLen *2);
			}else{
				printf("Write failed with result %d\n", result);
				G_Errors++;
			}
			printf("--------------------------------\n");
			printf("\n");

			UOS_Delay( 4000);
			if(syncCfg == 2){
				while(1){
					result = M_getstat(rxPath, Z147_RX_IN_SYNC, &isSync);

					if((k!=0) && result != 0){
						printf("Sync failed\n");
						return -1;
					}

					if(result != 0){
						G_Errors++;
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
			G_TxFrameCnt++;
		}
	}

	result = M_setstat(txPath, Z247_DISABLE_TX, 1);
	if(result != 0){
		G_Errors++;
	}
	result = M_setstat(rxPath, Z147_DISABLE_RX, 1);
	if(result != 0){
		G_Errors++;
	}
	printf("-------------------------------------------\n");
	printf("-------------------------------------------\n\n");
	printf("Transmit frame count : %d\n", G_TxFrameCnt);
	printf("Receive frame count : %d\n", G_RxFrameCnt);
	printf("Test Result : ");
	if(G_Errors != 0){
		printf("FAILED\n");
	}else{
		printf("PASSED\n");
	}
	printf("\n-------------------------------------------\n");
	printf("-------------------------------------------\n");


	if (M_close(rxPath) < 0){
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

	if( sig == UOS_SIG_USR1 ) {
		++G_sigCount;
		result = M_getblock( rxPath, (u_int8*)rxDataArray, (G_DataLen *2) );
		if(result < 0){
			G_Errors++;
		}
		/* program lower five ports as outputs, others as inputs */
		if(result > 0){
			printf("----------------- Receive -----------------\n");
			printf("Receiving = %d bytes data\n", result);
			G_RxDataByte = 0;
			/* Add 4 sync words and ignore them in check. */
			for(j=0;j<(result/2);j++){
				if((rxDataArray[j] != ARINC717_SUB_1_SYNC) &&
						(rxDataArray[j] != ARINC717_SUB_2_SYNC) &&
						(rxDataArray[j] != ARINC717_SUB_3_SYNC) &&
						(rxDataArray[j] != ARINC717_SUB_4_SYNC))
				{
					if(rxDataArray[j] != (G_RxDataByte)){
						printf("Expected 0x%x but received rxDataArray[%d] = 0x%x\n",(G_RxDataByte), j, rxDataArray[j]);
						G_Errors++;
					}
					if((j<10) || (j>((result/2) - 5))){
						printf(" [%d]0x%x",j, rxDataArray[j]);
					}
					G_RxDataByte++;
				}
				/* Reset the buffer. */
				rxDataArray[j] = 0;
			}
			printf("\n");
			printf("Received %d bytes successfully\n", result);
			printf("--------------------------------\n");
			G_RxFrameCnt++;
		}else{
			printf("Read failed with the result = %d\n", result);
		}
	}
}

