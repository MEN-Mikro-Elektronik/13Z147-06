/****************************************************************************
 ************                                                    ************
 ************                   Z147_RATE                        ************
 ************                                                    ************
 ****************************************************************************/
/*!
 *         \file z146_rate_test.c
 *       \author Apatil
 *        $Date: 2009/07/10 13:40:12 $
 *    $Revision: 1.5 $
 *
 *       \brief  Program for the Z147 driver rate
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
u_int32 G_StartParam = 7;
/*--------------------------------------+
|   PROTOTYPES                          |
+--------------------------------------*/
static void PrintError(char *info);

MDIS_PATH txPath;
u_int16 txDataArray[32768];
int G_Errors = 0;
u_int32 G_DataLen = 0;
u_int8 G_DataByte = 0;
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

	char	*txDevice;
	int32 result = 0;
	int32 isSync = 0;
	u_int32 i = 0;
	int j = 0;
	int k = 0;
	int dataRate =0;
	u_int32 data = 0;


	u_int16 *txBuf = NULL;
	u_int16 *rxBuf = NULL;
	printf("argc = %d\n",argc );
	if (argc < 1 || strcmp(argv[1],"-?")==0) {
		printf("Syntax: timing_test_tx_part <txDevice> \n");
		printf("Function: Test program for rate test of the Z147 driver\n");
		printf("%s \n",RCSid );
		return(1);
	}

	printf("%s \n",RCSid );
	txDevice = argv[1];

	for(dataRate = 0; dataRate <= Z147_RX_DATA_RATE_8192; dataRate++){
		/*--------------------+
	    |  open rxPath        |
	    +--------------------*/

		if ((txPath = M_open(txDevice)) < 0) {
			PrintError("open");
			return(1);
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
		result = M_setstat(txPath, Z247_TX_DATA_RATE, dataRate);
		if(result != 0){
			printf("Setting transmit data rate failed.\n");
			G_Errors++;
		}


		for(k=0;k<20;k++){
			printf("\n");
			printf("################### Test Run-%d (Data Rate-%d) ################\n", k+1, dataRate);
			printf("----------------- Transmit -----------------\n");
			G_DataByte = 0;
			for(i=0;i<(G_DataLen - 4);i++){
//				txDataArray[i] = (G_StartParam + G_DataByte++);
				txDataArray[i] = (G_DataByte++);
				if((i<10) || (i>((G_DataLen - 4) - 5))){
					printf(" 0x%x", txDataArray[i]);
				}
			}
			G_StartParam++;
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

			G_TxFrameCnt++;
			UOS_Delay( 4000);
		}
		UOS_Delay( 4000);
		result = M_setstat(txPath, Z247_DISABLE_TX, 1);
		if(result != 0){
			G_Errors++;
		}
		if (M_close(txPath) < 0){
			PrintError("close");
		}
	}

	printf("-------------------------------------------\n");
	printf("-------------------------------------------\n\n");
	printf("Transmit frame count : %d\n", G_TxFrameCnt);

	printf("Test Result : ");
	if(G_Errors != 0){
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

