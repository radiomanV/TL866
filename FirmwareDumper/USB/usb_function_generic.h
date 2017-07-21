/******************************************************************************
  File Information:
      FileName:       usb_function_generic.h
      Dependencies:    See INCLUDES section below
      Processor:      PIC18 or PIC24 USB Microcontrollers
      Hardware:       The code is natively intended to be used on the following
                      hardware platforms: PICDEM(TM) FS USB Demo Board,
                      PIC18F87J50 FS USB Plug-In Module, or
                      Explorer 16 + PIC24 USB PIM.  The firmware may be
                      modified for use on other USB platforms by editing the
                      HardwareProfile.h file.
      Compiler:       C18, C30, or C32
      Company:        Microchip Technology, Inc.
    
      Software License Agreement:
    
      The software supplied herewith by Microchip Technology Incorporated
      (the "Company") for its PIC(R) Microcontroller is intended and
      supplied to you, the Company's customer, for use solely and
      exclusively on Microchip PIC Microcontroller products. The
      software is owned by the Company and/or its supplier, and is
      protected under applicable copyright laws. All rights are reserved.
      Any use in violation of the foregoing restrictions may subject the
      user to criminal sanctions under applicable laws, as well as to
      civil liability for the breach of the terms and conditions of this
      license.
    
      THIS SOFTWARE IS PROVIDED IN AN "AS IS" CONDITION. NO WARRANTIES,
      WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
      TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
      PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
      IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
      CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
    
       Change History:
       Rev         Description
    
  Summary:
    This file contains all of functions, macros, definitions, variables,
    datatypes, etc. that are required for usage with vendor class function
    drivers. This file should be included in projects that use vendor class
    \function drivers. Vendor class function drivers include MCHPUSB
    (Microchip's custom class driver), WinUSB, and LibUSB. This file should also be included into the
    usb_descriptors.c file and any other user file that requires access to
    vendor class interfaces.
    
    
    
    This file is located in the "\<Install
    Directory\>\\Microchip\\Include\\USB" directory.
  Description:
    USB Vender Class Custom Driver File
    
    This file contains all of functions, macros, definitions, variables,
    datatypes, etc. that are required for usage with vendor class function
    drivers. This file should be included in projects that use vendor class
    \function drivers. This file should also be included into the
    usb_descriptors.c file and any other user file that requires access to
    vendor class interfaces.
    
    This file is located in the "\<Install
    Directory\>\\Microchip\\Include\\USB" directory.
    
    When including this file in a new project, this file can either be
    referenced from the directory in which it was installed or copied
    directly into the user application folder. If the first method is
    chosen to keep the file located in the folder in which it is installed
    then include paths need to be added so that the library and the
    application both know where to reference each others files. If the
    application folder is located in the same folder as the Microchip
    folder (like the current demo folders), then the following include
    paths need to be added to the application's project:
    
    .
    ..\\..\\Microchip\\Include
    
    If a different directory structure is used, modify the paths as
    required. An example using absolute paths instead of relative paths
    would be the following:
    
    C:\\Microchip Solutions\\Microchip\\Include
    
    C:\\Microchip Solutions\\My Demo Application                               
  ******************************************************************************/

//DOM-IGNORE-BEGIN
/********************************************************************
 File Description:

 Change History:
  Rev    Description
  ----   -----------
  2.6    No Change
  2.9h   Added prototype for USBCheckVendorRequest()
********************************************************************/
//DOM-IGNORE-END

#ifndef USBGEN_H
#define USBGEN_H

#include "GenericTypeDefs.h"
#include "usb_config.h"

/** I N C L U D E S **********************************************************/

/** D E F I N I T I O N S ****************************************************/

/** S T R U C T U R E S ******************************************************/

/** E X T E R N S ************************************************************/

/** P U B L I C  P R O T O T Y P E S *****************************************/

/********************************************************************
    Function:
        USB_HANDLE USBGenWrite(BYTE ep, BYTE* data, WORD len)
        
    Summary:
        Sends the specified data out the specified endpoint

    Description:
        This function sends the specified data out the specified 
        endpoint and returns a handle to the transfer information.

        Typical Usage:
        <code>
        //make sure that the last transfer isn't busy by checking the handle
        if(!USBHandleBusy(USBGenericInHandle))
        {
            //Send the data contained in the INPacket[] array out on
            //  endpoint USBGEN_EP_NUM
            USBGenericInHandle = USBGenWrite(USBGEN_EP_NUM,(BYTE*)&INPacket[0],sizeof(INPacket));
        }
        </code>
        
    PreCondition:
        None
        
    Parameters:
        BYTE ep    - the endpoint you want to send the data out of
        BYTE* data - pointer to the data that you wish to send
        WORD len   - the length of the data that you wish to send
        
    Return Values:
        USB_HANDLE - a handle for the transfer.  This information
        should be kept to track the status of the transfer
        
    Remarks:
        None
  
 *******************************************************************/
#define USBGenWrite(ep,data,len) USBTxOnePacket(ep,data,len)


/********************************************************************
    Function:
        USB_HANDLE USBGenRead(BYTE ep, BYTE* data, WORD len)
        
    Summary:
        Receives the specified data out the specified endpoint
        
    Description:
        Receives the specified data out the specified endpoint.

        Typical Usage:
        <code>
        //Read 64-bytes from endpoint USBGEN_EP_NUM, into the OUTPacket array.
        //  Make sure to save the return handle so that we can check it later
        //  to determine when the transfer is complete.
        if(!USBHandleBusy(USBOutHandle))
        {
            USBOutHandle = USBGenRead(USBGEN_EP_NUM,(BYTE*)&OUTPacket,64);
        }
        </code>

    PreCondition:
        None
        
    Parameters:
        BYTE ep - the endpoint you want to receive the data into
        BYTE* data - pointer to where the data will go when it arrives
        WORD len - the length of the data that you wish to receive
        
    Return Values:
        USB_HANDLE - a handle for the transfer.  This information
        should be kept to track the status of the transfer
        
    Remarks:
        None
  
 *******************************************************************/
#define USBGenRead(ep,data,len) USBRxOnePacket(ep,data,len)


/********************************************************************
	Function:
		void USBCheckVendorRequest(void)

 	Summary:
 		This routine handles vendor class specific requests that happen on EP0.
        This function should be called from the USBCBCheckOtherReq() call back
        function whenever implementing a custom/vendor class device.

 	Description:
 		This routine handles vendor specific requests that may arrive on EP0 as
 		a control transfer.  These can include, but are not necessarily 
 		limited to, requests for Microsft specific OS feature descriptor(s).  
 		This function should be called from the USBCBCheckOtherReq() call back 
 		function whenever using a vendor class device.

        Typical Usage:
        <code>
        void USBCBCheckOtherReq(void)
        {
            //Since the stack didn't handle the request I need to check
            //  my class drivers to see if it is for them
            USBCheckVendorRequest();
        }
        </code>

	PreCondition:
		None

	Parameters:
		Although this function has a void input, this handler function will
		typically need to look at the 8-byte SETUP packet contents that the
		host just sent, which may contain the vendor class specific request.
		
		Therefore, the statically allocated SetupPkt structure may be looked
		at while in the context of this function, and it will contain the most
		recently received 8-byte SETUP packet data.

	Return Values:
		None

	Remarks:
		This function normally gets called within the same context as the
		USBDeviceTasks() function, just after a new control transfer request
		from the host has arrived.  If the USB stack is operated in 
		USB_INTERRUPT mode (a usb_config.h option), then this function
		will be executed in the interrupt context.  If however the USB stack
		is operated in the USB_POLLING mode, then this function executes in the
		main loop context.
		
		In order to respond to class specific control transfer request(s) in
		this handler function, it is suggested to use one or more of the
		USBEP0SendRAMPtr(), USBEP0SendROMPtr(), or USBEP0Receive() API 
		functions.
 
 *******************************************************************/
void USBCheckVendorRequest(void);

#endif //USBGEN_H
