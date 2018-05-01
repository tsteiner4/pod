#include "../include/can.h"

/******************************************************************************/
/*                                Global Variables                             */
/******************************************************************************/
uint8_t CAN_TxReady = 0;
uint8_t CAN_RxReceived = 0;

CAN_MESSAGE CAN_RxMsg0, CAN_RxMsg1;

/******************************************************************************/
/******************************************************************************/


/******************************************************************************/
/*                              Initialization                                */
/******************************************************************************/

//Is this something we are going to use?
void CAN_filterInit(void){

	/*
To initialize the registers associated with the CAN filter banks (mode, scale, FIFO
assignment, activation and filter values), software has to set the FINIT bit (CAN_FMR). Filter
initialization also can be done outside the initialization mode. 
	*/

}

//Configuring the Master Control Register 
void CAN_MCRInit(void){

	can1->MCR |= 1<< 6; //Automatic Bus-Off Management
	can1->MCR |= 1<<5; //Automatic Wakeup Mode

	//TODO: is this something we want?
	can1->MCR |= 1<<4; //Non-Automatic Retransmission 
	can1->MCR |= 1<<2; //Transmit Priority

}


/* //Basically made a list of CAN related interrupts we could possibly use


void CAN_interruptInit(void){
	
	CAN_TypeDef *mycan = CAN1_BASE;

	//TODO: check whether we need certain interrupts or not, marked by "Not sure"

	//Clear all bits in Interrupt Enable Register
	mycan->IER = 0;

	//Bit Number: Description
	//17: Sleep Interrupt Enable
	mycan->IER |= CAN_IER_SLKIE_Msk; //Not sure
	
	//16: Wakeup Interrupt Enable
	mycan->IER |= CAN_IER_WKUIE_Msk; //Not sure
	
	//15: Error Interrupt Enable
	mycan->IER |= CAN_IER_ERRIE_Msk; 

	//11: Last Error Code Interrupt Enable
	mycan->IER |= CAN_IER_LECIE_Msk; //Not sure 

	//10: Bus-Off Interrupt Enable
	mycan->IER |= CAN_IER_BOFIE_Msk; //Not sure

	//9: Error Passive Interrupt Enable
	mycan->IER |= CAN_IER_EPVIE_Msk;

	//8: Error Warning Interrupt Enable
	mycan->IER |= CAN_IER_EWGIE_Msk; 
	

	//RECEIVE FIFO #1 INTERRUPTS
	//6: Receive FIFO #1 Overrun Interrupt Enable
	mycan->IER |= CAN_IER_FOVIE1_Msk; 

	//5: Receive FIFO #1 FIFO Full Interrupt Enable
	mycan->IER |= CAN_IER_FFIE1_Msk; 

	//4: Receive FIFO #1 Message Pending Interrupt Enable
	mycan->IER |= CAN_IER_FMPIE1_Msk; 	


	//RECEIVE FIFO #0 INTERRUPTS
	//3: Receive FIFO #0 FIFO Overrun Interrupt Enable
	mycan->IER |= CAN_IER_FOVIE0_Msk; 

	//2: Receive FIFO #0 FIFO Full Interrupt Enable
	mycan->IER |= CAN_IER_FFIE0_Msk; 

	//1: Receive FIFO #0 FIFO Message Pending Interrupt Enable
	mycan->IER |= CAN_IER_FMPIE0_Msk; 

	//0: Transmit Mailbox Empty Interrupt Enable
	mycan->IER |= CAN_IER_TMEIE_Msk; //Not sure, might want to fill this just so 
   									 //we are sending something	

}

*/

//Initializing send and receive mailboxes/FIFOs
void CAN_mailboxInit(void) {

	CAN_TypeDef *mycan = CAN1_BASE;

	//pointer (array) of type TxMailBox, of size 3. Represents tx mailboxes
	CAN_TxMailBox_TypeDef *tx_mailbox = mycan->sTxMailBox; 

	//array of type FIFOMailBox, of size 2. Represents 2 receive fifos
	CAN_FIFOMailBox_TypeDef *rx_fifo = mycan->sFIFOMailBox;


////////////////////Transfer Mailboxes (3) CAN_TIxR {x = 0,1,2}//////////////////////////////////////////////////
//BITS 31-21 are Standard ID bits
//Bit 2 is the standard id/ext  (0)
//Bit 1 is remote frame/data frame (0)
//Bit 0 is the transmit request set by software, cleared by hardware when sent

//Data Registers: 
	//High Byte: CAN_TDHxR {x = 0,1,2}
	//Low Byte: CAN_TDLxR {x = 0,1,2}


///////////////////Receive Mailboxes (2) CAN_RIxR {x = 0,1}/////////////////////////
//Bits 31-21 are standard id
//Bit 2 is standard id/ext id (0)
//Bit 1 is remote frame/data frame (0)
//Bit 0 reserved

//Data Registers: 
	//High Byte: CAN_RDHxR {x = 0,1}
	//Low Byte: CAN_RDLxR {x = 0,1}

	//Set all FIFOs/Mailboxes to use standard identifier, not extended
	tx_mailbox[0].TIR &= ~CAN_TI0R_IDE_Msk;
	tx_mailbox[1].TIR &= ~CAN_TI1R_IDE_Msk;
	tx_mailbox[2].TIR &= ~CAN_TI2R_IDE_Msk;
	
	rx_fifo[0].RIR &= ~CAN_RI0R_IDE_Msk;
	rx_fifo[1].RIR &= ~CAN_RI1R_IDE_Msk;

	//Set to data frames //TODO: Check if we want remote frames or data frames
	tx_mailbox[0].TIR &= ~CAN_TI0R_RTR_Msk;	
	tx_mailbox[0].TIR &= ~CAN_TI1R_RTR_Msk;	
	tx_mailbox[0].TIR &= ~CAN_TI2R_RTR_Msk;

	rx_fifo[0] &= ~CAN_RI0R_RTR_Msk;
	rx_fifo[1] &= ~CAN_RI1R_RTR_Msk;

	
}

//Set up CAN_BTR register, called from CAN_init()
void CAN_setBitTiming(void) {


}

void CAN_init(void) {
	
	CAN_TypeDef *can1 = CAN1_BASE; //CAN1 Base

	//TODO Actually write these values to the CAN_MCR Register correctly
	can1->MCR |= 1<<15; //Master Software Reset
    
	can1->MCR |= CAN_MCR_INRQ_Msk; //Set CAN to Initialization Mode

	while( can1->MSR & CAN_MSR_INAK_Msk == 0  ) {} //Waits until Hardware signals we are in
								//Initialization mode

	//CAN_filterInit();	
    CAN_setBitTiming();
    CAN_mailboxInit;
    CAN_interruptInit();
	
	can1->MCR &= (~0x1); //Sets CAFIFOMailBox to Normal Mode
	
	CAN_start();
 
	//Finish rest of configuration:
	//
	//From pg. 1355 of ref. manual
	//
	//"To initialize the CAN Controller, software has to set up the Bit Timing (CAN_BTR) and CAN
	//options (CAN_MCR) registers.
	//To initialize the registers associated with the CAN filter banks (mode, scale, FIFO
	//assignment, activation and filter values), software has to set the FINIT bit (CAN_FMR). Filter
	//initialization also can be done outside the initialization mode. "

}

/*----------------------------------------------------------------------------
  Leave initialisation mode, start normal mode
 *----------------------------------------------------------------------------*/
void CAN_start (void)  {

  CAN_TypeDef *can1 = CAN1_BASE; //CAN1 Base
  
  can1->MCR &= ~CAN_MCR_INRQ;                      // normal operating mode, reset INRQ
  while( can1->MSR & CAN_MSR_INAK_Msk != 0  ) {} //Waits until hardware leaves Initialization mode

}

/******************************************************************************/
/******************************************************************************/


/******************************************************************************/
/*                                Utility                                     */
/******************************************************************************/

//Functions for disabling all CAN based interrupts, if that's ever needed
void disable_CAN_interrupts(void){
	
	CAN_TypeDef *mycan = CAN1_BASE;

	//Clear all bits in Interrupt Enable Register
	mycan->IER = 0;
}

/******************************************************************************/
/******************************************************************************/


/******************************************************************************/
/*                         Sending Messages                                   */
/******************************************************************************/

/*
Bits 25:24 CODE[1:0]: Mailbox code
In case at least one transmit mailbox is free, the code value is equal to the number of the
next transmit mailbox free.
In case all transmit mailboxes are pending, the code value is equal to the number of the
transmit mailbox with the lowest priority
*/

/*----------------------------------------------------------------------------
  wite a message to CAN peripheral and transmit it (only uses mailbox 0) 
 *----------------------------------------------------------------------------*/
void CAN_WriteMsg (CAN_msg *msg)  {

  //Check if TX is ready before sending a message

  CAN_TypeDef *can1 = CAN1_BASE; //CAN1 Base
                                                  // Setup identifier information
  can1->sTxMailBox[0].TIR |= (unsigned int)(msg->id << 21) | CAN_ID_STD;
                                                  // Setup type information
  
  can1->sTxMailBox[0].TIR |= CAN_RTR_DATA;
  
                                                  // Setup data bytes
  can1->sTxMailBox[0].TDLR = (((unsigned int)msg->data[3] << 24) | 
                             ((unsigned int)msg->data[2] << 16) |
                             ((unsigned int)msg->data[1] <<  8) | 
                             ((unsigned int)msg->data[0])        );
  can1->sTxMailBox[0].TDHR = (((unsigned int)msg->data[7] << 24) | 
                             ((unsigned int)msg->data[6] << 16) |
                             ((unsigned int)msg->data[5] <<  8) |
                             ((unsigned int)msg->data[4])        );
                                                  // Setup length
  can1->sTxMailBox[0].TDTR &= ~CAN_TDT0R_DLC;
  can1->sTxMailBox[0].TDTR |=  (msg->len & CAN_TDT0R_DLC);

  can1->IER |= CAN_IER_TMEIE;                      // enable  TME interrupt 
  can1->sTxMailBox[0].TIR |=  CAN_TI0R_TXRQ;       // transmit message
}

/******************************************************************************/
/******************************************************************************/


/******************************************************************************/
/*                        Receiving Messages                                  */
/******************************************************************************/

/*----------------------------------------------------------------------------
  read a message from CAN peripheral and release it (uses only receive FIFO 0)
 *----------------------------------------------------------------------------*/
void CAN_ReadMsg0 (CAN_msg *msg)  {

  //TODO: Read FIFO 0 Status, see how many messages it has, then read them out 
  
                                                  // Read identifier information
  msg->id = (uint32_t)0x000007FF & (CAN->sFIFOMailBox[0].RIR >> 21);
     
                                                  // Read length (number of received bytes)
  msg->len = (unsigned char)0x0000000F & CAN->sFIFOMailBox[0].RDTR;
  
                                                  // Read data bytes
  msg->data[0] = (unsigned int)0x000000FF & (CAN->sFIFOMailBox[0].RDLR);
  msg->data[1] = (unsigned int)0x000000FF & (CAN->sFIFOMailBox[0].RDLR >> 8);
  msg->data[2] = (unsigned int)0x000000FF & (CAN->sFIFOMailBox[0].RDLR >> 16);
  msg->data[3] = (unsigned int)0x000000FF & (CAN->sFIFOMailBox[0].RDLR >> 24);

  msg->data[4] = (unsigned int)0x000000FF & (CAN->sFIFOMailBox[0].RDHR);
  msg->data[5] = (unsigned int)0x000000FF & (CAN->sFIFOMailBox[0].RDHR >> 8);
  msg->data[6] = (unsigned int)0x000000FF & (CAN->sFIFOMailBox[0].RDHR >> 16);
  msg->data[7] = (unsigned int)0x000000FF & (CAN->sFIFOMailBox[0].RDHR >> 24);

  CAN->RF0R |= CAN_RF0R_RFOM0;                    // Release FIFO 0 output mailbox
}

/******************************************************************************/
/******************************************************************************/


/******************************************************************************/
/******************************************************************************/


/******************************************************************************/
/*                                    ISRs                                    */
/******************************************************************************/

/*----------------------------------------------------------------------------
  CAN transmit interrupt handler
 *----------------------------------------------------------------------------*/
void CAN1_TX_IRQHandler(void) {

  CAN_TypeDef *can1 = CAN1_BASE;

  if (can1->TSR & CAN_TSR_RQCP0) {                 // request completed mbx 0
    can1->TSR |= CAN_TSR_RQCP0;                    // reset request complete mbx 0
    can1->IER &= ~CAN_IER_TMEIE;                   // disable TME interrupt
	
	CAN_TxReady = 1; 
  }
}

/*----------------------------------------------------------------------------
  CAN receive interrupt handlers
 *----------------------------------------------------------------------------*/
void CAN1_RX0_IRQHandler (void) {

  CAN_TypeDef *can1 = CAN1_BASE;

  if (can1->RF0R & CAN_RF0R_FMP0) {			      // message pending ?
	CAN_ReadMsg(&CAN_RxMsg0);                       // read the message

    CAN_RxReceived = 1;                                // set receive flag
  }
}

void CAN1_RX1_IRQHandler(void) {

  CAN_TypeDef *can1 = CAN1_BASE;

  if (can1->RF1R & CAN_RF1R_FMP2) {			      // message pending ?
	CAN_ReadMsg1(&CAN_RxMsg1);                       // read the message

    CAN_RxReceived = 1;                                // set receive flag
  }
}

//TODO: Check CAN_ESR reg for error status
void CAN1_SCE_IRQHandler( void ){


}
/******************************************************************************/
/******************************************************************************/


/******************************************************************************/
/*        Compiled depending on availablity of Serial Debug                   */
/******************************************************************************/

//TODO: Redo debugging functions
void CAN_print_errors(void) {

}

void CAN_message_dump(CAN_MESSAGE *message, bool outgoing) {

}

bool CAN_ping(uint16_t SID, bool initiator) {

}
/******************************************************************************/
/******************************************************************************/

