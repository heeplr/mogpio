#include <string.h>
#include <stdint.h>
#include "bsp/board_api.h"

#include "tusb.h"

#define USB_VID 0x06cb
#define USB_PID 0x0701

/* interface numbers */
enum {
    ITF_NUM_MSC = 0,
    ITF_NUM_VENDOR,
    ITF_NUM_TOTAL
};

/* Endpoint numbers */
#define EPNUM_MSC_OUT      0x01
#define EPNUM_MSC_IN       0x81

#define EPNUM_VENDOR_OUT   0x02
#define EPNUM_VENDOR_IN    0x82

/* string id's of usb descriptor */
enum {
    USBD_STR_LANGUAGE = 0,
    USBD_STR_MANUFACTURER,
    USBD_STR_PRODUCT,
    USBD_STR_SERIAL_NUMBER
};

#define CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_MSC_DESC_LEN + TUD_VENDOR_DESC_LEN)


/* string constants in descriptor */
static const char *string_desc_arr[] = {
    (const char[]){0x09, 0x04},
    "moGPIO",
    "USB GPIO bridge",
};
/* tmp buffer for descriptor strings */
static uint16_t _desc_str[32];

/* USB descriptor */
static const tusb_desc_device_t desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,

    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,

    .bMaxPacketSize0    = 64,

    .idVendor           = USB_VID,
    .idProduct          = USB_PID,
    .bcdDevice          = 0x0100,

    .iManufacturer      = USBD_STR_MANUFACTURER,
    .iProduct           = USBD_STR_PRODUCT,
    .iSerialNumber      = USBD_STR_SERIAL_NUMBER,

    .bNumConfigurations = 1,
};


static const uint8_t desc_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),
    TUD_MSC_DESCRIPTOR(ITF_NUM_MSC, 0, EPNUM_MSC_OUT, EPNUM_MSC_IN, 64),
    TUD_VENDOR_DESCRIPTOR(ITF_NUM_VENDOR, 0, EPNUM_VENDOR_OUT, EPNUM_VENDOR_IN, 64),
};


/******************************************************************************/

/* return our device descriptor */
uint8_t const *tud_descriptor_device_cb(void)
{
    return (uint8_t const *)&desc_device;
}

/* returns USB descriptor configuration of given index */
uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
    (void)index;
    return desc_configuration;
}

/* returns USB descriptor string of given index and language */
uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    (void)langid;

    uint8_t chr_count;

    switch(index) {
        case USBD_STR_LANGUAGE:
            memcpy(&_desc_str[1], string_desc_arr[0], 2);
            chr_count = 1;
            break;

        case USBD_STR_SERIAL_NUMBER:
            chr_count = board_usb_get_serial(_desc_str + 1, 32);
            break;

        default:
            if (!(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0]))) {
                return NULL;
            }

            const char *str = string_desc_arr[index];
            chr_count = (uint8_t)strlen(str);

            for (uint8_t i = 0; i < chr_count; i++) {
                _desc_str[1 + i] = (uint16_t)str[i];
            }
            break;
    }

    _desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2u * chr_count + 2u));
    return _desc_str;
}
