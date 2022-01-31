/*
             LUFA Library
     Copyright (C) Dean Camera, 2010.
              
  dean [at] fourwalledcubicle [dot] com
      www.fourwalledcubicle.com
*/

/*
  Copyright 2010  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this 
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in 
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting 
  documentation, and that the name of the author not be used in 
  advertising or publicity pertaining to distribution of the 
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/*-
 * Copyright (c) 2011 Darran Hunt (darran [at] hunt dot net dot nz)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file
 *
 *  Main source file for the N64-Arduino-Switch project. This file contains the main tasks of
 *  the project and is responsible for the initial application hardware configuration.
 */

#include "Joystick.h"
#include <stdlib.h>

volatile uint8_t volatile byteBuff[4];
volatile int byteBuffIdx = 0;
volatile signed char maxX = 60;
volatile signed char minX = -60;
volatile signed char maxY = 60;
volatile signed char minY = -60;


// Main entry point.
int main(void) {
    // We'll start by performing hardware and peripheral setup.
    SetupHardware();

    // We'll then enable global interrupts to read serial info (N64 controller state) from the Atmega328P
    sei();

    // Once that's done, we'll enter an infinite loop.
    for (;;) {
        // We need to run our task to process and deliver data for our IN and OUT endpoints.
        HID_Task();
        // We also need to run the main USB management task.
        USB_USBTask();
    }
}

// Configures hardware and peripherals, such as the USB peripherals.
void SetupHardware(void) {
    // We need to disable watchdog if enabled by bootloader/fuses.
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    // We need to disable clock division before initializing the USB hardware.
    clock_prescale_set(clock_div_1);
    // We can then initialize our hardware and peripherals, including the USB stack.
    // The USB stack should be initialized last.
    Serial_Init(115200, true);
    LEDs_Init();
    USB_Init();

    /* Must turn off USART before reconfiguring it, otherwise incorrect operation may occur */
    UCSR1B = 0;
    UCSR1A = 0;
    UCSR1C = 0;

    /* Special case 57600 baud for compatibility with the ATmega328 bootloader. */  
    UBRR1  = SERIAL_2X_UBBRVAL(115200);

    UCSR1C = ((1 << UCSZ11) | (1 << UCSZ10));
    UCSR1A = (1 << U2X1);
    UCSR1B = ((1 << RXCIE1) | (1 << TXEN1) | (1 << RXEN1));
}

// Fired to indicate that the device is enumerating.
void EVENT_USB_Device_Connect(void) {
    // We can indicate that we're enumerating here (via status LEDs, sound, etc.).
}

// Fired to indicate that the device is no longer connected to a host.
void EVENT_USB_Device_Disconnect(void) {
    // We can indicate that our device is not ready (via status LEDs, sound, etc.).
}

// Fired when the host set the current configuration of the USB device after enumeration.
void EVENT_USB_Device_ConfigurationChanged(void) {
    bool ConfigSuccess = true;

    // We setup the HID report endpoints.
    ConfigSuccess &= Endpoint_ConfigureEndpoint(JOYSTICK_OUT_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);
    ConfigSuccess &= Endpoint_ConfigureEndpoint(JOYSTICK_IN_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);

    // We can read ConfigSuccess to indicate a success or failure at this point.
}

// Process control requests sent to the device from the USB host.
void EVENT_USB_Device_ControlRequest(void) {
    // We can handle two control requests: a GetReport and a SetReport.

    // Not used here, it looks like we don't receive control request from the Switch.
}

// Process and deliver data from IN and OUT endpoints.
void HID_Task(void) {
    // If the device isn't connected and properly configured, we can't do anything here.
    if (USB_DeviceState != DEVICE_STATE_Configured)
        return;

    // We'll start with the OUT endpoint.
    Endpoint_SelectEndpoint(JOYSTICK_OUT_EPADDR);
    // We'll check to see if we received something on the OUT endpoint.
    if (Endpoint_IsOUTReceived()) {
        // If we did, and the packet has data, we'll react to it.
        if (Endpoint_IsReadWriteAllowed()) {
            // We'll create a place to store our data received from the host.
            USB_JoystickReport_Output_t JoystickOutputData;
            // We'll then take in that data, setting it up in our storage.
            while(Endpoint_Read_Stream_LE(&JoystickOutputData, sizeof(JoystickOutputData), NULL) != ENDPOINT_RWSTREAM_NoError);
            // At this point, we can react to this data.

            // However, since we're not doing anything with this data, we abandon it.
        }
        // Regardless of whether we reacted to the data, we acknowledge an OUT packet on this endpoint.
        Endpoint_ClearOUT();
    }

    // We'll then move on to the IN endpoint.
    Endpoint_SelectEndpoint(JOYSTICK_IN_EPADDR);
    // We first check to see if the host is ready to accept data.
    if (Endpoint_IsINReady()) {
        // We'll create an empty report.
        USB_JoystickReport_Input_t JoystickInputData;
        // We'll then populate this report with what we want to send to the host.
        GetNextReport(&JoystickInputData);
        // Once populated, we can output this data to the host. We do this by first writing the data to the control stream.
        while(Endpoint_Write_Stream_LE(&JoystickInputData, sizeof(JoystickInputData), NULL) != ENDPOINT_RWSTREAM_NoError);
        // We then send an IN packet on this endpoint.
        Endpoint_ClearIN();
    }
}

USB_JoystickReport_Input_t last_report;

// Prepare the next report for the host.
void GetNextReport(USB_JoystickReport_Input_t* const ReportData) {

    // Prepare an empty report
    memset(ReportData, 0, sizeof(USB_JoystickReport_Input_t));
    ReportData->RX = STICK_CENTER;
    ReportData->RY = STICK_CENTER;
    ReportData->HAT = HAT_CENTER;

    // A B Z Start
    ReportData->Button |= (SWITCH_A & byteBuff[0] >> 5) | (SWITCH_B & byteBuff[0] >> 5) | (SWITCH_ZL & byteBuff[0] << 1) | (SWITCH_PLUS & byteBuff[0] << 5);
    // L R
    ReportData->Button |= (SWITCH_L & byteBuff[1] >> 1) | (SWITCH_R & byteBuff[1] << 1);

    // D-pad
    switch(byteBuff[0] & 0xF) {
        case 0x8:
            ReportData->HAT = HAT_TOP;
            break;
        case 0x9:
            ReportData->HAT = HAT_TOP_RIGHT;
            break;
        case 0x1:
            ReportData->HAT = HAT_RIGHT;
            break;
        case 0x5:
            ReportData->HAT = HAT_BOTTOM_RIGHT;
            break;
        case 0x4:
            ReportData->HAT = HAT_BOTTOM;
            break;
        case 0x6:
            ReportData->HAT = HAT_BOTTOM_LEFT;
            break;
        case 0x2:
            ReportData->HAT = HAT_LEFT;
            break;
        case 0xA:
            ReportData->HAT = HAT_TOP_LEFT;
            break;
    }

    // C buttons
    switch(byteBuff[1] & 0xF) {
        case 0x8:
            ReportData->RY = STICK_MIN;
            break;
        case 0x4:
            ReportData->RY = STICK_MAX;
            break;
        case 0x2:
            ReportData->RX = STICK_MIN;
            break;
        case 0x1:
            ReportData->RX = STICK_MAX;
            break;
    }

    // scale for joystick insensitivity if needed https://n64squid.com/n64-joystick-360-degrees/
    signed char xPos = (signed char)byteBuff[2];
    maxX = xPos > maxX ? xPos : maxX;
    minX = xPos < minX ? xPos : minX;
    int scaledX = xPos > 0 ? xPos * 127 / maxX : xPos * -128 / minX;
    ReportData->LX = 128 + scaledX;

    signed char yPos = -(signed char)byteBuff[3];
    maxY = yPos > maxY ? yPos : maxY;
    minY = yPos < minY ? yPos : minY;
    int scaledY = yPos > 0 ? yPos * 127 / maxY : yPos * -128 / minY;
    ReportData->LY = 128 + scaledY;

    // Prepare to echo this report
    memcpy(&last_report, ReportData, sizeof(USB_JoystickReport_Input_t));

}

// Reading N64 controller info into a 4-byte array
ISR(USART1_RX_vect, ISR_BLOCK) {
    byteBuff[byteBuffIdx] = UDR1;
    byteBuffIdx++;
    if (byteBuffIdx > 3) {
        byteBuffIdx = 0;
    }
}
