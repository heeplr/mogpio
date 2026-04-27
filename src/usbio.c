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

#include "hardware/gpio.h"

#include "usbio.h"
#include "hal_gpio.h"
#include "logger.h"

/* create 32 bit pinmap with available pins' bit set */
static uint32_t pin_bmap(uint8_t bankid)
{
    uint32_t map = 0;
    for(uint8_t n=0; n < hal_gpio_bank_pincount((size_t) bankid); n++) {
        map |= 1 << n;
    }
    return map;
}

/*
===========================================================
GPIO helper
===========================================================
*/
static int apply_gpio_config(uint8_t bankid, uint8_t pin, uint8_t cfg)
{
    uint8_t mode = cfg & USBIO_GPIO_PINMOD_MASK;
    uint8_t pull = (cfg >> USBIO_GPIO_PINCFG_SHIFT) & 0x3;
    hal_gpio_function_t f = HAL_GPIO_FN_NOCHANGE;
    hal_gpio_mode_t m = HAL_GPIO_MODE_NOCHANGE;

    switch (mode) {
        case USBIO_GPIO_PINMOD_INPUT:
            INFO("pin %d -> INPUT", pin);
            f = HAL_GPIO_FN_INPUT;
            break;

        case USBIO_GPIO_PINMOD_OUTPUT:
            INFO("pin %d -> OUTPUT", pin);
            f = HAL_GPIO_FN_OUTPUT;
            break;

        /* no change */
        default:
            ERROR("unknown pin mode: %d", mode);
            f = HAL_GPIO_FN_NOCHANGE;
            break;
    }

    int rc = hal_gpio_set_function((size_t) bankid, (size_t) pin, f);
    if(rc != HAL_GPIO_OK) {
        return rc;
    }

    switch (pull) {
        case USBIO_GPIO_PINCFG_PULLDOWN:
            m = HAL_GPIO_MODE_PULL_DOWN;
            break;

        case USBIO_GPIO_PINCFG_PULLUP:
            m = HAL_GPIO_MODE_PULL_UP;
            break;

        case USBIO_GPIO_PINCFG_PUSHPULL:
            m = HAL_GPIO_MODE_PUSHPULL;
            break;

        case USBIO_GPIO_PINCFG_DEFAULT:
        default:
            m = HAL_GPIO_MODE_NOCHANGE;
            break;
    }

    return hal_gpio_set_mode((size_t) bankid, (size_t) pin, m);
}


/*
===========================================================
CTRL COMMAND HANDLER
===========================================================
*/
static bool handle_ctrl(uint8_t cmd, uint8_t *buf, uint16_t *answer_len)
{
    switch(cmd)
    {
        case USBIO_CTRLCMD_HS:
            INFO("CTRL: HS");
            *answer_len = 0;
            return true;

        case USBIO_CTRLCMD_PROTVER:
            INFO("CTRL: PROTVER");
            buf[0] = 1;
            *answer_len = 1;
            return true;

        case USBIO_CTRLCMD_FWVER:
            INFO("CTRL: FWVER");
            buf[0]=1;
            buf[1]=0;
            buf[2]=0;
            buf[3]=0;
            buf[4]=1;
            buf[5]=0;
            *answer_len = 6;
            return true;

        case USBIO_CTRLCMD_ENUMGPIO:
        {
            INFO("CTRL: ENUMGPIO");
            typedef struct {
                uint8_t id;
                uint8_t pins;
                uint32_t bmap;
            } __attribute__((packed)) bank;

            bank banks[USBIO_MAX_GPIOBANKS];

            for(uint8_t id=0;
                    (size_t) id < hal_gpio_bankcount() && id < USBIO_MAX_GPIOBANKS;
                    id++) {
                banks[id].id = id;
                banks[id].pins = hal_gpio_bank_pincount((size_t) id);
                banks[id].bmap = pin_bmap(id);
            };

            memcpy(buf, &banks, sizeof(bank) * hal_gpio_bankcount());
            *answer_len = sizeof(bank) * hal_gpio_bankcount();
            return true;
        }

        case USBIO_CTRLCMD_ENUMI2C:
            INFO("CTRL: ENUMI2C");
            *answer_len = 0;
            return true;

        default:
            ERROR("CTRL: unknown cmd: %d", cmd);
            break;
    }

    return false;
}

/*
===========================================================
GPIO COMMAND HANDLER
===========================================================
*/
static void handle_gpio(usbio_ctrl_pkt_t *req, usbio_ctrl_pkt_t *resp)
{
    resp->hdr.type  = USBIO_PKTTYPE_GPIO;
    resp->hdr.cmd   = req->hdr.cmd;

    switch(req->hdr.cmd)
    {
        case USBIO_GPIOCMD_INIT:
        {
            usbio_gpio_init_t *p = (void*) req->data;

            // no answer requested
            resp->len = 0;
            resp->hdr.flags = USBIO_PKTFLAG_CMP;

            INFO(
                "GPIO: INIT bank: %d pin: %d config: %d",
                p->bankid, p->pin, p->config
            );
            if(apply_gpio_config(p->bankid, p->pin, p->config) != HAL_GPIO_OK)
                goto _hp_error;

            break;
        }

        case USBIO_GPIOCMD_READ:
        {
            usbio_gpio_rw_t *in = (void*) req->data;
            usbio_gpio_rw_t out = { 0 };

            INFO("GPIO: READ bank: %d pin: %d", in->bankid, in->pin);

            /* read value of that pin */
            bool value;
            int ret;
            ret = hal_gpio_read((size_t) in->bankid, (size_t) in->pin, &value);

            /* build answer */
            out.value = (value ? 1u : 0u) << in->pin;
            out.bankid   = in->bankid;
            out.pincount = in->pincount;
            out.pin      = in->pin;
            memcpy(resp->data, &out, sizeof(out));
            resp->hdr.flags = USBIO_PKTFLAG_RSP;
            resp->len = sizeof(out);

            if(ret != HAL_GPIO_OK) {
                ERROR("GPIO: READ error: %d", ret);
                goto _hp_error;
            }

            break;
        }

        case USBIO_GPIOCMD_WRITE:
        {
            usbio_gpio_rw_t *in = (void*) req->data;

            // no answer requested
            resp->hdr.flags = USBIO_PKTFLAG_CMP;
            resp->len = 0;

            uint32_t mask = 1u << in->pin;
            bool high = (in->value & mask) != 0;
            INFO("GPIO: WRITE  bank: %d pin: %d -> %d", in->bankid, in->pin, high);
            if(hal_gpio_write((size_t) in->bankid, (size_t) in->pin, high) != HAL_GPIO_OK)
                goto _hp_error;
            break;
        }

        default:
            ERROR("GPIO: unknown cmd: %d", req->hdr.cmd);
            goto _hp_error;
    }

    return;

_hp_error:
    /* mark error */
    resp->hdr.flags |= USBIO_PKTFLAG_ERR;
    return;
}


/*******************************************************************************
 *  TinyUSB callbacks
 */
/* Invoked when received control request with VENDOR TYPE */
bool tud_vendor_control_xfer_cb(uint8_t rhport,
                               uint8_t stage,
                               const tusb_control_request_t *request)
{
    /* Control transfer buffers */
    static usbio_ctrl_pkt_t ctrl_req;
    static usbio_ctrl_pkt_t ctrl_resp;

    // Used to test the direction
    bool dir_in = (request->bmRequestType_bit.direction == TUSB_DIR_IN) ? true : false;

    INFO("Control transfer: if=0x%02x stage=%d req=0x%02x type=0x%02x dir=%s wValue=0x%04x wIndex=0x%04x wLength=%d",
        request->wIndex,
        stage,
        request->bRequest,
        request->bmRequestType_bit.type,
        dir_in ? "IN" : "OUT",
        request->wValue,
        request->wIndex,
        request->wLength);

    if (request->bmRequestType_bit.type != TUSB_REQ_TYPE_VENDOR) {
        ERROR("type != VENDOR received: %d", request->bmRequestType_bit.type);
        return false;
    }

    /*
    ============================
    HOST -> DEVICE (OUT)
    ============================
    */
    if (!dir_in)
    {
        switch(stage) {

            /* Stage 1: SETUP -> prepare buffer */
            case CONTROL_STAGE_SETUP:
                INFO("OUT: CONTROL_STAGE_SETUP");
                return tud_control_xfer(rhport, request, &ctrl_req, request->wLength);

            /* Stage 2: DATA -> receive */
            case CONTROL_STAGE_DATA:
                INFO("OUT: CONTROL_STAGE_DATA (type: %d, cmd: %d, flags: %d)", ctrl_req.hdr.type, ctrl_req.hdr.cmd, ctrl_req.hdr.flags);

                uint16_t len = 0;
                memset(&ctrl_resp, 0, sizeof(ctrl_resp));

                switch(ctrl_req.hdr.type) {

                    case USBIO_PKTTYPE_CTRL:
                        INFO("OUT: CONTROL_STAGE_SETUP: CTRL packet");
                        if(!handle_ctrl(ctrl_req.hdr.cmd, ctrl_resp.data, &len))
                            return false;

                        ctrl_resp.hdr.type  = ctrl_req.hdr.type;
                        ctrl_resp.hdr.cmd   = ctrl_req.hdr.cmd;
                        ctrl_resp.hdr.flags = USBIO_PKTFLAG_RSP;
                        ctrl_resp.len = (uint8_t) len;
                        break;

                    case USBIO_PKTTYPE_GPIO:
                        INFO("OUT: CONTROL_STAGE_SETUP: GPIO packet");
                        handle_gpio(&ctrl_req, &ctrl_resp);
                        break;

                    default:
                        INFO("OUT: CONTROL_STAGE_DATA: unknown packet type: %d", ctrl_req.hdr.type);
                        return false;
                }

                //~ /* COMPLETE CONTROL TRANSFER */
                //return tud_control_status(rhport, request);
                return true;

            /* Stage 2: ACK -> DATA received, now process */
            case CONTROL_STAGE_ACK:
                INFO("OUT: CONTROL_STAGE_ACK (type: %d, cmd: %d, flags: %d)", ctrl_req.hdr.type, ctrl_req.hdr.cmd, ctrl_req.hdr.flags);
                return true;

            default:
                ERROR("OUT: unknown stage: %d", stage);
                return false;
        }
    }

    /*
    ============================
    DEVICE -> HOST (IN)
    ============================
    */
    else if (request->bmRequestType_bit.direction == TUSB_DIR_IN) {

        switch (stage) {

            case CONTROL_STAGE_SETUP:
                INFO("IN: CONTROL_STAGE_SETUP");
                /* no answer? */
                if(!(ctrl_resp.hdr.flags & USBIO_PKTFLAG_RSP)) {
                    return false;
                }
                return tud_control_xfer(rhport,
                                        request,
                                        &ctrl_resp,
                                        sizeof(usbio_hdr_t) + 1 + ctrl_resp.len);

            case CONTROL_STAGE_DATA:
                INFO("IN: CONTROL_STAGE_DATA");
                return true;

            case CONTROL_STAGE_ACK:
                INFO("IN: CONTROL_STAGE_ACK");
                return true;

            default:
                ERROR("IN: unknown stage: %d", stage);
                return false;

        }
    }

    return false;
}
