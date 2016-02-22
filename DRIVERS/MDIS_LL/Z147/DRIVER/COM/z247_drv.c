/*********************  P r o g r a m  -  M o d u l e ***********************/
/*!
 *        \file  z247_drv.c
 *
 *      \author  APatil
 *        $Date: 2009/08/03 16:43:18 $
 *    $Revision: 1.6 $
 *
 *      \brief   Low-level driver for ARINC429 transmitter
 *
 *     Required: OSS, DESC, DBG libraries
 *
 *     \switches _ONE_NAMESPACE_PER_DRIVER_
 */
/*-------------------------------[ History ]--------------------------------
 *
 * $Log: z247_drv.c,v $
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2004 by MEN Mikro Elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/

#define _NO_LL_HANDLE        /* ll_defs.h: don't define LL_HANDLE struct */

/*-----------------------------------------+
|  INCLUDES                                |
+-----------------------------------------*/
#include <MEN/men_typs.h>    /* system dependent definitions   */
#include <MEN/maccess.h>     /* hw access macros and types     */
#include <MEN/dbg.h>         /* debug functions                */
#include <MEN/oss.h>         /* oss functions                  */
#include <MEN/desc.h>        /* descriptor functions           */
#include <MEN/modcom.h>      /* ID PROM functions              */
#include <MEN/mdis_api.h>    /* MDIS global defs               */
#include <MEN/mdis_com.h>    /* MDIS common defs               */
#include <MEN/mdis_err.h>    /* MDIS error codes               */
#include <MEN/ll_defs.h>     /* low-level driver definitions   */

/*-----------------------------------------+
|  DEFINES                                 |
+-----------------------------------------*/

#define ADDRSPACE_COUNT    1          /**< nbr of required address spaces */
#define ADDRSPACE_SIZE     0x1000         /**< size of address space          */
#define CH_NUMBER          1          /**< number of device channels      */

#define USER_DATA_NOT_UPDATED 0
#define USER_DATA_UPDATED  1
/* debug defines */
#define DBG_MYLEVEL        llHdl->dbgLevel    /**< debug level  */
#define DBH                llHdl->dbgHdl      /**< debug handle */
#define OSH                llHdl->osHdl       /**< OS handle    */

/* descriptor key defines*/
#define RESET_DEFAULT      0          /**< default arwen reset (enabled)     */
#define RESET_OFF          1          /**< disables the arwen reset function */

#define Z247_IIR_STAT			0x0800		/**< Interrupt status register offset. */
#define Z247_TX_LSR_OFFSET		0x0801		/**< LSR register offset. */
#define Z247_TX_LSR_UE_RST		0x81		/**< Error reset value */
#define Z247_TX_TXC_OFFSET		0x0802		/**< TXC register offset. */
#define Z247_TX_TXA_OFFSET	    0x0804		/**< TXA register offset. */
#define Z247_TX_SUB_PTR_OFFSET	0x0806		/**< SUB_PTR register offset. */

#define Z247_TX_IER_OFFSET	    0x0808		/**< Offset of the TX_IER register */
#define Z247_TX_IER_DEFAULT	    0x3         /**< Default of the IER register */

#define Z247_TX_TLS_IRQ_OFFSET  0x0			/**< Transmit line status IRQ offset. */
#define Z247_TX_TLS_IRQ_MASK    0x1			/**< Transmit line status IRQ mask. */
#define Z247_TX_TSA_IRQ_OFFSET  0x1			/**< Transmit line status IRQ offset. */
#define Z247_TX_TSA_IRQ_MASK    0x2			/**< Transmit line status IRQ mask. */

#define Z247_TX_LCR_OFFSET		0x0809		/**< Offset of the TX_LCR register */
#define Z247_TX_LOOP_OFFSET		0			/**< Offset of the TX_LCR LOOP bit */
#define Z247_TX_LOOP_MASK  		0x01		/**< MASK of the TX_LCR LOOP bit */
#define Z247_TX_DATA_RATE_OFFSET	2			/**< Offset of the TX_LCR PAR_EN bit */
#define Z247_TX_DATA_RATE_MASK  	0x1C		/**< MASK of the TX_LCR Data Rate */
#define Z247_TX_MODE_OFFSET		4			/**< Offset of the TX_LCR PAR_TYP bit */
#define Z247_TX_MODE_MASK  		0x20		/**< MASK of the TX_LCR PAR_TYP bit */
#define Z247_TX_SLW_OFFSET		5			/**< Offset of the TX_LCR SDI_EN bit */
#define Z247_TX_SLW_MASK  		0x20		/**< MASK of the TX_LCR SDI_EN bit */

#define Z247_TX_LCR_DEFAULT		0x80		/**< Offset of the TX_LCR register */

#define Z247_TX_FCR_OFFSET		0x080A		/**< Offset of the TX_FCR register */
#define Z247_TX_FCR_DEFAULT 	0x07			/**< Default of the TX_FCR register */
#define Z247_TX_FCR_MASK    	0x7			/**< Mask of the TX_FCR register */

#define Z247_TX_RST_OFFSET		0x080B		/**< Offset of the TX_RST register */

#define Z247_FIFO_START_ADDR    0x0000		/**< Start address of the hardware FIFO of the transmitter. */
#define Z247_TX_FIFO_MAX 		1023		/**< Size of the hardware FIFO (in word) of the transmitter. */

#define Z247_TX_WRITE_MAX 		16		/**< Size of the hardware FIFO (in word) of the transmitter. */

#define Z247_TX_DATA_RATE_64     0    /**< Set data rate of 64 words/sec. */
#define Z247_TX_DATA_RATE_128    1    /**< Set data rate of 128 words/sec. */
#define Z247_TX_DATA_RATE_256    2    /**< Set data rate of 256 words/sec. */
#define Z247_TX_DATA_RATE_512    3    /**< Set data rate of 512 words/sec. */
#define Z247_TX_DATA_RATE_1024   4    /**< Set data rate of 1024 words/sec. */
#define Z247_TX_DATA_RATE_2048   5    /**< Set data rate of 2048 words/sec. */
#define Z247_TX_DATA_RATE_4096   6    /**< Set data rate of 4096 words/sec. */
#define Z247_TX_DATA_RATE_8192   7    /**< Set data rate of 8192 words/sec. */

#define Z247_TX_TRIG_LVL_08   	 1    /**< Set trigger level to 8 words. */
#define Z247_TX_TRIG_LVL_16   	 2    /**< Set trigger level to 16 words. */
#define Z247_TX_TRIG_LVL_32      3    /**< Set trigger level to 32 words. */
#define Z247_TX_TRIG_LVL_64   	 4    /**< Set trigger level to 64 words. */
#define Z247_TX_TRIG_LVL_128  	 5    /**< Set trigger level to 128 words. */
#define Z247_TX_TRIG_LVL_256   	 6    /**< Set trigger level to 256 words. */
#define Z247_TX_TRIG_LVL_512     7    /**< Set trigger level to 512 words. */
/*-----------------------------------------+
|  TYPEDEFS                                |
+-----------------------------------------*/
/** low-level handle */
typedef struct {
	/* general */
	int32                   memAlloc;       /**< size allocated for the handle */
	OSS_HANDLE              *osHdl;         /**< oss handle        */
	OSS_IRQ_HANDLE          *irqHdl;        /**< irq handle        */
	DESC_HANDLE             *descHdl;       /**< desc handle       */
	MACCESS                 ma;             /**< hw access handle  */
	MDIS_IDENT_FUNCT_TBL    idFuncTbl;      /**< id function table */
	/* debug */
	u_int32                 dbgLevel;       /**< debug level  */
	DBG_HANDLE              *dbgHdl;        /**< debug handle */

	OSS_SIG_HANDLE          *portChangeSig; /**< signal for port change */
	OSS_SIG_HANDLE          *tlsErrorSig;   /**< signal for transmitter line status. */

	/* toggle mode */
	OSS_ALARM_HANDLE        *alarmHdl;      /**< alarm handle               */
	OSS_SEM_HANDLE          *devSemHdl;     /**< device semaphore handle    */

	/* Buffer for user frame. */
	u_int16*					usrBuffer;
	volatile u_int32 		usrBufferSize;
	u_int8					isUsrDataUpdated;

	/* Ring buffer for driver transmission */
	u_int16*				drvRingBuffer;
	volatile u_int32 		drvRingTail;
	volatile u_int32 		drvRingDataCnt;
	volatile u_int32 		drvRingSize;

	u_int32					writeBlockSize;
	u_int8 					disableTx;
	u_int8					isTxIrqExit;
	u_int32 				txFrameCnt;
} LL_HANDLE;

/* include files which need LL_HANDLE */
#include <MEN/ll_entry.h>       /* low-level driver jump table */
#include <MEN/z247_drv.h>        /* Z247 driver header file      */

/*-----------------------------------------+
|  PROTOTYPES                              |
+-----------------------------------------*/
static int32 Z247_Init(DESC_SPEC *descSpec, OSS_HANDLE *osHdl,
		MACCESS *ma, OSS_SEM_HANDLE *devSemHdl,
		OSS_IRQ_HANDLE *irqHdl, LL_HANDLE **llHdlP);
static int32 Z247_Exit(LL_HANDLE **llHdlP);
static int32 Z247_Read(LL_HANDLE *llHdl, int32 ch, int32 *value);
static int32 Z247_Write(LL_HANDLE *llHdl, int32 ch, int32 value);
static int32 Z247_SetStat(LL_HANDLE *llHdl,int32 ch, int32 code,
		INT32_OR_64 value32_or_64);
static int32 Z247_GetStat(LL_HANDLE *llHdl, int32 ch, int32 code,
		INT32_OR_64 *value32_or64P);
static int32 Z247_BlockRead(LL_HANDLE *llHdl, int32 ch, void *buf, int32 size,
		int32 *nbrRdBytesP);
static int32 Z247_BlockWrite(LL_HANDLE *llHdl, int32 ch, void *buf, int32 size,
		int32 *nbrWrBytesP);

static int32 Z247_Irq(LL_HANDLE *llHdl);

static int32 Z247_Info(int32 infoType, ...);
static char* Ident(void);
static int32 Cleanup(LL_HANDLE *llHdl, int32 retCode);
static void  ConfigureDefault( LL_HANDLE *llHdl );
static int HwWrite(LL_HANDLE    *llHdl);
static int SetDataRate(LL_HANDLE *llHdl, u_int8 txSpeed);
static void RegStatus(LL_HANDLE *llHdl );
static u_int16 ReadFromBuffer( LL_HANDLE *llHdl);


/****************************** Z247_GetEntry ********************************/
/** Initialize driver's jump table
 *
 *  \param drvP     \OUT pointer to the initialized jump table structure
 */
#ifdef _ONE_NAMESPACE_PER_DRIVER_
extern void LL_GetEntry(
		LL_ENTRY* drvP
)
#else
extern void __Z247_GetEntry(
		LL_ENTRY* drvP
)
#endif
{
	drvP->init        = Z247_Init;
	drvP->exit        = Z247_Exit;
	drvP->read        = Z247_Read;
	drvP->write       = Z247_Write;
	drvP->blockRead   = Z247_BlockRead;
	drvP->blockWrite  = Z247_BlockWrite;
	drvP->setStat     = Z247_SetStat;
	drvP->getStat     = Z247_GetStat;
	drvP->irq         = Z247_Irq;
	drvP->info        = Z247_Info;
}

/******************************** Z247_Init **********************************/
/** Allocate and return low-level handle, initialize hardware
 *
 * The function initializes all channels with the definitions made
 * in the descriptor. The interrupt is disabled.
 *
 * The following descriptor keys are used:
 *
 * \code
 * Descriptor key        Default          Range
 * --------------------  ---------------  -------------
 * DEBUG_LEVEL_DESC      OSS_DBG_DEFAULT  see dbg.h
 * DEBUG_LEVEL           OSS_DBG_DEFAULT  see dbg.h
 * ID_CHECK              1                0..1
 * \endcode
 *
 *  \param descP      \IN  pointer to descriptor data
 *  \param osHdl      \IN  oss handle
 *  \param ma         \IN  hw access handle
 *  \param devSemHdl  \IN  device semaphore handle
 *  \param irqHdl     \IN  irq handle
 *  \param llHdlP     \OUT pointer to low-level driver handle
 *
 *  \return           \c 0 on success or error code
 */
static int32 Z247_Init(
		DESC_SPEC       *descP,
		OSS_HANDLE      *osHdl,
		MACCESS         *ma,
		OSS_SEM_HANDLE  *devSemHdl,
		OSS_IRQ_HANDLE  *irqHdl,
		LL_HANDLE       **llHdlP
)
{
	LL_HANDLE *llHdl = NULL;
	u_int32 gotsize;
	int32 error;
	u_int32 value;

	/*------------------------------+
	|  prepare the handle           |
	+------------------------------*/
	*llHdlP = NULL;		/* set low-level driver handle to NULL */

	/* alloc */
	if ((llHdl = (LL_HANDLE*)OSS_MemGet(
			osHdl, sizeof(LL_HANDLE), &gotsize)) == NULL)
		return (ERR_OSS_MEM_ALLOC);

	/* clear */
	OSS_MemFill(osHdl, gotsize, (char*)llHdl, 0x00);

	/* init */
	llHdl->memAlloc    = gotsize;
	llHdl->osHdl       = osHdl;
	llHdl->irqHdl      = irqHdl;
	llHdl->ma          = *ma;
	llHdl->devSemHdl   = devSemHdl;

	/*------------------------------+
	|  init id function table       |
	+------------------------------*/
	/* driver's ident function */
	llHdl->idFuncTbl.idCall[0].identCall = Ident;
	/* library's ident functions */
	llHdl->idFuncTbl.idCall[1].identCall = DESC_Ident;
	llHdl->idFuncTbl.idCall[2].identCall = OSS_Ident;
	/* terminator */
	llHdl->idFuncTbl.idCall[3].identCall = NULL;
	llHdl->disableTx = 0;
	llHdl->drvRingDataCnt = 0;
	llHdl->txFrameCnt = 0;
	llHdl->writeBlockSize = 0;
	/*------------------------------+
	|  prepare debugging            |
	+------------------------------*/
	DBG_MYLEVEL = OSS_DBG_DEFAULT;		/* set OS specific debug level */
	DBGINIT((NULL,&DBH));

	/*------------------------------+
	|  scan descriptor              |
	+------------------------------*/
	/* prepare access */
	if ((error = DESC_Init(descP, osHdl, &llHdl->descHdl)))
		return (Cleanup(llHdl, error));

	/* DEBUG_LEVEL_DESC */
	if ((error = DESC_GetUInt32(llHdl->descHdl, OSS_DBG_DEFAULT,
			&value, "DEBUG_LEVEL_DESC")) &&
			error != ERR_DESC_KEY_NOTFOUND)
		return (Cleanup(llHdl, error));

	DESC_DbgLevelSet(llHdl->descHdl, value);	/* set level */

	/* DEBUG_LEVEL */
	if ((error = DESC_GetUInt32(llHdl->descHdl, OSS_DBG_DEFAULT,
			&llHdl->dbgLevel, "DEBUG_LEVEL")) &&
			error != ERR_DESC_KEY_NOTFOUND)
		return (Cleanup(llHdl, error));

	DBGWRT_1((DBH, "Z247_Init: base address = %08p\n", (void*)llHdl->ma));

	/*------------------------------+
	|  init hardware                |
	+------------------------------*/
	ConfigureDefault(llHdl);

	/* set low-level driver handle */
	*llHdlP = llHdl;
	IDBGWRT_2((DBH, ">>> LL - Z247_Init: Register status at the end of Init\n"));
	RegStatus(llHdl);
	return (ERR_SUCCESS);
}

/****************************** Z247_Exit ************************************/
/** De-initialize hardware and clean up memory
 *
 *  The function deinitializes all channels by setting them as inputs.
 *  The interrupt is disabERR_SUCCESSled.
 *
 *  \param llHdlP     \IN  pointer to low-level driver handle
 *
 *  \return           \c 0 on success or error code
 */
static int32 Z247_Exit(
		LL_HANDLE **llHdlP
)
{
	LL_HANDLE *llHdl = *llHdlP;
	int32 error = 0;


	int timeoutCnt = 20;
	while(llHdl->isTxIrqExit != 0){
		if(timeoutCnt > 0){
			/* Else wait for some time and check again */
			timeoutCnt--;
			OSS_Delay(OSH, 100);
		}else{
			/* Exit the loop and return. */
			return ERR_LL_DEV_BUSY;
		}
	}
	/* Disable the interrupt */
	MWRITE_D8(llHdl->ma, Z247_TX_IER_OFFSET, 0);
	DBGWRT_2((DBH, "Z247_Exit\n"));
	IDBGWRT_2((DBH, ">>> LL - Z247_Exit: Register status in Exit before CleanUp\n"));
		RegStatus(llHdl);
	/*------------------------------+
	|  clean up memory              |
	+------------------------------*/
	*llHdlP = NULL;		/* set low-level driver handle to NULL */
	error = Cleanup(llHdl, error);

	return (error);
}

/****************************** Z247_Read ************************************/
/** Read a value from the device
 *
 *  The function reads the current state of all port pins.
 *
 *  \param llHdl      \IN  low-level handle
 *  \param ch         \IN  current channel
 *  \param valueP     \OUT read value
 *
 *  \return           \c 0 on success or error code
 */
static int32 Z247_Read(
		LL_HANDLE *llHdl,
		int32 ch,
		int32 *valueP
)
{
	DBGWRT_1((DBH, "LL - Z247_Read: ch=%d, valueP=%d\n",ch,*valueP));

	return( ERR_LL_ILL_FUNC );
}

/****************************** Z247_Write ***********************************/
/** Description:  Write a value to the device
 *
 *  The function writes a value to the ports which are programmed as outputs.
 *
 *  \param llHdl      \IN  low-level handle
 *  \param ch         \IN  current channel
 *  \param value      \IN  value to write
 *
 *  \return           \c 0 on success or error code
 */
static int32 Z247_Write(
		LL_HANDLE *llHdl,
		int32 ch,
		int32 value
)
{
	DBGWRT_1((DBH, "LL - Z247_Write: ch=%d, valueP=%d\n",ch,value));

	return( ERR_LL_ILL_FUNC );
}

/****************************** Z247_SetStat *********************************/
/** Set the driver status
 *
 *  The driver supports \ref getstat_setstat_codes "these status codes"
 *  in addition to the standard codes (see mdis_api.h).
 *  Note: only inputs are able fire an interrupt
 *
 *  \param llHdl         \IN  low-level handle
 *  \param code          \IN  \ref getstat_setstat_codes "status code"
 *  \param ch            \IN  current channel
 *  \param value32_or_64 \IN  data or pointer to block data structure
 *                            (M_SG_BLOCK) for block status codes
 *  \return              \c 0 on success or error code
 */
static int32 Z247_SetStat(
		LL_HANDLE   *llHdl,
		int32       code,
		int32       ch,
		INT32_OR_64 value32_or_64
)
{
	int32 value = (int32)value32_or_64;		/* 32bit value */

	MACCESS ma = llHdl->ma;
	int32 error = ERR_SUCCESS;
	u_int8 regData = 0;

	DBGWRT_1((DBH, "LL - Z247_SetStat: ch=%d code=0x%04x value=0x%x\n",
			ch, code, value));

	switch (code) {
	/*--------------------------+
		|  debug level              |
		+--------------------------*/
	case M_LL_DEBUG_LEVEL:
		llHdl->dbgLevel = value;
		break;
		/*--------------------------+
		|  enable interrupts        |
		+--------------------------*/
	case M_MK_IRQ_ENABLE:
		break;

		/*--------------------------+
		|  channel direction        |
		+--------------------------*/
	case M_LL_CH_DIR:
		if (value != M_CH_INOUT) {
			error = ERR_LL_ILL_DIR;
		}
		break;

		/*--------------------------+
		|  register signal          |
		+--------------------------*/
	case Z247_SET_SIGNAL:
		/* signal already installed ? */
		if (llHdl->portChangeSig) {
			error = ERR_OSS_SIG_SET;
			break;
		}
		error = OSS_SigCreate(OSH, value, &llHdl->portChangeSig);
		break;
		/*--------------------------+
		|  register tls signal      |
		+--------------------------*/
	case Z247_SET_ERR_SIGNAL:
		/* signal already installed ? */
		if (llHdl->tlsErrorSig) {
			error = ERR_OSS_SIG_SET;
			break;
		}
		error = OSS_SigCreate(OSH, value, &llHdl->tlsErrorSig);
		break;
		/*--------------------------+
		|  unregister signal        |
		+--------------------------*/
	case Z247_CLR_SIGNAL:
		/* signal already installed ? */
		if (llHdl->portChangeSig == NULL) {
			error = ERR_OSS_SIG_CLR;
			break;
		}
		error = OSS_SigRemove(OSH, &llHdl->portChangeSig);
		break;

		/*--------------------------+
		|  unregister signal        |
		+--------------------------*/
	case Z247_CLR_ERR_SIGNAL:
		/* signal already installed ? */
		if (llHdl->tlsErrorSig == NULL) {
			error = ERR_OSS_SIG_CLR;
			break;
		}
		error = OSS_SigRemove(OSH, &llHdl->tlsErrorSig);
		break;
		/*--------------------------+
		|  Interrupt enable            |
		+--------------------------*/
	case Z247_TX_TXCIEN_STAT:
		/* Enable interrupt */
		MWRITE_D8(llHdl->ma, Z247_TX_IER_OFFSET, (u_int8)value32_or_64);
		break;

		/*--------------------------+
		|  Loopback mode            |
		+--------------------------*/
	case Z247_LOOPBACK:
		/* Set loopback mode */
		regData = MREAD_D8(llHdl->ma, Z247_TX_LCR_OFFSET);
		if(value32_or_64 != 0){
			regData = regData | Z247_TX_LOOP_MASK;
		}else{
			regData = regData & (~Z247_TX_LOOP_MASK);
		}
		MWRITE_D8(llHdl->ma, Z247_TX_LCR_OFFSET, regData);
		break;

		/*--------------------------+
		|  Transmit Rate            |
		+--------------------------*/
	case Z247_TX_DATA_RATE:
		if((value >= 0) && (value <= Z247_TX_DATA_RATE_8192)){

			/* Set the reset signal to active. */
			MWRITE_D8(llHdl->ma, Z247_TX_RST_OFFSET, 1);
			/* Disable the interrupt. */
			MWRITE_D8(llHdl->ma, Z247_TX_IER_OFFSET, 0);
			llHdl->disableTx = 1;
			error = SetDataRate(llHdl, (u_int8)value);
			/* Reset the signal. */
			MWRITE_D8(llHdl->ma, Z247_TX_RST_OFFSET, 0);
		}else{
			error = ERR_LL_ILL_PARAM;
		}
		break;

     	/*--------------------------------------+
		|  Transmit threshold level status    |
		+---------------------------------------*/
	case Z247_TX_THR_LEV:
		regData = (value32_or_64 & Z247_TX_FCR_MASK);
		MWRITE_D8(llHdl->ma, Z247_TX_FCR_OFFSET, regData);
		break;

     	/*-------------------------+
		|  Disable transmission    |
		+-------------------------*/
	case Z247_DISABLE_TX:
		llHdl->disableTx = 1;
		break;

		/*--------------------------+
		|  (unknown)                |
		+--------------------------*/
	default:
		error = ERR_LL_UNK_CODE;
	}

	return (error);
}

/****************************** Z247_GetStat *********************************/
/** Get the driver status
 *
 *  The driver supports \ref getstat_setstat_codes "these status codes"
 *  in addition to the standard codes (see mdis_api.h).
 *  \param llHdl             \IN  low-level handle
 *  \param code              \IN  \ref getstat_setstat_codes "status code"
 *  \param ch                \IN  current channel
 *  \param value32_or_64P    \IN  pointer to block data structure (M_SG_BLOCK) for
 *                                block status codes
 *  \param value32_or_64P    \OUT data pointer or pointer to block data structure
 *                                (M_SG_BLOCK) for block status codes
 *
 *  \return                  \c 0 on success or error code
 */
static int32 Z247_GetStat(
		LL_HANDLE   *llHdl,
		int32       code,
		int32       ch,
		INT32_OR_64 *value32_or_64P
)
{
	int32 *valueP = (int32*)value32_or_64P;		/* pointer to 32bit value */
	INT32_OR_64 *value64P = value32_or_64P;		/* stores 32/64bit pointer */
	int32 error = ERR_SUCCESS;
	int32 regData = 0;


	DBGWRT_1((DBH, "LL - Z247_GetStat: ch=%d code=0x%04x\n", ch, code));

	switch (code) {
	/*--------------------------+
		|  debug level              |
		+--------------------------*/
	case M_LL_DEBUG_LEVEL:
		*valueP = llHdl->dbgLevel;
		break;

		/*--------------------------+
		|  number of channels       |
		+--------------------------*/
	case M_LL_CH_NUMBER:
		*valueP = CH_NUMBER;
		break;

		/*--------------------------+
		|  channel direction        |
		+--------------------------*/
	case M_LL_CH_DIR:
		*valueP = M_CH_INOUT;
		break;

		/*--------------------------+
		|  channel type info        |
		+--------------------------*/
	case M_LL_CH_TYP:
		*valueP = M_CH_BINARY;
		break;

		/*--------------------------+
		|  ID PROM check enabled    |
		+--------------------------*/
	case M_LL_ID_CHECK:
		*valueP = 0;
		break;
		/*--------------------------+
		|  ident table pointer      |
		|  (treat as non-block!)    |
		+--------------------------*/
	case M_MK_BLK_REV_ID:
		*value64P = (INT32_OR_64)&llHdl->idFuncTbl;
		break;

		/*--------------------------+
		|  Interrupt enable            |
		+--------------------------*/
	case Z247_TX_TXCIEN_STAT:
		*value64P = (INT32_OR_64)(MREAD_D8(llHdl->ma, Z247_TX_IER_OFFSET));
		break;

		/*--------------------------+
		|  Loopback mode            |
		+--------------------------*/
	case Z247_LOOPBACK:
		regData = MREAD_D8(llHdl->ma, Z247_TX_LCR_OFFSET);
		*value64P = (INT32_OR_64)((regData & Z247_TX_LOOP_MASK) >> Z247_TX_LOOP_OFFSET);

		break;

		/*------------------+
		|  Transmit rate    |
		+------------------*/
	case Z247_TX_DATA_RATE:
		regData = MREAD_D8(llHdl->ma, Z247_TX_LCR_OFFSET);
		*value64P = (INT32_OR_64)((regData & Z247_TX_DATA_RATE_MASK) >> Z247_TX_DATA_RATE_OFFSET);
		break;

		/*--------------------------------------+
		|  Transmit threshold level status    |
		+---------------------------------------*/
	case Z247_TX_THR_LEV:
		*value64P = (MREAD_D8(llHdl->ma, Z247_TX_FCR_OFFSET) & Z247_TX_FCR_MASK);
		break;

		/*--------------------------+
		|  (unknown)                |
		+--------------------------*/
	default:
		error = ERR_LL_UNK_CODE;
		break;
	}

	return (error);
}

/******************************* Z247_BlockRead ******************************/
/** Read a data block from the device
 *
 *  \param llHdl       \IN  low-level handle
 *  \param ch          \IN  current channel
 *  \param buf         \IN  data buffer
 *  \param size        \IN  data buffer size
 *  \param nbrRdBytesP \OUT number of read bytes
 *
 *  \return            \c 0 on success or error code
 */
static int32 Z247_BlockRead(
		LL_HANDLE *llHdl,
		int32     ch,
		void      *buf,
		int32     size,
		int32     *nbrRdBytesP
)
{
	DBGWRT_1((DBH, "LL - Z247_BlockRead: ch=%d, size=%d\n",ch,size));

	/* return number of read bytes */
	*nbrRdBytesP = 0;

	return( ERR_LL_ILL_FUNC );
}

/****************************** Z247_BlockWrite *****************************/
/** Write a data block from the device
 *
 *  \param llHdl  	   \IN  low-level handle
 *  \param ch          \IN  current channel
 *  \param buf         \IN  data buffer
 *  \param size        \IN  data buffer size
 *  \param nbrWrBytesP \OUT number of written bytes
 *
 *  \return            \c 0 on success or error code
 */
static int32 Z247_BlockWrite(
		LL_HANDLE *llHdl,
		int32     ch,
		void      *buf,
		int32     size,
		int32     *nbrWrBytesP
)
{
	int32 result = ERR_SUCCESS;
	u_int32 llDataLen = size/2;
	u_int16 * userBuf = (u_int16*)buf;

	DBGWRT_2((DBH, ">>> LL - LL - Z247_BlockWrite: size=%d \n",size));
	//		for(i=0;i<size;i++){
	//			DBGWRT_1((DBH, "LL - Z247_Write: user Data[%d] = %d\n",i, *((u_int32*)buf + i)));
	//		}

	/* Check for user buffer size */
	if((size != 0) && (buf != NULL)){
		if(llDataLen == llHdl->usrBufferSize){
			/* Copy data from user space to kernel space (ring buffer). */

			OSS_MemCopy(OSH, size, (char*)userBuf, (char*)llHdl->usrBuffer);
			/* Set the indication of the new data. */
			llHdl->isUsrDataUpdated = USER_DATA_UPDATED;
			/* Configure the interrupts */
			MWRITE_D8(llHdl->ma, Z247_TX_IER_OFFSET, Z247_TX_IER_DEFAULT);
		}else{
			result = ERR_MBUF_ILL_SIZE;
		}
	}else{
		result = ERR_MBUF_ILL_SIZE;
	}

	/* Return number of written bytes as per the result status. */
	if(result == ERR_SUCCESS){
		*nbrWrBytesP = size;
	}else{
		*nbrWrBytesP = 0;
	}
	IDBGWRT_2((DBH, ">>> LL - Z247_BlockWrite: At the end of Z247_BlockWrite\n"));
	RegStatus(llHdl);
	return result;
}


/****************************** Z247_Irq ************************************/
/** Interrupt service routine
 *
 *  The interrupt is triggered when transmit FIFO has space to accept more data.
 *  Then the remaining data is transmitted. If data size is more than the FIFO size
 *  then the interrupt will be set again. If the data fits in the fifo then the
 *  interrupt will be disabled.
 *  \param llHdl       \IN  low-level handle
 *  \return LL_IRQ_DEVICE   irq caused by device
 *          LL_IRQ_DEV_NOT  irq not caused by device
 *          LL_IRQ_UNKNOWN  unknown
 */
static int32 Z247_Irq(
		LL_HANDLE *llHdl
)
{
	int32 result = 0;
	u_int32 irqReq = 0;

	/* interrupt caused by TX ? */
	irqReq = MREAD_D8(llHdl->ma, Z247_IIR_STAT);

	if(irqReq & Z247_TX_TLS_IRQ_MASK){
		/* Reset the underflow error. */
		MWRITE_D8(llHdl->ma, Z247_TX_LSR_OFFSET, Z247_TX_LSR_UE_RST);

		/////
		// Is System Reset required ?????
		/////

		/* if requested send error signal to application ??? Is it correct ??? what shall be the signal inhalt? */
		if (llHdl->portChangeSig){
			OSS_SigSend(OSH, llHdl->portChangeSig);
		}

	}else if (irqReq & Z247_TX_TSA_IRQ_MASK) {

		IDBGWRT_3((DBH, ">>> LL - Z247_Irq: request %08x\n", irqReq));

		/* Else disable the queue space interrupt. */
		MWRITE_D8(llHdl->ma, Z247_TX_IER_OFFSET, 0);
		/* interrupt is cleared by disabling it.  */

		/* Call the tx routine to send remaining data. */
		HwWrite(llHdl);

		if(llHdl->disableTx == 0){
			/* Enable the queue space interrupt. */
			MWRITE_D8(llHdl->ma, Z247_TX_IER_OFFSET, Z247_TX_IER_DEFAULT);
		}else{
			llHdl->isTxIrqExit = 1;
		}

		IDBGWRT_2((DBH, ">>> LL - Z247_Irq: Status after HwWrite:\n"));
		RegStatus(llHdl);
		result = LL_IRQ_DEVICE;
	}else{
		result = LL_IRQ_DEV_NOT;
	}

	return result;

}

/****************************** Z247_Info ***********************************/
/** Get information about hardware and driver requirements
 *
 *  The following info codes are supported:
 *
 * \code
 *  Code                      Description
 *  ------------------------  -----------------------------
 *  LL_INFO_HW_CHARACTER      hardware characteristics
 *  LL_INFO_ADDRSPACE_COUNT   nr of required address spaces
 *  LL_INFO_ADDRSPACE         address space information
 *  LL_INFO_IRQ               interrupt required
 *  LL_INFO_LOCKMODE          process lock mode required
 * \endcode
 *
 *  The LL_INFO_HW_CHARACTER code returns all address and
 *  data modes (ORed) which are supported by the hardware
 *  (MDIS_MAxx, MDIS_MDxx).
 *
 *  The LL_INFO_ADDRSPACE_COUNT code returns the number
 *  of address spaces used by the driver.
 *
 *  The LL_INFO_ADDRSPACE sizecode returns information about one
 *  specific address space (MDIS_MAxx, MDIS_MDxx). The returned
 *  data mode represents the widest hardware access used by
 *  the driver.
 *
 *  The LL_INFO_IRQ code returns whether the driver supports an
 *  interrupt routine (TRUE or FALSE).
 *
 *  The LL_INFO_LOCKMODE code returns which process locking
 *  mode the driver needs (LL_LOCK_xxx).
 *
 *  \param infoType    \IN  info code
 *  \param ...         \IN  argument(s)
 *
 *  \return            \c 0 on success or error code
 */
static int32 Z247_Info(
		int32 infoType,
		...
)
{
	int32   error = ERR_SUCCESS;
	va_list argptr;

	va_start(argptr, infoType);

	switch (infoType) {
	/*-------------------------------+
		|  hardware characteristics      |
		|  (all addr/data modes ORed)    |
		+-------------------------------*/
	case LL_INFO_HW_CHARACTER:
	{
		u_int32 *addrModeP = va_arg(argptr, u_int32*);
		u_int32 *dataModeP = va_arg(argptr, u_int32*);

		*addrModeP = MDIS_MA08;
		*dataModeP = MDIS_MD08 | MDIS_MD16;
		break;
	}

	/*-------------------------------+
		|  nr of required address spaces |
		|  (total spaces used)           |
		+-------------------------------*/
	case LL_INFO_ADDRSPACE_COUNT:
	{
		u_int32 *nbrOfAddrSpaceP = va_arg(argptr, u_int32*);

		*nbrOfAddrSpaceP = ADDRSPACE_COUNT;
		break;
	}
	/*-------------------------------+
		|  address space type            |
		|  (widest used data mode)       |
		+-------------------------------*/
	case LL_INFO_ADDRSPACE:
	{
		u_int32 addrSpaceIndex = va_arg(argptr, u_int32);
		u_int32 *addrModeP = va_arg(argptr, u_int32*);
		u_int32 *dataModeP = va_arg(argptr, u_int32*);
		u_int32 *addrSizeP = va_arg(argptr, u_int32*);

		if (addrSpaceIndex >= ADDRSPACE_COUNT) {
			error = ERR_LL_ILL_PARAM;
		} else {
			*addrModeP = MDIS_MA08;
			*dataModeP = MDIS_MD16;
			*addrSizeP = ADDRSPACE_SIZE;
		}
		break;
	}
	/*-------------------------------+
		|  interrupt required            |
		+-------------------------------*/
	case LL_INFO_IRQ:
	{
		u_int32 *useIrqP = va_arg(argptr, u_int32*);

		*useIrqP = 1;
		break;
	}
	/*-------------------------------+
		|  process lock mode             |
		+-------------------------------*/
	case LL_INFO_LOCKMODE:
	{
		u_int32 *lockModeP = va_arg(argptr, u_int32*);

		*lockModeP = LL_LOCK_NONE;
		break;
	}
	/*-------------------------------+
		|  (unknown)                     |
		+-------------------------------*/
	default:
		error = ERR_LL_ILL_PARAM;
	}

	va_end(argptr);

	return (error);
}

/******************************** Ident ************************************/
/** Return ident string
 *
 *  \return            pointer to ident string
 */
static char* Ident(void)
{
	return ("Z247 - Z247 low level driver: $Id: z247_drv.c,v 1.11 2015/09/23 14:17:37 APatil");
}

/********************************* Cleanup *********************************/
/** Close all handles, free memory and return error code
 *
 *  \warning The low-level handle is invalid after this function is called.
 *
 *  \param llHdl      \IN  low-level handle
 *  \param retCode    \IN  return value
 *
 *  \return           \IN  retCode
 */
static int32 Cleanup(
		LL_HANDLE *llHdl,
		int32     retCode
)
{
	/*------------------------------+
	|  close handles                |
	+------------------------------*/
	/* clean up desc */
	if (llHdl->descHdl)
		retCode = DESC_Exit(&llHdl->descHdl);

	/* clean up debug */
	DBGEXIT((&DBH));

	/*------------------------------+
	|  free memory                  |
	+------------------------------*/
	/* Delete the allocated memory. */
	if(llHdl->usrBuffer != NULL){
		OSS_MemFree(llHdl->osHdl, (int8*)llHdl->usrBuffer, llHdl->usrBufferSize *2);
		llHdl->usrBuffer = NULL;
	}
	if(llHdl->usrBuffer != NULL){
		OSS_MemFree(llHdl->osHdl, (int8*)llHdl->drvRingBuffer, llHdl->drvRingSize * 2);
		llHdl->drvRingBuffer = NULL;
	}

	/* free my handle */
	OSS_MemFree(llHdl->osHdl, (int8*)llHdl, llHdl->memAlloc);

	/*return error code */
	return (retCode);
}

/**********************************************************************/
/** Configure default values to registers.
 *
 *  Sets the controller registers to default values:
 *
 *  \param llHdl      \IN  low-level handle
 */

static void
ConfigureDefault( LL_HANDLE *llHdl )
{

	MWRITE_D8(llHdl->ma, Z247_TX_RST_OFFSET, 1);

	/* Configure TX LCR */
	MWRITE_D8(llHdl->ma, Z247_TX_LCR_OFFSET, Z247_TX_LCR_DEFAULT);

	/* Set the speed */
	SetDataRate(llHdl, Z247_TX_DATA_RATE_64);

	MWRITE_D8(llHdl->ma, Z247_TX_RST_OFFSET, 0);

}

/********************************* HwWrite **********************************
 *
 *  Description: Write data to the hardware register
 *
 *---------------------------------------------------------------------------
 *  Input......: llHdl		low-level handle
 *               retCode    return value
 *  Output.....: return	    retCode
 *  Globals....: -
 ****************************************************************************/
int HwWrite(LL_HANDLE    *llHdl){
	int32 result = ERR_SUCCESS;
	u_int32 dataCount = 0;
	u_int16 data = 0;
	u_int16 i = 0;
	u_int16 txcStatus = 0;

	DBGWRT_3((DBH, "LL - Z247_Write: \n"));

	/* Check TXC register for remaining space in the TX queue. */
	txcStatus = MREAD_D16(llHdl->ma, Z247_TX_TXC_OFFSET);
	DBGWRT_2((DBH, "LL - Z247_Write: txcStatus = %d\n", txcStatus));
	DBGWRT_2((DBH, "LL - Z247_Write: llHdl->writeBlockSize = %d\n", llHdl->writeBlockSize));

	/* Fill the complete queue. */
	dataCount = (llHdl->writeBlockSize - txcStatus);
	DBGWRT_2((DBH, "LL - Z247_Write: writing %d bytes\n", dataCount));

	/* If enough space then write the data to the queue */
	if(dataCount > 0){
		// Need to add support for user buffer tx.
		for(i=0;i<dataCount;i++){
			data = ReadFromBuffer(llHdl);
			MWRITE_D16(llHdl->ma, Z247_FIFO_START_ADDR + (i*2),  data);
		}

		/* Acknowledge the the data before enabling the queue space interrupt. */
		MWRITE_D16(llHdl->ma, Z247_TX_TXA_OFFSET, dataCount);
		DBGWRT_2((DBH, "LL - Z247_Write:HWWrite Z247_TX_TXA_OFFSET = %d\n",dataCount));
	} /* Else, do nothing. */

	return result;
}

///********************************* HwWrite **********************************
// *
// *  Description: Write data to the hardware register
// *
// *---------------------------------------------------------------------------
// *  Input......: llHdl		low-level handle
// *               retCode    return value
// *  Output.....: return	    retCode
// *  Globals....: -
// ****************************************************************************/
//int HwWrite(LL_HANDLE    *llHdl){
//	int32 result = ERR_SUCCESS;
//	int8 ringResult = 0;
//	u_int32 dataBitMask = 0;
//
//	u_int32 dataCount = 0;
//	u_int16 data = 0;
//	u_int16 i = 0;
//	u_int8 txcStatus = 0;
//
////	DBGWRT_1((DBH, "LL - Z247_Write: \n"));
//
//	/* Check TXC register for remaining space in the TX queue. */
//	txcStatus = MREAD_D16(llHdl->ma, Z247_TX_TXC_OFFSET);
//	DBGWRT_1((DBH, "LL - Z247_Write: txcStatus = %d\n", txcStatus));
//
//	/* Fill the complete queue. */
//	dataCount = (Z247_TX_FIFO_MAX - txcStatus);
//	DBGWRT_1((DBH, "LL - Z247_Write: writing %d bytes\n", dataCount));
//
//	/* If enough space then write the data to the queue */
//	if(dataCount != 0){
//		// Need to add support for user buffer tx.
//		for(i=0;i<dataCount;i++){
//			data = ReadFromBuffer(llHdl);
//			MWRITE_D16(llHdl->ma, Z247_FIFO_START_ADDR + (i*2),  data);
//		}
//
//		/* Acknowledge the the data before enabling the queue space interrupt. */
//		MWRITE_D16(llHdl->ma, Z247_TX_TXA_OFFSET, dataCount);
////		DBGWRT_1((DBH, "LL - Z247_Write:HWWrite Z247_TX_TXA_OFFSET = %d\n",dataCount));
//	} /* Else, do nothing. */
//
//	return result;
//}


/**********************************************************************/
/** Set TX Data rate.
 *
 *  Set transmit data rate according to the user configuration.
 *
 *  \param llHdl      \IN  low-level handle
 *  \param result     \OUT result of the operation; 0 on success and -1 on error
 *  \return           \OUT uint32 data
 */
int SetDataRate( LL_HANDLE *llHdl, u_int8 txSpeed){

	int result = 0;
	int buffSize = 0;
	int trigLevel = 0;
	int gotsize = 0;
	u_int8 regData = 0;

	switch(txSpeed){
	case Z247_TX_DATA_RATE_64:
		buffSize = 504;		/* 512 - 8 (4 sync words) */
		trigLevel = Z247_TX_TRIG_LVL_08;
		llHdl->writeBlockSize = 16;
		break;
	case Z247_TX_DATA_RATE_128:
		buffSize = 1016;	/* 1024 - 8 (4 sync words) */
		trigLevel = Z247_TX_TRIG_LVL_16;
		llHdl->writeBlockSize = 32;
		break;
	case Z247_TX_DATA_RATE_256:
		buffSize = 2040;	/* 2048 - 8 (4 sync words) */
		trigLevel = Z247_TX_TRIG_LVL_32;
		llHdl->writeBlockSize = 64;
		break;
	case Z247_TX_DATA_RATE_512:
		buffSize = 4088;	/* 4096 - 8 (4 sync words) */
		trigLevel = Z247_TX_TRIG_LVL_64;
		llHdl->writeBlockSize = 128;
		break;
	case Z247_TX_DATA_RATE_1024:
		buffSize = 8184;	/* 8192 - 8 (4 sync words) */
		trigLevel = Z247_TX_TRIG_LVL_128;
		llHdl->writeBlockSize = 256;
		break;
	case Z247_TX_DATA_RATE_2048:
		buffSize = 16376;	/* 16384 - 8 (4 sync words) */
		trigLevel = Z247_TX_TRIG_LVL_256;
		llHdl->writeBlockSize = 512;
		break;
	case Z247_TX_DATA_RATE_4096:
		buffSize = 32760;	/* 32768 - 8 (4 sync words) */
		trigLevel = Z247_TX_TRIG_LVL_256;
		llHdl->writeBlockSize = 512;
		break;
	case Z247_TX_DATA_RATE_8192:
		buffSize = 65528;	/* 65536 - 8 (4 sync words) */
		trigLevel = Z247_TX_TRIG_LVL_256;
		llHdl->writeBlockSize = 512;
		break;
	default:
		txSpeed = Z247_TX_DATA_RATE_64;
		buffSize = 504;		/* 512 - 8 (4 sync words) */
		trigLevel = Z247_TX_TRIG_LVL_08;
		llHdl->writeBlockSize = 16;
		break;
	}
	/* Delete the allocated memory. */
	if(llHdl->usrBuffer != NULL){
		OSS_MemFree(llHdl->osHdl, (int8*)llHdl->usrBuffer, llHdl->usrBufferSize *2);
		llHdl->usrBuffer = NULL;
	}
	if(llHdl->drvRingBuffer != NULL){
		OSS_MemFree(llHdl->osHdl, (int8*)llHdl->drvRingBuffer, llHdl->drvRingSize * 2);
		llHdl->drvRingBuffer = NULL;
	}
	/* Allocate memory to user buffer */
	if ((llHdl->drvRingBuffer = (u_int16*)OSS_MemGet(
			llHdl->osHdl, buffSize, &gotsize)) == NULL)
	{
		llHdl->drvRingBuffer = NULL;
		result = ERR_OSS_MEM_ALLOC;
	}else{

		/* Allocate memory to driver ring buffer */
		if ((llHdl->usrBuffer = (u_int16*)OSS_MemGet(
				llHdl->osHdl, buffSize, &gotsize)) == NULL)
		{
			llHdl->usrBuffer = NULL;
			result = ERR_OSS_MEM_ALLOC;
		}else{
			llHdl->drvRingSize = buffSize / 2;
			llHdl->usrBufferSize = buffSize / 2;

			regData = MREAD_D8(llHdl->ma, Z247_TX_LCR_OFFSET);
			DBGWRT_1((DBH, "LL - Z247_SetRate: LCR value = 0x%x\n",regData));
			regData = regData & (~Z247_TX_DATA_RATE_MASK);
			DBGWRT_1((DBH, "LL - Z247_SetRate: LCR value after rate reset = 0x%x\n",regData));
			regData |= ((txSpeed << Z247_TX_DATA_RATE_OFFSET) & Z247_TX_DATA_RATE_MASK);
			DBGWRT_1((DBH, "LL - Z247_SetRate: LCR value after rate set = 0x%x\n",regData));
			/* Set the Data Rate  */
			MWRITE_D8(llHdl->ma, Z247_TX_LCR_OFFSET, regData);

			/* Configure TX FCR trigger level*/
			MWRITE_D8(llHdl->ma, Z247_TX_FCR_OFFSET, trigLevel);

			/* Set the user data flag. */
			llHdl->isUsrDataUpdated = USER_DATA_NOT_UPDATED;
			llHdl->disableTx = 0;
			llHdl->isTxIrqExit = 0;
			llHdl->drvRingDataCnt = 0;
			llHdl->txFrameCnt = 0;
		}
	}

	return result;
}

/**********************************************************************/
/** Read data from the driver buffer.
 *
 *  Read one word from the ring buffer.
 *
 *  \param llHdl      \IN  low-level handle
 *  \param result     \OUT result of the operation; 0 on success and -1 on error
 *  \return           \OUT uint32 data
 */
u_int16 ReadFromBuffer( LL_HANDLE *llHdl){

	u_int16 data = 0;
	u_int16* tmpBufferPtr = NULL;

	/* If data count has reached to an end then reset it to the start. */
	if(llHdl->drvRingDataCnt == 0){
		/* Check whether the user data updated.  */
		if(llHdl->isUsrDataUpdated == USER_DATA_UPDATED){
			/* If updated then fill the new data to the driver buffer. */
			tmpBufferPtr = llHdl->usrBuffer;
			llHdl->usrBuffer = llHdl->drvRingBuffer;
			llHdl->drvRingBuffer = tmpBufferPtr;
			llHdl->drvRingSize = llHdl->usrBufferSize;
			llHdl->txFrameCnt = 0;
		}
		llHdl->drvRingDataCnt = llHdl->drvRingSize;
		llHdl->drvRingTail = 0;
		llHdl->isUsrDataUpdated = USER_DATA_NOT_UPDATED;
		llHdl->txFrameCnt++;

	}

	data = llHdl->drvRingBuffer[llHdl->drvRingTail];
	DBGWRT_3((DBH, "LL - Z247_Write: Tx Data[%d] = 0x%x\n",llHdl->drvRingTail,  data));
	llHdl->drvRingTail = (unsigned int)(llHdl->drvRingTail + 1) % llHdl->drvRingSize;
	llHdl->drvRingDataCnt--;

	return data;
}


/**********************************************************************/
/** Print register configuration.
 *
 *  Print the status of the registers.
 *
 *  \param llHdl      \IN low-level handle
 */


void RegStatus(LL_HANDLE *llHdl ){
	DBGWRT_2((DBH, " \n"));
	DBGWRT_2((DBH, " >>  LL - Z247_drv status: IIR stat = 0x%x\n", MREAD_D8(llHdl->ma, Z247_IIR_STAT)));
	DBGWRT_2((DBH, " >>  LL - Z247_drv status: lsr = 0x%x\n", MREAD_D8(llHdl->ma, Z247_TX_LSR_OFFSET)));
	DBGWRT_2((DBH, " >>  LL - Z247_drv status: txa = 0x%x\n", MREAD_D16(llHdl->ma, Z247_TX_TXA_OFFSET)));
	DBGWRT_2((DBH, " >>  LL - Z247_drv status: txc = 0x%x\n", MREAD_D16(llHdl->ma, Z247_TX_TXC_OFFSET)));
	DBGWRT_2((DBH, " >>  LL - Z247_drv status: IER = 0x%x\n", MREAD_D8(llHdl->ma, Z247_TX_IER_OFFSET)));
	DBGWRT_2((DBH, " >>  LL - Z247_drv status: LCR = 0x%x\n", MREAD_D8(llHdl->ma, Z247_TX_LCR_OFFSET)));
	DBGWRT_2((DBH, " >>  LL - Z247_drv status: FCR = 0x%x\n", MREAD_D8(llHdl->ma, Z247_TX_FCR_OFFSET)));
	DBGWRT_2((DBH, " >>  LL - Z247_drv status: RST = 0x%x\n", MREAD_D8(llHdl->ma, Z247_TX_RST_OFFSET)));
	DBGWRT_2((DBH, " \n"));
}
