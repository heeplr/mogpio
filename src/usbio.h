
#ifndef _USBIO_H
#define _USBIO_H


// -----------------------------------------------------------------------------
// USBIO protocol
// -----------------------------------------------------------------------------

#define USBIO_PKTTYPE_CTRL              1
#define USBIO_PKTTYPE_GPIO              3
#define USBIO_PKTTYPE_I2C               4

#define USBIO_PKTFLAG_ACK               (1u << 0)
#define USBIO_PKTFLAG_RSP               (1u << 1)
#define USBIO_PKTFLAG_CMP               (1u << 2)
#define USBIO_PKTFLAG_ERR               (1u << 3)

#define USBIO_PKTFLAGS_REQRESP          (USBIO_PKTFLAG_CMP | USBIO_PKTFLAG_ACK)

#define USBIO_CTRLCMD_PROTVER           0
#define USBIO_CTRLCMD_FWVER             1
#define USBIO_CTRLCMD_HS                2
#define USBIO_CTRLCMD_ENUMGPIO          16
#define USBIO_CTRLCMD_ENUMI2C           17

#define USBIO_GPIOCMD_DEINIT            0
#define USBIO_GPIOCMD_INIT              1
#define USBIO_GPIOCMD_READ              2
#define USBIO_GPIOCMD_WRITE             3
#define USBIO_GPIOCMD_END               4

/* USBIO GPIO config */
enum usbio_gpio_pincfg {
    USBIO_GPIO_PINCFG_DEFAULT,
    USBIO_GPIO_PINCFG_PULLUP,
    USBIO_GPIO_PINCFG_PULLDOWN,
    USBIO_GPIO_PINCFG_PUSHPULL
};
#define USBIO_GPIO_PINCFG_SHIFT         2
#define USBIO_GPIO_PINCFG_MASK          (0x3 << USBIO_GPIO_PINCFG_SHIFT)
#define USBIO_GPIO_SET_PINCFG(pin)      (((pin) << USBIO_GPIO_PINCFG_SHIFT) & USBIO_GPIO_PINCFG_MASK)

enum usbio_gpio_pinmode {
    USBIO_GPIO_PINMOD_MINVAL,
    USBIO_GPIO_PINMOD_INPUT,
    USBIO_GPIO_PINMOD_OUTPUT,
    USBIO_GPIO_PINMOD_MAXVAL
};

#define USBIO_GPIO_PINMOD_MASK          0x3
#define USBIO_GPIO_SET_PINMOD(pin)      ((pin) & USBIO_GPIO_PINMOD_MASK)

#define USBIO_MAX_GPIO_PINS             HW_GPIO_MAX
#define USBIO_MAX_GPIOBANKS             5
#define USBIO_GPIOSPERBANK              32

// -----------------------------------------------------------------------------
// On-wire packet layouts
// -----------------------------------------------------------------------------

typedef struct __attribute__((packed)) {
  uint8_t type;
  uint8_t cmd;
  uint8_t flags;
} usbio_hdr_t;

typedef struct __attribute__((packed)) {
  usbio_hdr_t hdr;
  uint8_t len;
  uint8_t data[64];
} usbio_ctrl_pkt_t;

typedef struct __attribute__((packed)) {
  usbio_hdr_t hdr;
  uint16_t len;
  uint8_t data[64];
} usbio_bulk_pkt_t;


typedef struct __attribute__((packed)) {
  uint8_t ver;
} usbio_protver_t;

typedef struct __attribute__((packed)) {
  uint8_t major;
  uint8_t minor;
  uint16_t patch;
  uint16_t build;
} usbio_fwver_t;

typedef struct __attribute__((packed)) {
  uint8_t id;
  uint8_t pins;
  uint32_t bmap;
} usbio_gpio_bank_desc_t;

typedef struct __attribute__((packed)) {
  uint8_t bankid;
  uint8_t config;
  uint8_t pincount;
  uint8_t pin;
} usbio_gpio_init_t;

typedef struct __attribute__((packed)) {
  uint8_t bankid;
  uint8_t pincount;
  uint8_t pin;
  uint32_t value;
} usbio_gpio_rw_t;



#endif /* _USBIO_H */
