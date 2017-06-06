/*********************  P r o g r a m  -  M o d u l e ***********************/
/*!
 *        \file  z147_drv.c
 *
 *      \author  APatil
 *        $Date: 2009/08/03 16:43:18 $
 *    $Revision: 1.6 $
 *
 *      \brief   Low-level driver for ARINC717 receiver
 *
 *     Required: OSS, DESC, DBG libraries
 *
 *     \switches _ONE_NAMESPACE_PER_DRIVER_
 */
/*-------------------------------[ History ]--------------------------------
 *
 * $Log: z147_drv.c,v $
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

#define ADDRSPACE_COUNT    			1          /**< nbr of required address spaces */
#define ADDRSPACE_SIZE     			0x1000         /**< size of address space          */
#define CH_NUMBER          			1          /**< number of device channels      */

/* debug defines */
#define DBG_MYLEVEL        			llHdl->dbgLevel    /**< debug level  */
#define DBH                			llHdl->dbgHdl      /**< debug handle */
#define OSH                			llHdl->osHdl       /**< OS handle    */

#define Z147_STAT_REG				0x800		/**< Offset of the status register. */
#define Z147_RX_IRQ_MASK    		0x03		/**< Receive IRQ mask. */
#define Z147_LSR_REG_OFFSET			0x801		/**< Offset of the LSR status register. */
#define Z147_LSR_OFFSET				0x001		/**< Offset of the LSR status register. */
#define Z147_LSR_OE_MASK			0x02		/**< Overflow error mask. */
#define Z147_LSR_SE_MASK			0x04		/**< Reset value of the LSR status register. */
#define Z147_LSR_LSE_MASK			0x08		/**< Reset value of the LSR status register. */
#define Z147_LSR_RESET_VAL 			0x0E		/**< Reset value of the LSR status register. */
#define Z147_LSR_INSYNC_MASK		0x10
#define Z147_LSR_RXSUB_MASK			0x60
#define Z147_LSR_RXSUB_OFFSET		0x05

#define Z147_RX_RXC_REG_OFFSET		0x802		/**< Status register for received word count. */
#define Z147_RX_RXC_OFFSET			0x002		/**< Status register for received word count. */
#define Z147_RX_RXA_OFFSET			0x804		/**< Status register for acknowledge word count. */
#define Z147_RX_SUB_PTR_OFFSET		0x806		/**< Status register sub frame pointer. */

#define Z147_RX_IER_OFFSET			0x808		/**< Offset of the RX interrupt register */
#define Z147_RX_IER_DEFAULT			0x3		    /**< Default value of the RX interrupt register */

#define Z147_RX_LCR_OFFSET			0x809		/**< Offset of the RX_LCR register */
#define Z147_RX_LCR_DEFAULT			0x02		/**< Default value of the RX_LCR register */
#define Z147_RX_SYNC_OFFSET			   2		/**< Offset of the RX_LCR sync mode */
#define Z147_RX_SYNC_MASK  			0x03		/**< MASK of the RX_LCR sync mode */
#define Z147_RX_DATA_RATE_OFFSET	2			/**< Offset of the RX_LCR data rate */
#define Z147_RX_DATA_RATE_MASK  	0x1C		/**< MASK of the RX_LCR Data Rate */
#define Z147_RX_MODE_OFFSET			4			/**< Offset of the RX_LCR RX mode */
#define Z147_RX_MODE_MASK  			0x20		/**< MASK of the RX_LCR RX mode */

#define Z147_RX_FCR_OFFSET			0x80A		/**< Offset of the RX_FCR register */
#define Z147_RX_FCR_DEFAULT			0x07		/**< Default value of the RX_FCR register */
#define Z147_RX_THR_LEV_MASK		0x07		/**< Offset of the register RX_LCR RX threshold level mask */

#define Z147_RX_RST_OFFSET			0x80B
#define Z147_RX_RST_MASK 			0x01

#define Z147_RX_RLSIEN_OFFSET		0x0  		/**< Offset of the receive line status interrupt enable bit. */
#define Z147_RX_RLSIEN_MASK  		0x1  		/**< Offset of the receive line status interrupt enable bit. */

#define Z147_RX_RDAIEN_OFFSET		0x1  		/**< Offset of the receive line status interrupt enable bit. */
#define Z147_RX_RDAIEN_MASK  		0x2  		/**< Offset of the receive line status interrupt enable bit. */

#define Z147_RX_FIFO_START_ADDR		0x0000		/**< Receive FIFO start address. */
#define Z147_RX_FIFO_LEN			0x07FF		/**< Receive FIFO size. */

#define Z147_RX_LINE_STAT_IRQ		1			/**< Receive line status interrupt offset */
#define Z147_RX_DATA_AVAIL_IRQ		2			/**< Receive data available interrupt offset */


#define Z147_RX_DATA_RATE_64        0    /**< Set data rate of 64 words/sec. */
#define Z147_RX_DATA_RATE_128       1    /**< Set data rate of 128 words/sec. */
#define Z147_RX_DATA_RATE_256       2    /**< Set data rate of 256 words/sec. */
#define Z147_RX_DATA_RATE_512       3    /**< Set data rate of 512 words/sec. */
#define Z147_RX_DATA_RATE_1024      4    /**< Set data rate of 1024 words/sec. */
#define Z147_RX_DATA_RATE_2048      5    /**< Set data rate of 2048 words/sec. */
#define Z147_RX_DATA_RATE_4096      6    /**< Set data rate of 4096 words/sec. */
#define Z147_RX_DATA_RATE_8192      7    /**< Set data rate of 8192 words/sec. */

#define Z147_RX_TRIG_LVL_08   	    1    /**< Set trigger level to 8 words. */
#define Z147_RX_TRIG_LVL_16   	    2    /**< Set trigger level to 16 words. */
#define Z147_RX_TRIG_LVL_32         3    /**< Set trigger level to 32 words. */
#define Z147_RX_TRIG_LVL_64   	    4    /**< Set trigger level to 64 words. */
#define Z147_RX_TRIG_LVL_128  	    5    /**< Set trigger level to 128 words. */
#define Z147_RX_TRIG_LVL_256   	    6    /**< Set trigger level to 256 words. */
#define Z147_RX_TRIG_LVL_512        7    /**< Set trigger level to 512 words. */

#define USER_DATA_NOT_UPDATED 		0	 /**< User buffer is not updated. */
#define USER_DATA_UPDATED  			1	 /**< User buffer is updated.

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

	OSS_SIG_HANDLE          *rxDataSig; 	/**< signal for port change */
	OSS_SIG_HANDLE          *rxErrorSig; 	/**< signal for port change */

	/* toggle mode */
	OSS_ALARM_HANDLE        *alarmHdl;      /**< alarm handle               */
	OSS_SEM_HANDLE          *devSemHdl;     /**< device semaphore handle    */

	/* Buffer for user frame. */
	u_int16*				usrBuffer;		 /**< Buffer for user data. */
	volatile u_int32 		usrBuffSize;	 /**< User buffer size. */
	u_int8					isUsrDataUpdated;/**< Flag to indicate whether the user has updated data. */

	/* Ring buffer for driver transmission */
	u_int16*				drvRingBuffer;   /**< Buffer for driver data. */
	volatile u_int32	    drvRingHead;	 /**< Head position of the ring buffer. */
	volatile u_int32 		drvRingTail;	 /**< Tail position of the ring buffer. */
	volatile u_int32 		drvRingSize;     /**< Size of the ring buffer. */
	u_int32 				drvRingSyncPos;  /**< Sync position if the ring buffer. */

	u_int16					subFrameSize;	 /**< Sub frame size according to the configuration.*/

	u_int8					isDrvSync;		/**< Flag to indicate whether the driver is in sync with FPGA. */
	u_int8					disableRx;		/**< Flag to indicate whether the driver is disabled by user. */
	u_int8					isRxIrqExit;	/**< Flag to indicate whether the driver exited from IRQ routine. */
	u_int64					rxIrqCnt;		/**< Receive interrupt count. */
	u_int64					rxOverrunErrCnt;	/**< Receive overrun error count. */
	u_int64					rxStreamIntErrCnt;  /**< Receive stream interrupt error count. */
	u_int64					rxLostSyncErrCnt;	/**< Receive lost sync error count. */
} LL_HANDLE;


/* include files which need LL_HANDLE */
#include <MEN/ll_entry.h>       /* low-level driver jump table */
#include <MEN/z147_drv.h>        /* Z147 driver header file      */

/*-----------------------------------------+
|  PROTOTYPES                              |
+-----------------------------------------*/
static int32 Z147_Init(DESC_SPEC *descSpec, OSS_HANDLE *osHdl,
		MACCESS *ma, OSS_SEM_HANDLE *devSemHdl,
		OSS_IRQ_HANDLE *irqHdl, LL_HANDLE **llHdlP);
static int32 Z147_Exit(LL_HANDLE **llHdlP);
static int32 Z147_Read(LL_HANDLE *llHdl, int32 ch, int32 *value);
static int32 Z147_Write(LL_HANDLE *llHdl, int32 ch, int32 value);
static int32 Z147_SetStat(LL_HANDLE *llHdl,int32 ch, int32 code,
		INT32_OR_64 value32_or_64);
static int32 Z147_GetStat(LL_HANDLE *llHdl, int32 ch, int32 code,
		INT32_OR_64 *value32_or64P);
static int32 Z147_BlockRead(LL_HANDLE *llHdl, int32 ch, void *buf, int32 size,
		int32 *nbrRdBytesP);
static int32 Z147_BlockWrite(LL_HANDLE *llHdl, int32 ch, void *buf, int32 size,
		int32 *nbrWrBytesP);
static int32 Z147_Irq(LL_HANDLE *llHdl);

static int32 Z147_Info(int32 infoType, ...);
static char* Ident(void);
static int32 Cleanup(LL_HANDLE *llHdl, int32 retCode);
static void  ConfigureDefault( LL_HANDLE *llHdl );
static int SetDataRate(LL_HANDLE *llHdl, u_int8 rxSpeed);
static void RegStatus(LL_HANDLE *llHdl);

/****************************** Z147_GetEntry ********************************/
/** Initialize driver's jump table
 *
 *  \param drvP     \OUT pointer to the initialized jump table structure
 */
#ifdef _ONE_NAMESPACE_PER_DRIVER_
extern void LL_GetEntry(
		LL_ENTRY* drvP
)
#else
extern void __Z147_GetEntry(
		LL_ENTRY* drvP
)
#endif
{
	drvP->init        = Z147_Init;
	drvP->exit        = Z147_Exit;
	drvP->read        = Z147_Read;
	drvP->write       = Z147_Write;
	drvP->blockRead   = Z147_BlockRead;
	drvP->blockWrite  = Z147_BlockWrite;
	drvP->setStat     = Z147_SetStat;
	drvP->getStat     = Z147_GetStat;
	drvP->irq         = Z147_Irq;
	drvP->info        = Z147_Info;
}

/******************************** Z147_Init **********************************/
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
static int32 Z147_Init(
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
	/* Receive buffer */
	llHdl->drvRingHead    = 0;
	llHdl->drvRingTail    = 0;
	llHdl->drvRingSyncPos = 0;
	llHdl->disableRx	  = 0;
	llHdl->drvRingBuffer  = NULL;
	llHdl->usrBuffer      = NULL;
	llHdl->rxIrqCnt       = 0;
	llHdl->rxOverrunErrCnt = 0;
	llHdl->rxStreamIntErrCnt  = 0;
	llHdl->rxLostSyncErrCnt = 0;
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

	/*------------------------------+
	|  init hardware                |
	+------------------------------*/
	ConfigureDefault(llHdl);

	/* Set the user data flag. */
	llHdl->isUsrDataUpdated = USER_DATA_NOT_UPDATED;

	*llHdlP = llHdl;		/* set low-level driver handle */

	return (ERR_SUCCESS);
}

/****************************** Z147_Exit ************************************/
/** De-initialize hardware and clean up memory
 *
 *  The function deinitializes all channels by setting them as inputs.
 *  The interrupt is disabled.
 *
 *  \param llHdlP     \IN  pointer to low-level driver handle
 *
 *  \return           \c 0 on success or error code
 */
static int32 Z147_Exit(
		LL_HANDLE **llHdlP
)
{

	LL_HANDLE *llHdl = *llHdlP;
	int32 error = 0;
	int timeoutCnt = 20;
	while(llHdl->isRxIrqExit != 0){
		if(timeoutCnt > 0){
			/* Else wait for some time and check again */
			timeoutCnt--;
			OSS_Delay(OSH, 100);
		}else{
			/* Exit the loop and return. */
			return ERR_LL_DEV_BUSY;
		}
	}
	/* reset the default interrupts */
	MWRITE_D8(llHdl->ma, Z147_RX_IER_OFFSET, 0);
	IDBGWRT_2((DBH, ">>> LL - Z147_Exit: Register status in the Exit\n"));
	RegStatus(llHdl);
	DBGWRT_1((DBH, "Z147_Exit\n"));

	/*------------------------------+
	|  clean up memory              |
	+------------------------------*/
	*llHdlP = NULL;		/* set low-level driver handle to NULL */
	error = Cleanup(llHdl, error);
	RegStatus(llHdl);
	/*------------------------------+
	|  free memory                  |
	+------------------------------*/
	/* free my handle */
	OSS_MemFree(OSH, (int8*)llHdl, llHdl->memAlloc);

	return (error);
}

/****************************** Z147_Read ************************************/
/** Read a value from the device
 *
 *  The function is not supported
 *
 *  \param llHdl      \IN  low-level handle
 *  \param ch         \IN  current channel
 *  \param valueP     \OUT read value
 *
 *  \return           \c 0 on success or error code
 */
static int32 Z147_Read(
		LL_HANDLE *llHdl,
		int32 ch,
		int32 *valueP
)
{
	DBGWRT_1((DBH, "LL - Z147_Read: ch=%d, valueP=%d\n",ch,*valueP));

	return( ERR_LL_ILL_FUNC );
}

/****************************** Z147_Write ***********************************/
/** Description:  Write a value to the device
 *
 *  The function is not supported
 *
 *  \param llHdl      \IN  low-level handle
 *  \param ch         \IN  current channel
 *  \param value      \IN  value to write
 *
 *  \return           \c 0 on success or error code
 */
static int32 Z147_Write(
		LL_HANDLE *llHdl,
		int32 ch,
		int32 value
)
{
	DBGWRT_1((DBH, "LL USE_IRQ- Z147_Write: ch=%d, size=%d\n",ch,value));

	return( ERR_LL_ILL_FUNC );
}

/****************************** Z147_SetStat *********************************/
/** Set the driver status
 *
 *  The driver supports \ref rx_getstat_setstat_codes "these status codes"
 *  in addition to the standard codes (see mdis_api.h).
 *  Note: only inputs are able fire an interrupt
 *
 *  \param llHdl         \IN  low-level handle
 *  \param code          \IN  \ref rx_getstat_setstat_codes "status code"
 *  \param ch            \IN  current channel
 *  \param value32_or_64 \IN  data or pointer to block data structure
 *                            (M_SG_BLOCK) for block status codes
 *  \return              \c 0 on success or error code
 */
static int32 Z147_SetStat(
		LL_HANDLE   *llHdl,
		int32       code,
		int32       ch,
		INT32_OR_64 value32_or_64
)
{
	int32 value = (int32)value32_or_64;		/* 32bit value */

	int32 error = ERR_SUCCESS;
	u_int8 regData = 0;

	DBGWRT_1((DBH, "LL - Z147_SetStat: ch=%d code=0x%04x value=0x%x\n",
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
	case Z147_SET_SIGNAL:
		/* signal already installed ? */
		if (llHdl->rxDataSig) {
			error = ERR_OSS_SIG_SET;
			break;
		}
		error = OSS_SigCreate(OSH, value, &llHdl->rxDataSig);
		break;
		/*--------------------------+
		|  unregister signal        |
		+--------------------------*/
	case Z147_CLR_SIGNAL:
		/* signal already installed ? */
		if (llHdl->rxDataSig == NULL) {
			error = ERR_OSS_SIG_CLR;
			break;
		}
		error = OSS_SigRemove(OSH, &llHdl->rxDataSig);
		break;

		/*--------------------------+
		|  register error signal    |
		+--------------------------*/
	case Z147_SET_ERR_SIGNAL:
		/* signal already installed ? */
		if (llHdl->rxErrorSig) {
			error = ERR_OSS_SIG_SET;
			break;
		}
		error = OSS_SigCreate(OSH, value, &llHdl->rxErrorSig);
		break;
		/*--------------------------+
		|  unregister error signal  |
		+--------------------------*/
	case Z147_CLR_ERR_SIGNAL:
		/* signal already installed ? */
		if (llHdl->rxErrorSig == NULL) {
			error = ERR_OSS_SIG_CLR;
			break;
		}
		error = OSS_SigRemove(OSH, &llHdl->rxErrorSig);
		break;

		/*--------------------------------------------+
		|  Receive line status interrupt status       |
		+---------------------------------------------*/
	case Z147_RX_ERR_IRQ_STAT:
		/* Set IRQ mode */
		regData = MREAD_D8(llHdl->ma, Z147_RX_IER_OFFSET);
		if(value32_or_64 != 0){
			regData = regData | Z147_RX_RLSIEN_MASK;
		}else{
			regData = regData & (~Z147_RX_RLSIEN_MASK);
		}
		MWRITE_D8(llHdl->ma, Z147_RX_IER_OFFSET, regData);
		break;

		/*-------------------------+
		|  Receive Rate            |
		+-------------------------*/
	case Z147_RX_DATA_RATE:
		if((value >= 0) && (value <= Z147_RX_DATA_RATE_MASK)){
			MWRITE_D8(llHdl->ma, Z147_RX_RST_OFFSET, 1);
			/* Disable the interrupt. */
			MWRITE_D8(llHdl->ma, Z147_RX_IER_OFFSET, 0);
			llHdl->disableRx = 1;
			error = SetDataRate(llHdl, (u_int8)value);
			/* Disable the interrupt. */
			MWRITE_D8(llHdl->ma, Z147_RX_IER_OFFSET, Z147_RX_IER_DEFAULT);
			MWRITE_D8(llHdl->ma, Z147_RX_RST_OFFSET, 0);

		}else{
			error = ERR_LL_ILL_PARAM;
		}
		break;

		/*--------------------------------------+
		|  Receive threshold level status    |
		+---------------------------------------*/
	case Z147_RX_THR_LEV:
		regData = MREAD_D8(llHdl->ma, Z147_RX_FCR_OFFSET);
		regData = regData & (~Z147_RX_THR_LEV_MASK);
		regData |= (value32_or_64 & Z147_RX_THR_LEV_MASK);
		MWRITE_D8(llHdl->ma, Z147_RX_FCR_OFFSET, regData);
		break;

     	/*----------------------+
		|  Disable reception    |
		+----------------------*/
	case Z147_DISABLE_RX:
		llHdl->disableRx = 1;
		break;

		/*-----------------+
		|  Receive SYNC    |
		+-----------------*/
	case Z147_RX_SYNC_CFG:
		if((value >= 0) && (value < Z147_RX_SYNC_MASK)){
			MWRITE_D8(llHdl->ma, Z147_RX_RST_OFFSET, 1);
			/* Disable the interrupt. */
			MWRITE_D8(llHdl->ma, Z147_RX_IER_OFFSET, 0);
			llHdl->disableRx = 1;
			regData = MREAD_D8(llHdl->ma, Z147_RX_LCR_OFFSET);

			regData = regData & (~Z147_RX_DATA_RATE_MASK);
			regData |= (value  & Z147_RX_DATA_RATE_MASK);
			/* Set the sync mode.  */
			MWRITE_D8(llHdl->ma, Z147_RX_LCR_OFFSET, regData);
			/* Enable the interrupt. */
			MWRITE_D8(llHdl->ma, Z147_RX_IER_OFFSET, Z147_RX_IER_DEFAULT);
			MWRITE_D8(llHdl->ma, Z147_RX_RST_OFFSET, 0);

		}else{
			error = ERR_LL_ILL_PARAM;
		}
		break;

		/*-----------------+
		|  Receive Mode    |
		+-----------------*/
	case Z147_RX_MODE_CFG:

			MWRITE_D8(llHdl->ma, Z147_RX_RST_OFFSET, 1);
			/* Disable the interrupt. */
			MWRITE_D8(llHdl->ma, Z147_RX_IER_OFFSET, 0);
			llHdl->disableRx = 1;
			/* Set RX mode */
			regData = MREAD_D8(llHdl->ma, Z147_RX_LCR_OFFSET);
			if(value32_or_64 != 0){
				regData = regData | Z147_RX_MODE_MASK;
			}else{
				regData = regData & (~Z147_RX_MODE_MASK);
			}
			/* Set the RX mode.  */
			MWRITE_D8(llHdl->ma, Z147_RX_LCR_OFFSET, regData);
			/* Enable the interrupt. */
			MWRITE_D8(llHdl->ma, Z147_RX_IER_OFFSET, Z147_RX_IER_DEFAULT);
			MWRITE_D8(llHdl->ma, Z147_RX_RST_OFFSET, 0);

		break;

		/*--------------------------+
		|  (unknown)                |
		+--------------------------*/
	default:
		error = ERR_LL_UNK_CODE;
	}

	return (error);
}

/****************************** Z147_GetStat *********************************/
/** Get the driver status
 *
 *  The driver supports \ref rx_getstat_setstat_codes "these status codes"
 *  in addition to the standard codes (see mdis_api.h).
 *  Getstat Z147_IRQ_LAST_REQUEST will return the last IRQ request mask
 *  and clear it than.
 *  \param llHdl             \IN  low-level handle
 *  \param code              \IN  \ref rx_getstat_setstat_codes "status code"
 *  \param ch                \IN  current channel
 *  \param value32_or_64P    \IN  pointer to block data structure (M_SG_BLOCK) for
 *                                block status codes
 *  \param value32_or_64P    \OUT data pointer or pointer to block data structure
 *                                (M_SG_BLOCK) for block status codes
 *
 *  \return                  \c 0 on success or error code
 */
static int32 Z147_GetStat(
		LL_HANDLE   *llHdl,
		int32       code,
		int32       ch,
		INT32_OR_64 *value32_or_64P
)
{
	int32 *valueP = (int32*)value32_or_64P;		/* pointer to 32bit value */
	INT32_OR_64 *value64P = value32_or_64P;		/* stores 32/64bit pointer */
	int32 error = ERR_SUCCESS;
	u_int8 regData = 0;
	u_int8 lsrStatus = 0;

	DBGWRT_2((DBH, "LL - Z147_GetStat: ch=%d code=0x%04x\n", ch, code));

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
		|  RX data length           |
		+--------------------------*/
	case Z147_RX_DATA_LEN:
		*valueP = (INT32_OR_64)llHdl->usrBuffSize * 2;
		break;

		/*--------------------------------------------+
		|  Receive line status interrupt status       |
		+---------------------------------------------*/
	case Z147_RX_ERR_IRQ_STAT:
		regData = MREAD_D8(llHdl->ma, Z147_RX_IER_OFFSET);
		*valueP = (INT32_OR_64)((regData & Z147_RX_RLSIEN_MASK) >> Z147_RX_RLSIEN_OFFSET );
		break;

		/*-----------------+
		|  Receive rate    |
		+-----------------*/
	case Z147_RX_DATA_RATE:
		regData = MREAD_D8(llHdl->ma, Z147_RX_LCR_OFFSET);
		*valueP = (INT32_OR_64)((regData & Z147_RX_DATA_RATE_MASK) >> Z147_RX_DATA_RATE_OFFSET);
		break;

		/*--------------------------------------+
		|  Receive threshold level status    |
		+---------------------------------------*/
	case Z147_RX_THR_LEV:
		regData = MREAD_D8(llHdl->ma, Z147_RX_FCR_OFFSET);
		*valueP = (INT32_OR_64)((regData & Z147_RX_THR_LEV_MASK));
		break;

		/*------------+
		|  IS SYNC    |
		+------------*/
	case Z147_RX_IN_SYNC:
		lsrStatus = MREAD_D8(llHdl->ma, Z147_LSR_REG_OFFSET);

		if((llHdl->isDrvSync != 0) &&
		   (llHdl->isUsrDataUpdated == USER_DATA_UPDATED) &&
		   ((lsrStatus & Z147_LSR_INSYNC_MASK) == Z147_LSR_INSYNC_MASK))
		{
			*valueP = 1;
			DBGWRT_3((DBH, "LL - Z147_GetStat: Z147_RX_IN_SYNC = in sync\n"));
		}else{
			*valueP = 0;
			DBGWRT_1((DBH, "LL - Z147_GetStat: Z147_RX_IN_SYNC = not in sync\n"));
			DBGWRT_1((DBH, "LL - Z147_GetStat: Z147_RX_IN_SYNC = is DrvSync = %d lsrStatus = 0x%x\n",llHdl->isDrvSync, lsrStatus));
		}
		break;

		/*-----------------+
		|  Receive SYNC    |
		+-----------------*/
	case Z147_RX_SYNC_CFG:
		regData = MREAD_D8(llHdl->ma, Z147_RX_LCR_OFFSET);
		*valueP = (INT32_OR_64)(regData & Z147_RX_SYNC_MASK);
		break;

		/*-----------------+
		|  Receive mode    |
		+-----------------*/
	case Z147_RX_MODE_CFG:
		regData = MREAD_D8(llHdl->ma, Z147_RX_LCR_OFFSET);
		*valueP = (INT32_OR_64)((regData & Z147_RX_MODE_MASK) >> Z147_RX_MODE_OFFSET);
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

/******************************* Z147_BlockRead ******************************/
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
static int32 Z147_BlockRead(
		LL_HANDLE *llHdl,
		int32     ch,
		void      *buf,
		int32     size,
		int32     *nbrRdBytesP
)
{
	int32 result = 0;
	u_int32 dataLenByte = llHdl->usrBuffSize * 2;

	DBGWRT_1((DBH, ">>> LL - Z147_BlockRead: ch=%d, size=%d\n",ch,size));

	if((nbrRdBytesP != NULL) && (buf != NULL)){

		/* Check whether the driver is in sync. */
		if(llHdl->isDrvSync != 0 ){
			/* Check user buffer length */
			if((size >= (int32)dataLenByte) && (size >= (int32)(llHdl->subFrameSize * 8))){

				OSS_MemCopy(OSH, dataLenByte, (char*)llHdl->usrBuffer, (char*)buf);

				*nbrRdBytesP = dataLenByte;
				IDBGWRT_1((DBH, ">>> LL - Z147_BlockRead: Data length byte = %d\n", dataLenByte));

			}else{
				IDBGWRT_1((DBH, ">>> LL - Z147_BlockRead: User buffer is not sufficient user size = %d and needed driver size = %d.\n",size,  dataLenByte));
				result = ERR_MBUF_USERBUF;
				/* return number of read bytes */
				*nbrRdBytesP = 0;
			}
		}else{
			IDBGWRT_1((DBH, ">>> LL - Z147_BlockRead: driver is not in sync \n"));
			/* return number of read bytes */
			*nbrRdBytesP = 0;
			result = ERR_LL_DEV_NOTRDY;
		}

	}else{
		IDBGWRT_1((DBH, ">>> LL - Z147_BlockRead: Either nbrRdBytesP pointer is NUL or buffer pointer is NULL.\n "));
		result = ERR_MBUF_ILL_SIZE;
		/* return number of read bytes */
		*nbrRdBytesP = 0;
	}
	IDBGWRT_2((DBH, ">>> LL - Z147_BlockRead: Register status at the end of BlockRead\n"));
	RegStatus(llHdl);

	return result;
}

/****************************** Z147_BlockWrite *****************************/
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
static int32 Z147_BlockWrite(
		LL_HANDLE *llHdl,
		int32     ch,
		void      *buf,
		int32     size,
		int32     *nbrWrBytesP
)
{
	DBGWRT_1((DBH, ">>> LL - LL - Z147_BlockWrite: ch=%d, size=%d\n",ch,size));

	/* return number of written bytes */
	*nbrWrBytesP = 0;

	return( ERR_LL_ILL_FUNC );
}

/****************************** Z147_Irq ************************************/
/** Interrupt service routine
 *
 *  Interrupt is triggered when data is received by FPGA. If there is an error
 *  during reception then receive line status interrupt is triggered.
 *  If the driver can detect the interrupt's cause it returns
 *  LL_IRQ_DEVICE or LL_IRQ_DEV_NOT, otherwise LL_IRQ_UNKNOWN.
 *
 *  \param llHdl       \IN  low-level handle
 *  \return LL_IRQ_DEVICE   irq caused by device
 *          LL_IRQ_DEV_NOT  irq not caused by device
 */
static int32 Z147_Irq(LL_HANDLE *llHdl){

	int32 result = LL_IRQ_DEV_NOT;
	u_int32 statReg = 0;
	u_int8 lsrStatus = 0;
	u_int32 dataLen = 0;
	u_int16 i = 0;
	u_int8 subFrameNum = 0;
	u_int16 subFramePtr = 0;
	u_int16* tmpBuffPtr = NULL;

	statReg = MREAD_D32(llHdl->ma, Z147_STAT_REG);
	lsrStatus = (statReg >> (Z147_LSR_OFFSET * 8)) & 0xFF;

	/* Check whether the interrupt caused by RX ? and Process the interrupts only in SYNC. */
	if(((statReg & Z147_RX_LINE_STAT_IRQ) || (statReg & Z147_RX_DATA_AVAIL_IRQ))
			&& (lsrStatus & Z147_LSR_INSYNC_MASK) == Z147_LSR_INSYNC_MASK)
	{

		/* disable all IRQs */
		MWRITE_D8(llHdl->ma, Z147_RX_IER_OFFSET, 0);

		/* Call routine according to the interrupt. */
		if((statReg & Z147_RX_LINE_STAT_IRQ) != 0){
			/* An error is detected so discard all the received data. */

			/* Read the RXC to get the received data length. */
			dataLen = ((statReg >> (Z147_RX_RXC_OFFSET * 8)) &  0xFFFF);

			IDBGWRT_1((DBH, ">>> LL - Z147_Irq with RX line error: dataLen %08x\n", dataLen));

			/* Acknowledge the received data, which will lead to discard. */
			MWRITE_D16(llHdl->ma, Z147_RX_RXA_OFFSET, dataLen);
			if((lsrStatus & Z147_LSR_OE_MASK) == Z147_LSR_OE_MASK){
				llHdl->rxOverrunErrCnt++;
				DBGWRT_1((DBH, ">>> Z147_IRQ: Overrun error count = %llu\n",llHdl->rxOverrunErrCnt));
			}
			if((lsrStatus & Z147_LSR_SE_MASK) == Z147_LSR_SE_MASK){
				llHdl->rxStreamIntErrCnt++;
				DBGWRT_1((DBH, ">>> Z147_IRQ: Stream interruption error count = %llu\n",llHdl->rxStreamIntErrCnt));
			}
			if((lsrStatus & Z147_LSR_LSE_MASK) == Z147_LSR_LSE_MASK){
				llHdl->rxLostSyncErrCnt++;
				DBGWRT_1((DBH, ">>> Z147_IRQ: Lost synchronization error count = %llu\n",llHdl->rxLostSyncErrCnt));
			}
			/* Clear the errors */
			MWRITE_D8(llHdl->ma, Z147_LSR_REG_OFFSET, Z147_LSR_RESET_VAL);

			IDBGWRT_2((DBH, ">>> LL - Z147_Irq: Register Status after error interrupt:\n"));
			RegStatus(llHdl);
			/* if requested send signal to application */
			if (llHdl->rxErrorSig){
				OSS_SigSend(OSH, llHdl->rxErrorSig);
			}
			result = LL_IRQ_DEVICE ;
		}else if (statReg & Z147_RX_DATA_AVAIL_IRQ){
			llHdl->rxIrqCnt++;
			DBGWRT_1((DBH, ">>> Z147_IRQ: Interrupt count = %llu\n",llHdl->rxIrqCnt));
			IDBGWRT_3((DBH, ">>> LL - Z147_Irq: status register = %08x\n", statReg));
			IDBGWRT_3((DBH, ">>> LL - Z147_Irq: SUB_PTR status = %d\n", MREAD_D16(llHdl->ma, Z147_RX_SUB_PTR_OFFSET)));
			IDBGWRT_3((DBH, ">>> LL - Z147_Irq: LSR status = %08x\n", lsrStatus));
			if(llHdl->isDrvSync == 0){
				/* Calculate the frame position. */
				subFrameNum = ((lsrStatus & Z147_LSR_RXSUB_MASK) >> Z147_LSR_RXSUB_OFFSET);
				subFramePtr = MREAD_D16(llHdl->ma, Z147_RX_SUB_PTR_OFFSET);

				/* Sub frame number is considered n-1. */
				llHdl->drvRingSyncPos = ((subFrameNum  * llHdl->subFrameSize) + subFramePtr);
				llHdl->drvRingHead = llHdl->drvRingSyncPos;

				llHdl->isDrvSync = 1;

				IDBGWRT_2((DBH, ">>> LL - Z147_Irq subFrameNum: %d\n", subFrameNum));
				IDBGWRT_2((DBH, ">>> LL - Z147_Irq subFramePtr: %d\n", subFramePtr));
				IDBGWRT_2((DBH, ">>> LL - Z147_Irq buffLocation: %d\n", llHdl->drvRingSyncPos));
			}
			/* Read the RXC to get the received data length. */
			dataLen = ((statReg >> (Z147_RX_RXC_OFFSET * 8)) &  0xFFFF);
			//IDBGWRT_1((DBH, ">>> LL - Z147_Irq: Data length = %d\n", dataLen));

			/* Store data in the driver buffer. */
			for(i=0; i<dataLen; i++){
				/* Add word to the ring buffer */
				llHdl->drvRingBuffer[llHdl->drvRingHead] = MREAD_D16(llHdl->ma, (Z147_RX_FIFO_START_ADDR + (i * 2)));
				IDBGWRT_3((DBH, ">>> LL - Z147_Irq: Rx Data word-%d = 0x%x\n", llHdl->drvRingHead, llHdl->drvRingBuffer[llHdl->drvRingHead]));

				/* Update the head pointer. */
				llHdl->drvRingHead = (llHdl->drvRingHead + 1) % llHdl->drvRingSize;

				/* If the sync position is reached then copy the data to the user buffer. */
				if(llHdl->drvRingHead == llHdl->drvRingSyncPos){

					/* Set the indication of the new data. */
					llHdl->isUsrDataUpdated = USER_DATA_NOT_UPDATED;
					tmpBuffPtr = llHdl->drvRingBuffer;
					llHdl->drvRingBuffer = llHdl->usrBuffer;
					llHdl->usrBuffer = tmpBuffPtr;
					/* Set the indication of the new data. */
					llHdl->isUsrDataUpdated = USER_DATA_UPDATED;

					/* FIFO is empty now send signal to the application. */
					/* if requested send signal to application */
					if (llHdl->rxDataSig){
						OSS_SigSend(OSH, llHdl->rxDataSig);
					}
				}

				/* Check whether the buffer is full. */
				if (llHdl->drvRingHead == llHdl->drvRingSize)	{
					/* Start from the beginning. */
					llHdl->drvRingHead = 0;
				}
			}
			/* Acknowledge the data. */
			MWRITE_D16(llHdl->ma, Z147_RX_RXA_OFFSET, dataLen);

			result = LL_IRQ_DEVICE ;

		}
		if(llHdl->disableRx == 0){
			/* Enable the default interrupts */
			MWRITE_D8(llHdl->ma, Z147_RX_IER_OFFSET, Z147_RX_IER_DEFAULT);
		}else{
			/* Set the flag to indicate the the IRQ is exited safely. */
			llHdl->isRxIrqExit = 1;
		}

	}/* Else don't care about the interrupts and data */

	return result;
}

/****************************** Z147_Info ***********************************/
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
 *  The LL_INFO_ADDRSPACE code returns information about one
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
static int32 Z147_Info(
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
	return ("Z147 - Z147 low level driver: $Id: z147_drv.c,v 1.11 2015/09/23 14:17:37 APatil");
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
		DESC_Exit(&llHdl->descHdl);

	/* clean up debug */
	DBGEXIT((&DBH));

	/*------------------------------+
	|  free memory                  |
	+------------------------------*/
	if(llHdl->usrBuffer != NULL){
		OSS_MemFree(llHdl->osHdl, (int8*)llHdl->usrBuffer, llHdl->usrBuffSize *2);
	}
	if(llHdl->usrBuffer != NULL){
		OSS_MemFree(llHdl->osHdl, (int8*)llHdl->drvRingBuffer, llHdl->drvRingSize * 2);
	}

	/* Doesn't need to clear the rest of the configuration */

	/*return error code */
	return (retCode);
}

/**********************************************************************/
/** Configure default values to registers.
 *
 *  Sets the controller registers to default values:
 *  - all interrupt ebabled
 *  - default timeout and receive settings
 *
 *
 *  \param llHdl      \IN  low-level handle
 */

static void ConfigureDefault( LL_HANDLE *llHdl )
{

	MWRITE_D8(llHdl->ma, Z147_RX_RST_OFFSET, 1);

	/* Configure RX LCR */
	MWRITE_D8(llHdl->ma, Z147_RX_LCR_OFFSET, Z147_RX_LCR_DEFAULT);
//	MWRITE_D8(llHdl->ma, Z147_RX_LCR_OFFSET, 0); // REMOVE (sync disable)

	/* Enable the default interrupts */
	MWRITE_D8(llHdl->ma, Z147_RX_IER_OFFSET, Z147_RX_IER_DEFAULT);

	/* Set the speed */
	SetDataRate(llHdl, Z147_RX_DATA_RATE_64);

	/* Reset the driver sync. */
	llHdl->isDrvSync = 0;
	/* Clear the error register. */
	MWRITE_D8(llHdl->ma, Z147_LSR_REG_OFFSET, 0xFF); // REMOVE

	MWRITE_D8(llHdl->ma, Z147_RX_RST_OFFSET, 0);

	DBGWRT_2((DBH, " >>  LL - Z147_Init: Register status after Init"));
	RegStatus(llHdl);
}


/**********************************************************************/
/** Set TX Data rate.
 *
 *  Set transmit data rate according to the user configuration.
 *
 *  \param llHdl      \IN  low-level handle
 *  \param rxSpeed    \IN  rx speed
 *  \return           \OUT uint32 data
 */
int SetDataRate( LL_HANDLE *llHdl, u_int8 rxSpeed){

	int result = 0;
	int buffSize = 0;
	int gotsize = 0;
	int trigLevel = 0;
	u_int8 regData = 0;

	switch(rxSpeed){
	case Z147_RX_DATA_RATE_64:
		llHdl->subFrameSize = 64;
		trigLevel = Z147_RX_TRIG_LVL_08;
		buffSize = 512;
		break;
	case Z147_RX_DATA_RATE_128:
		llHdl->subFrameSize = 128;
		trigLevel = Z147_RX_TRIG_LVL_16;
		buffSize = 1024;
		break;
	case Z147_RX_DATA_RATE_256:
		llHdl->subFrameSize = 256;
		trigLevel = Z147_RX_TRIG_LVL_32;
		buffSize = 2048;
		break;
	case Z147_RX_DATA_RATE_512:
		llHdl->subFrameSize = 512;
		trigLevel = Z147_RX_TRIG_LVL_64;
		buffSize = 4096;
		break;
	case Z147_RX_DATA_RATE_1024:
		llHdl->subFrameSize = 1024;
		trigLevel = Z147_RX_TRIG_LVL_128;
		buffSize = 8192;
		break;
	case Z147_RX_DATA_RATE_2048:
		llHdl->subFrameSize = 2048;
		trigLevel = Z147_RX_TRIG_LVL_256;
		buffSize = 16384;
		break;
	case Z147_RX_DATA_RATE_4096:
		llHdl->subFrameSize = 4096;
		trigLevel = Z147_RX_TRIG_LVL_256;
		buffSize = 32768;
		break;
	case Z147_RX_DATA_RATE_8192:
		llHdl->subFrameSize = 8192;
		trigLevel = Z147_RX_TRIG_LVL_256;
		buffSize = 65536;
		break;
	default:
		rxSpeed = Z147_RX_DATA_RATE_64;
		trigLevel = Z147_RX_TRIG_LVL_08;
		llHdl->subFrameSize = 64;
		buffSize = 512;
		break;
	}
	/* Delete the allocated memory. */
	if(llHdl->usrBuffer != NULL){
		OSS_MemFree(llHdl->osHdl, (int8*)llHdl->usrBuffer, llHdl->usrBuffSize *2);
		llHdl->usrBuffer = NULL;
	}
	if(llHdl->drvRingBuffer != NULL){
		OSS_MemFree(llHdl->osHdl, (int8*)llHdl->drvRingBuffer, llHdl->drvRingSize * 2);
		llHdl->drvRingBuffer = NULL;
	}

	/* Allocate memory to user buffer */
	if ((llHdl->drvRingBuffer = (u_int16*)OSS_MemGet(
			OSH, buffSize, &gotsize)) == NULL)
	{
		llHdl->drvRingBuffer = NULL;
		result = ERR_OSS_MEM_ALLOC;
	}else{

		/* Allocate memory to driver ring buffer */
		if ((llHdl->usrBuffer = (u_int16*)OSS_MemGet(
				OSH, buffSize, &gotsize)) == NULL)
		{
			llHdl->usrBuffer = NULL;
			result = ERR_OSS_MEM_ALLOC;
		}else{
			llHdl->usrBuffSize = buffSize/2;
			llHdl->drvRingSize = buffSize/2;

			regData = MREAD_D8(llHdl->ma, Z147_RX_LCR_OFFSET);

			regData = regData & (~Z147_RX_DATA_RATE_MASK);
			regData |= ((rxSpeed << Z147_RX_DATA_RATE_OFFSET) & Z147_RX_DATA_RATE_MASK);
			/* Set the Data Rate  */
			MWRITE_D8(llHdl->ma, Z147_RX_LCR_OFFSET, regData);

			/* Configure RX FCR */
			MWRITE_D8(llHdl->ma, Z147_RX_FCR_OFFSET, trigLevel);

			llHdl->isDrvSync = 0;
			llHdl->disableRx = 0;
			llHdl->drvRingHead = 0;
			llHdl->drvRingSyncPos = 0;
			llHdl->isRxIrqExit = 0;
			llHdl->isUsrDataUpdated = 0;
		}
	}

	return result;
}

/**********************************************************************/
/** Print register configuration.
 *
 *  Print the status of the registers.
 *
 *  \param llHdl      \IN low-level handle
 */

void RegStatus(LL_HANDLE *llHdl){
	DBGWRT_2((DBH, " \n"));
	DBGWRT_2((DBH, " >>  LL - Z147_drv status: IIR  = 0x%x\n", MREAD_D8(llHdl->ma, Z147_STAT_REG)));
	DBGWRT_2((DBH, " >>  LL - Z147_drv status: RXA  = 0x%x\n", MREAD_D16(llHdl->ma, Z147_RX_RXA_OFFSET)));
	DBGWRT_2((DBH, " >>  LL - Z147_drv status: RXC  = 0x%x\n", MREAD_D16(llHdl->ma, Z147_RX_RXC_REG_OFFSET)));
	DBGWRT_2((DBH, " >>  LL - Z147_drv status: IER  = 0x%x\n", MREAD_D8(llHdl->ma, Z147_RX_IER_OFFSET)));
	DBGWRT_2((DBH, " >>  LL - Z147_drv status: LCR  = 0x%x\n", MREAD_D8(llHdl->ma, Z147_RX_LCR_OFFSET)));
	DBGWRT_2((DBH, " >>  LL - Z147_drv status: FCR  = 0x%x\n", MREAD_D8(llHdl->ma, Z147_RX_FCR_OFFSET)));
	DBGWRT_2((DBH, " >>  LL - Z147_drv status: RST  = 0x%x\n", MREAD_D8(llHdl->ma, Z147_RX_RST_OFFSET)));

	DBGWRT_2((DBH, " \n"));
}
