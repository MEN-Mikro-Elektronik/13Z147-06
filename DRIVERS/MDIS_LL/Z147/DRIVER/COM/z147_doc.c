/*********************  P r o g r a m  -  M o d u l e ***********************/
/*!
 *        \file  z147_doc.c
 *
 *      \author  APatil
 *        $Date: 2015/10/16 18:08:00 $
 *        $Revision: 1.1 $
 *
 *      \brief   User documentation for MDIS5 Z147 driver
 *
 *     Required: -
 *
 *     \switches -
 */
 /*-------------------------------[ History ]--------------------------------
 *
 * $Log: z147_doc.c,v $
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2015 by MEN Mikro Elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/

/*! \mainpage
	The Z147 MDIS Driver Package contains:\n
	- ARINC717 RX drivers for the 16Z147_ARINC717 IP core\n
	- ARINC717 TX drivers for the 16Z247_ARINC717 IP core\n
	
	\n
	The RX and TX IP core channels can be used independently using RX and TX
	instances of the driver. Each instance of the RX and TX can be configured
	independently.

	The driver supports 8 different data rates for transmission and reception:
	-	768 Bits/sec.   ( 64  words/sec.)
	-	1536 Bits/sec. (128 words/sec.)
	-	3072 Bits/sec. (256 words/sec.)
	-	6144 Bits/sec. (512 words /sec.)
	-	12288 Bits/sec. (1024 words/sec.)
	-	24576 Bits/sec. (2048 words/sec.)
	-	49152 Bits/sec. (4096 words/sec.)
	-	98304 Bits/sec. (8192 words/sec.)	
	
    The drivers provide following possibilities to configure ARINC 717 frames:
    - Transmit/Receive Data rate
	- Transmit/Receive mode Harvard Bi-Phase/Bipolar Return to Zero
	- Receive synchronization mode

	\n
	\section Variants Variants
	- rx : ARINC717 RX driver
	- tx : ARINC717 TX driver


	\n
	\section RxFuncDesc === RX Driver Functional Description ===

	\n \subsection RxGeneral General
	The receive driver can be configured and used independently. The configuration
	of the receive driver must be same as the transmitter of the other end, in
	order to work the communication.
	The default configuration of the receive driver is compatible to the transmit
	driver.	Any change in the receive driver is supposed to be taken into consideration
	for the transmit driver in order to avoid failure of the communication.

    \n \subsection RxRead Receiving Data
	The M_getblock() reads data from the driver. The driver receives the data and
	store it in an internal buffer. When data is received, a signal is generated
	by the driver which was assigned using M_setstat() #Z147_SET_SIGNAL to the
	application. 
	
	According the ARINC 717 specification, the data reception is cyclic when the
	receiver is synchronized with the transmitter. The user will get received data
	including the sync words 0x247 (SUB_FRAME_1), 0x5B8 (SUB_FRAME_2),
	0xA47 (SUB_FRAME_3), 0xDB8(SUB_FRAME_4), to identify the subframes.


    \n \subsection RxInterrupts Interrupt and Signal
    
    If an interrupt is enabled the driver will send the signal which was assigned
    using M_setstat() #Z147_SET_SIGNAL to the application when data is received. 
	The signal can be uninstalled using #Z147_CLR_SIGNAL.


 	\n \subsection RxSetget Driver Configuration 
	The driver can be configured using M_setstat(), using following options:
	
	    <table border="0">	   
	<tr>
	<td><b>Config Parameter</b></td>
	<td><b> Values </b></td>
	<td><b>Description</b></td></tr>

    <tr><td>#Z147_RX_DATA_RATE</td><td> - 0b000</td><td>: 768 Bits/sec. (64 words/sec.)</td></tr>

    <tr><td>   </td><td> - 0b001 </td>   <td>: 1536Bits/sec.   (128 words/sec.)</td></tr>
    <tr><td>   </td><td> - 0b010 </td>    <td>: 3072 Bits/sec.   (256 words/sec.)</td></tr>
    <tr><td>   </td><td> - 0b011   </td>    <td>: 6144 Bits/sec.   (512 words/sec.)</td></tr>
    <tr><td>   </td><td> - 0b100   </td>    <td>: 12288 Bits/sec. (1024 words/sec.)</td></tr>
	<tr><td>   </td><td> - 0b101   </td>    <td>: 24576 Bits/sec. (2048 words/sec.)</td></tr>
	<tr><td>   </td><td> - 0b110   </td>    <td>: 49152 Bits/sec. (4096 words/sec.)</td></tr>
	<tr><td>   </td><td> - 0b111   </td>    <td>: 98304 Bits/sec. (8192 words/sec.)</td></tr>
	<tr><td>#Z147_RX_SYNC_CFG</td><td> -  0b000</td><td>: No synchronization: Start receiving from the first detected edge </td></tr>
    <tr><td>   </td><td> -  0b001   </td>   <td>: Part synchronization: Start receiving when only two sync words are in the correct order</td></tr>
    <tr><td>   </td><td> -  0b010   </td>    <td>: Full synchronization: Start receiving when all sync words came in the correct order</td></tr>
    <tr><td>   </td><td> -  0b011   </td>    <td>: Reserved (Full synchronization shall be used)</td></tr>

    </table>
	
   
    \n \subsection RxDefault Default values
    M_open() and M_close() configures the Receive driver as follows: 
    
    - Receive Rate: 768 Bits/sec. (64  words/sec.)
	- Receive mode: Harvard Bi-Phase 
	- Receive sync mode: 0b10 = Full synchronization 


    \n \section RxApi_functions Supported API Functions of the Receive Driver
	
	<table border="0">
    <tr>
        <td><b>API function</b></td>
        <td><b>Functionality</b></td>
        <td><b>Corresponding low level function</b></td></tr>

    <tr><td>M_open()</td><td>Open device</td><td>Z147_Init()</td></tr>

    <tr><td>M_close()     </td><td>Close device             </td>
    <td>Z147_Exit())</td></tr>
	<tr><td>M_getblock()  </td><td>Block read from device   </td>
    <td>Z147_BlockRead()</td></tr>
    
    <tr><td>M_setstat()   </td><td>Set device parameter     </td>
    <td>Z147_SetStat()</td></tr>
    <tr><td>M_getstat()   </td><td>Get device parameter     </td>
    <td>Z147_GetStat()</td></tr>

    </table>

    \n \section RxDescriptor_entries Descriptor Entries
    (no descriptor entries supported)    
    
    \n \section RxCodes Driver specific Getstat/Setstat codes
    see \ref rx_getstat_setstat_codes "section about Getstat/Setstat codes"


    \n \section TxFuncDesc === TX Driver Functional Description ===

    \n \subsection TxGeneral General
	The transmit driver can be configured and used independently. The configuration
	of the transmit driver must be same as the receiver of the other end, in
	order to work the communication.
	The default configuration of the transmit driver is compatible to the receive
	driver. Any change in the transmit driver suppose to be taken into consideration
	for the receive driver in order to avoid failure of the communication.
	
    \n \subsection TxWrite Transmitting Data
	The M_setblock() writes data from the user provided buffer to the device.
	The configured data rate needs to be taken into consideration in order to
	know about user data size.
	As per the ARINC 717 specification, the data transmission is cyclic. The cyclic 
	transmission is triggered by the first data request from the user. Once the
	transmitter is synchronized with receiver, it stays in sync by transmitting
	the same data continuously. When the user transmits new data, it will be
	buffered internally and then transmitted later with a new frame cycle.
	
    \n \subsection TxInterrupts Interrupt and Signal
    
 	A signal is generated by the driver, which was assigned using M_setstat()
 	#Z247_SET_SIGNAL to the application, when a transmission request is completed.
	The signal can be uninstalled using #Z247_CLR_SIGNAL.

	\n \subsection TxSetget Driver Configuration 
	The driver can be configured using M_setstat(), using following options:
	
    <table border="0">	   
	<tr>
	<td><b>Config Parameter</b></td>
	<td><b>Value</b></td>
	<td><b>Description</b></td></tr>

    <tr><td>#Z247_TX_DATA_RATE</td><td> - 0b000</td><td>: 768 Bits/sec. (64 words/sec.)</td></tr>

    <tr><td>   </td><td> - 0b001   </td>   <td>: 1536Bits/sec.   (128 words/sec.)</td></tr>
    <tr><td>   </td><td> - 0b010   </td>    <td>: 3072 Bits/sec.   (256 words/sec.)</td></tr>
    <tr><td>   </td><td> - 0b011   </td>    <td>: 6144 Bits/sec.   (512 words/sec.)</td></tr>
    <tr><td>   </td><td> - 0b100   </td>    <td>: 12288 Bits/sec. (1024 words/sec.)</td></tr>
	<tr><td>   </td><td> - 0b101   </td>    <td>: 24576 Bits/sec. (2048 words/sec.)</td></tr>
	<tr><td>   </td><td> - 0b110   </td>    <td>: 49152 Bits/sec. (4096 words/sec.)</td></tr>
	<tr><td>   </td><td> - 0b111   </td>    <td>: 98304 Bits/sec. (8192 words/sec.)</td></tr>

    </table>
	
    \n \subsection TxDefault Default values
    M_open() and M_close() configures the transmit driver as follows: 
    
    - Transmit Rate: 768 Bits/sec. (64  words/sec.)
	- Transmit mode: Harvard Bi-Phase 
	
    \n \section TxApi_functions Supported API Functions of the Transmit Driver

    <table border="0">
    <tr>
        <td><b>API function</b></td>
        <td><b>Functionality</b></td>
        <td><b>Corresponding low level function</b></td></tr>

    <tr><td>M_open()</td><td>Open device</td><td>Z247_Init()</td></tr>

    <tr><td>M_close()     </td><td>Close device             </td>
    <td>Z247_Exit())</td></tr>
    <tr><td>M_setblock()  </td><td>Block write from device  </td>
    <td>Z247_BlockWrite()</td></tr>
    <tr><td>M_setstat()   </td><td>Set device parameter     </td>
    <td>Z247_SetStat()</td></tr>
    <tr><td>M_getstat()   </td><td>Get device parameter     </td>
    <td>Z247_GetStat()</td></tr>

    </table>

    \n \section TxDescriptor_entries Descriptor Entries
    (no descriptor entries supported)    
    
    \n \section TxCodes Driver specific Getstat/Setstat codes
    see \ref tx_getstat_setstat_codes "section about Getstat/Setstat codes"

    \n \section Documents Overview of all Documents

    \subsection z147_example  Simple example for using the driver
    z147_example.c (see example section)
*/

/** \example z147_example.c
Simple example for driver usage
*/

/*! \page dummy
  \menimages
*/

