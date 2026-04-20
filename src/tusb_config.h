/*
 * SPDX-License-Identifier: MIT License
 *
 * Copyright (c) 2026 Daniel Hiepler
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * This file is part of the moGPIO firmware.
 */

#ifndef TUSB_CONFIG_H
#define TUSB_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif


// Defined by compiler flags for flexibility
#ifndef CFG_TUSB_MCU
#error CFG_TUSB_MCU must be defined
#endif

// RHPort number used for device can be defined by board.mk, default to port 0
#ifndef BOARD_TUD_RHPORT
#define BOARD_TUD_RHPORT                0
#endif

// RHPort max operational speed can be defined by board.mk
// Default to Highspeed for MCU with internal HighSpeed PHY (can be port specific), otherwise FullSpeed
#ifndef BOARD_TUD_MAX_SPEED
#define BOARD_TUD_MAX_SPEED             OPT_MODE_FULL_SPEED
#endif

// Default is max speed that hardware controller could support with on-chip PHY
#define CFG_TUD_MAX_SPEED               BOARD_TUD_MAX_SPEED

// Device mode with rhport and speed defined by board.mk
#define CFG_TUSB_RHPORT0_MODE           (OPT_MODE_DEVICE | BOARD_TUD_MAX_SPEED)

#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS                     OPT_OS_NONE
#endif

// CFG_TUSB_DEBUG is defined by compiler in DEBUG build
#ifdef CFG_TUSB_DEBUG
#undef CFG_TUSB_DEBUG
#endif

// Enable device support
#define CFG_TUD_ENABLED                 1


//--------------------------------------------------------------------
// DEVICE CONFIGURATION
//--------------------------------------------------------------------

#define CFG_TUD_ENDPOINT0_SIZE          64

//------------- CLASS -------------//

// Vendor specific class configuration
#define CFG_TUD_VENDOR                  1
#define CFG_TUD_VENDOR_RX_BUFSIZE       64
#define CFG_TUD_VENDOR_TX_BUFSIZE       64


// DFU RT does not required for this project
#define CFG_TUD_DFU_RT                  0
// CDC class
#define CFG_TUD_CDC                     1
#define CFG_TUD_CDC_RX_BUFSIZE          128
#define CFG_TUD_CDC_TX_BUFSIZE          128

// MSC class for mass storage support
#define CFG_TUD_MSC                     1
#define CFG_TUD_MSC_EP_BUFSIZE          64
// HID class is not needed
#define CFG_TUD_HID                     0
// MIDI class is not needed
#define CFG_TUD_MIDI                    0
// Audio class is not needed
#define CFG_TUD_AUDIO                   0
// Video class is not needed
#define CFG_TUD_VIDEO                   0
// BTH class is not needed
#define CFG_TUD_BTH                     0



#ifdef __cplusplus
}
#endif

#endif // TUSB_CONFIG_H
