#include "../include/can.h"

/******************************************************************************/
/*                                Local Variables                             */
/******************************************************************************/
static volatile unsigned int numOverflows = 0;

/******************************************************************************/
/******************************************************************************/


/******************************************************************************/
/*                              Initialization                                */
/******************************************************************************/

//Initializing filters needed for proper CAN function
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

//Set up which interrrupts we care to use
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

	CAN_filterInit();	
    CAN_setBitTiming();
    CAN_mailboxInit;
    CAN_interruptInit();
	
	can1->MCR &= (~0x1); //Sets CAFIFOMailBox to Normal Mode
	
	while( can1->MSR & CAN_MSR_INAK_Msk != 0  ) {} //Waits until hardware leaves 
						//Initialization mode

	//TODO finish rest of configuration:
	//
	//From pg. 1355 of ref. manual
	//
	//"To initialize the CAN Controller, software has to set up the Bit Timing (CAN_BTR) and CAN
	//options (CAN_MCR) registers.
	//To initialize the registers associated with the CAN filter banks (mode, scale, FIFO
	//assignment, activation and filter values), software has to set the FINIT bit (CAN_FMR). Filter
	//initialization also can be done outside the initialization mode. "

}
/******************************************************************************/
/******************************************************************************/


/******************************************************************************/
/*                                Utility                                     */
/******************************************************************************/
//TODO: figure out how to check error
int CAN_check_error(void){  

}

void check_bus_integrity(void) {
    
}

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

// Returns true/false based on whether or not it's possible to send the message currently
bool CAN_send( int numBytes, uint32_t *data_buf) {
    
	int mailbox_num = 0;
	int i;
	Can_TypeDef *can1 = CAN1_BASE;
	
	//pointer (array) of type TxMailBox, of size 3. Represents tx mailboxes
	CAN_TxMailBox_TypeDef *tx_mailbox = can1->sTxMailBox; 
	
	//Select an empty mailbox, and update mailbox_num accordingly (0..2)
	for(i = 0; i< 3; i++){	
		if( (can1->TSR & ( ~(CAN_TSR_TME0_Msk << i) ) ) != 0){
			mailbox_num = i;
			break;
		}
		if( i == 2 ){ //All mailboxes are full
			//TODO figure out what to do when mailboxes are full
		}
	}

	// TODO Set up the data length code (DLC) and the data
	

	//Request Transmission of message in corresponding mailbox
	tx_mailbox[mailbox_num].TIR = CAN_TI0R_TXRQ_Msk;

	//Wait until mailbox status is empty, before checking status of message
	while( (can1->TSR & ( ~(CAN_TSR_TME0_Msk << mailbox_num) ) ) == 0){}

	//Check successful sending bits RQCP and TXOK
	if ( (can1->TSR & (CAN_TSR_RQCP0_Msk << (mailbox_num * 8)) == 0) ||  
			(can1->TSR & (CAN_TSR_TXOK0_Msk << ( mailbox_num * 8)) == 0)) { //if fault occured
        
		
		//Arbitration Lost fault
		if( can1->TSR & (CAN_TSR_ALST0_Msk << (mailbox_num * 8)) != 0  ){
			//TODO Handle Fault
		}
		//Transmission Error Detection Fault
		else if(_can1->TSR & (CAN_TSR_TERR0_Msk << (mailbox_num * 8)) != 0 ){
			//TODO Handle Fault
		}
		return false;

    }
    if (debuggingOn) CAN_message_dump(sending, true);
    return true;
}

/******************************************************************************/
/******************************************************************************/


/******************************************************************************/
/*                        Receiving Messages                                  */
/******************************************************************************/

//Function for receiving all messages found in Receive FIFOs. Messages are returned in 
//an array up to size 6.
bool CAN_receive_all( uint32_t *rec_buf ) {	

	int fifo0, fifo1, i;

	CAN_TypeDef * can1 = CAN1_BASE;

	fifo0 = can1->RF0R & CAN_RF0R_FMP0_Msk;
	fifo1 = can1->RF1R & CAN_RF1R_FMP0_Msk;
	
	if( (fifo0 + fifo1) == 0){
		return true; //No messages to receive, do nothing
	}
	//otherwise allocate a buffer to hold all messages
	//NOTE: will be calling function's responsibility to free memory
	rec_buf = rec_buf = malloc( (fifo0+fifo0) * sizeof(CAN_MESSAGE) ); 
	if(rec_buf == NULL){ //Error in allocation
		return false;
	}

	if( fifo0 > 0){ //There is at least 1 message in FIFO0
			
			for( i = 0; i < fifo0; i++){
				rec_buf[i].id = can1->sFIFOMailBox[0].RIR;	
				rec_buf[i].dlc_time_stamp = can1->sFIFOMailBox[0].RTDR;
				rec_buf[i].data_low = can1->sFIFOMailBox[0].RDLR;
				rec_buf[i].data_high = can1->sFIFOMailBox[0].RDHR;
			}
	}
	if( fifo1 > 0){ //There is at least 1 message in FIFO1
		
			for( i = fifo0; i < (fifo1+fifo0); i++){
				rec_buf[i].id = can1->sFIFOMailBox[1].RIR;	
				rec_buf[i].dlc_time_stamp = can1->sFIFOMailBox[1].RTDR;
				rec_buf[i].data_low = can1->sFIFOMailBox[1].RDLR;
				rec_buf[i].data_high = can1->sFIFOMailBox[1].RDHR;
			}
	}
	
	//TODO Check Error Cases

    if (debuggingOn) CAN_message_dump(&receiving, false);
    
    return true;
}


//Function for receiving one entire FIFO, based on the value of fifo_select. (0 or 1) 
// The entire FIFO needs to read out because FIFO is set to empty once a message
// is acknowledged, no matter how many messages are stored.
bool CAN_receive_FIFO( int fifo_select, uint32_t *rec_buf) {

	int fifo0, fifo1, i;

	CAN_TypeDef * can1 = CAN1_BASE;

	fifo0 = (can1->RF0R & CAN_RF0R_FMP0_Msk) & (fifo_select == 0);
	fifo1 = can1->RF1R & CAN_RF1R_FMP0_Msk & (fifo_select == 1);

	if( fifo0 ){ //There is at least 1 message in FIFO0 and it got selected
			//NOTE: will be calling function's responsibility to free memory
			rec_buf = malloc(fifo0 * sizeof(CAN_MESSAGE) );
			for( i = 0; i < fifo0; i++){
				rec_buf[i].id = can1->sFIFOMailBox[0].RIR;	
				rec_buf[i].dlc_time_stamp = can1->sFIFOMailBox[0].RTDR;
				rec_buf[i].data_low = can1->sFIFOMailBox[0].RDLR;
				rec_buf[i].data_high = can1->sFIFOMailBox[0].RDHR;
			}
	}
	else if( fifo1 ){ //There is at least 1 message in FIFO1 and it got selected
		
			//NOTE: will be calling function's responsibility to free memory
			rec_buf = malloc(fifo1 * sizeof(CAN_MESSAGE) );
			for( i = 0; i < fifo1; i++){
				rec_buf[i].id = can1->sFIFOMailBox[1].RIR;	
				rec_buf[i].dlc_time_stamp = can1->sFIFOMailBox[1].RTDR;
				rec_buf[i].data_low = can1->sFIFOMailBox[1].RDLR;
				rec_buf[i].data_high = can1->sFIFOMailBox[1].RDHR;
			}
	}
	else{ //No messages to receive, do nothing
		return true;
	}
	
	//TODO Check Error Cases

    if (debuggingOn) CAN_message_dump(&receiving, false);
    
    return true;
}
/******************************************************************************/
/******************************************************************************/


/******************************************************************************/
/*                          Heartbeat Related                                 */
/******************************************************************************/
//TODO: what does this do?
void CAN_send_fault(void) {
    
}

//TODO: Send heartbeat message, figure out what needs to be sent
void CAN_send_heartbeat(void) {
   	
	int numBytes;
	uint32_t data_buf[ TODO ];

	numBytes = //TODO;
	data_buf = //TODO;

	CAN_send( numBytes, &data_buf);

}
/******************************************************************************/
/******************************************************************************/


/******************************************************************************/
/*                                    ISRs                                    */
/******************************************************************************/

//TODO: create ISR's for CAN interrupts
void __ISR (MAIN_CAN_VECTOR, IPL1SOFT) MAIN_CAN_Interrupt(void) {
    
}

void __ISR (ALT_CAN_VECTOR, IPL1SOFT) ALT_CAN_Interrupt (void) {
    
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

