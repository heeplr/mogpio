
#include "tusb.h"
#include "hal_gpio.h"
#include "util.h"
#include "logger.h"


/* 8KB is the smallest size that windows allow to mount */
#define DISK_BLOCK_NUM   16
#define DISK_BLOCK_SIZE  512
#define ROOT_ENTRIES     16
/* first data sector */
#define DATA_LBA0        3
#define DATA_CLUSTERS    (DISK_BLOCK_NUM - DATA_LBA0)
#define EOF_CLUSTER      0x0FFF

#define DISK_BLOCK_NUM  16
#define DISK_BLOCK_SIZE 512


/* function declarations */
static bool parse_config_line(const char *line, size_t len);
static bool parse_pin_line(const char *line, size_t len);
static size_t build_config_text(char *data, size_t capacity);
static size_t build_pins_text(char *data, size_t capacity);

/* blockdev in RAM */
static char msc_disk0[DISK_BLOCK_NUM][DISK_BLOCK_SIZE] = {
  //------------- Block0: Boot Sector -------------//
  // byte_per_sector    = DISK_BLOCK_SIZE; fat12_sector_num_16  = DISK_BLOCK_NUM;
  // sector_per_cluster = 1; reserved_sectors = 1;
  // fat_num            = 1; fat12_root_entry_num = 16;
  // sector_per_fat     = 1; sector_per_track = 1; head_num = 1; hidden_sectors = 0;
  // drive_number       = 0x80; media_type = 0xf8; extended_boot_signature = 0x29;
  // filesystem_type    = "FAT12   "; volume_serial_number = 0x1234; volume_label = "TinyUSB 0  ";
  // FAT magic code at offset 510-511
  {
      0xEB, 0x3C, 0x90, 0x4D, 0x53, 0x44, 0x4F, 0x53, 0x35, 0x2E, 0x30, 0x00, 0x02, 0x01, 0x01, 0x00,
      0x01, 0x10, 0x00, 0x10, 0x00, 0xF8, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x29, 0x34, 0x12, 0x00, 0x00, 'm' , 'o' , 'G' , 'P' , 'I' ,
      'O' , ' ' , ' ' , ' ' , ' ' , ' ' , 0x46, 0x41, 0x54, 0x31, 0x32, 0x20, 0x20, 0x20, 0x00, 0x00,

      // Zero up to 2 last bytes of FAT magic code
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0xAA
  },

  //------------- Block1: FAT12 Table -------------//
  {
      0xF8, 0xFF, 0xFF, 0xFF, 0x0F // // first 2 entries must be F8FF, third entry is cluster end of readme file
  },

  //------------- Block2: Root Directory -------------//
  {
      // first entry is volume label
      'm' , 'o' , 'G' , 'P' , 'I' , 'O' , ' ' , ' ' , ' ' , ' ' , ' ' , 0x08, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4F, 0x6D, 0x65, 0x43, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      // second entry is readme file
      'R' , 'E' , 'A' , 'D' , 'M' , 'E' , ' ' , ' ' , 'T' , 'X' , 'T' , 0x20, 0x00, 0xC6, 0x52, 0x6D,
      0x65, 0x43, 0x65, 0x43, 0x00, 0x00, 0x88, 0x6D, 0x65, 0x43, 0x02, 0x00,
      0x01, 0x00, 0x00, 0x00 // readme's files size (4 Bytes)
  }
};

/* function that parses one line of a textfile */
typedef bool (lineparser_t)(const char *line, size_t len);
/* function to generate a textfile */
typedef size_t (generator_t)(char *data, size_t capacity);

/* a virtual file */
typedef struct {
  const char*       name;     // 8.3 format
  uint32_t          size;
  uint16_t          first_cluster;
  uint16_t          cluster_count;
  uint32_t          data_lba0;  // first LBA of this file's data region
  bool              dirty;
  lineparser_t *    parser;
  generator_t *     generator;
} vfile_t;

/* IDs of the file we provide */
enum {
    FILE_PINS_ID,
    FILE_CONFIG_ID
};

/* the files we provide */
static vfile_t files[] = {
  { "PINS    TXT", 0, 0, 0, 0, false, &parse_pin_line, &build_pins_text },
  { "CONFIG  TXT", 0, 0, 0, 0, false, &parse_config_line, &build_config_text },
};

#define FILE_COUNT (sizeof(files)/sizeof(files[0]))

/* temp buffer for files */
static char file_cache[FILE_COUNT][DATA_CLUSTERS * DISK_BLOCK_SIZE];
/* flag that gets set upon initial parse of CONFIG.TXT (which is
 * generated with default values) to initialize the HAL GPIO pins */
static bool hal_pins_configured = false;


/******************************************************************************/

static inline void put_le16(char *p, uint16_t v) {
  p[0] = (char)(v & 0xFF);
  p[1] = (char)(v >> 8);
}

static inline void put_le32(char *p, uint32_t v) {
  p[0] = (char)(v & 0xFF);
  p[1] = (char)((v >> 8) & 0xFF);
  p[2] = (char)((v >> 16) & 0xFF);
  p[3] = (char)((v >> 24) & 0xFF);
}

static inline uint16_t get_le16(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static inline uint32_t get_le32(const uint8_t *p) {
    return (uint32_t)p[0]
         | ((uint32_t)p[1] << 8)
         | ((uint32_t)p[2] << 16)
         | ((uint32_t)p[3] << 24);
}

/* generate CONFIG.TXT */
static size_t build_config_text(char *data, size_t capacity) {
    size_t used = 0;

    unsigned int bankcount = hal_gpio_bankcount();
    for (uint8_t bankid = 0; bankid < bankcount; ++bankid) {
        unsigned int pincount = hal_gpio_bank_pincount(bankid);
        for (unsigned int pin = 0; pin < pincount; ++pin) {

            const char *func_str = hal_gpio_function_name(HAL_GPIO_FN_NOCHANGE);
            hal_gpio_function_t function;
            if (hal_gpio_get_function(bankid, pin, &function) < 0) {
                used = append_fmt(data, capacity, used, "%u:%u=?\r\n", bankid, pin);
                continue;
            }

            switch (function) {
                case HAL_GPIO_FN_INPUT:
                    func_str = hal_gpio_function_name(HAL_GPIO_FN_INPUT);
                    break;
                case HAL_GPIO_FN_OUTPUT:
                    func_str = hal_gpio_function_name(HAL_GPIO_FN_OUTPUT);
                    break;
                case HAL_GPIO_FN_NONE:
                default:
                    func_str = hal_gpio_function_name(HAL_GPIO_FN_NOCHANGE);
                    break;
            }

            const char *mode_str = hal_gpio_mode_name(HAL_GPIO_MODE_PUSHPULL);
            hal_gpio_mode_t mode;
            if (hal_gpio_get_mode(bankid, pin, &mode) < 0) {
                used = append_fmt(data, capacity, used, "%u:%u=?\r\n", bankid, pin);
                continue;
            }

            switch (mode) {
                case HAL_GPIO_MODE_PULL_DOWN:
                    mode_str = hal_gpio_mode_name(HAL_GPIO_MODE_PULL_DOWN);
                    break;
                case HAL_GPIO_MODE_PULL_UP:
                    mode_str = hal_gpio_mode_name(HAL_GPIO_MODE_PULL_UP);
                    break;
                case HAL_GPIO_MODE_PUSHPULL:
                    mode_str = hal_gpio_mode_name(HAL_GPIO_MODE_PUSHPULL);
                    break;
                default:
                    mode_str = hal_gpio_mode_name(HAL_GPIO_MODE_NOCHANGE);
                    break;
            }

            used = append_fmt(data, capacity, used, "%u:%u=%s,%s\r\n", bankid, pin, func_str, mode_str);
            if (used == SIZE_MAX) {
                return 0;
            }
        }
    }

    if (used < capacity) {
        data[used] = '\0';
    }
    return used;
}

/* generate PINS.TXT */
static size_t build_pins_text(char *data, size_t capacity) {
    size_t used = 0;

    unsigned int bankcount = hal_gpio_bankcount();
    for (uint8_t bankid = 0; bankid < bankcount; ++bankid) {
        unsigned int pincount = hal_gpio_bank_pincount(bankid);
        for (unsigned int pin = 0; pin < pincount; ++pin) {
            bool value = false;
            int res = hal_gpio_read(bankid, pin, &value);
            if (res != HAL_GPIO_OK) {
                ERROR("gpio_read error: %d", res);
                used = append_fmt(data, capacity, used, "%u:%u=?\r\n", bankid, pin);
            } else {
                used = append_fmt(data, capacity, used, "%u:%u=%u\r\n", bankid, pin, value ? 1u : 0u);
            }
            if (used == SIZE_MAX) {
                return 0;
            }
        }
    }

    if (used < capacity) {
        data[used] = '\0';
    }
    return used;
}

/* parse one line of CONFIG.TXT */
static bool parse_config_line(const char *line, size_t len) {
    char buf[128];
    if (len == 0) {
        return true;
    }
    if (len >= sizeof(buf)) {
        return false;
    }

    memcpy(buf, line, len);
    buf[len] = '\0';

    char *sep = strchr(buf, ':');
    if (!sep) {
        return false;
    }
    *sep = '\0';

    char *eq = strchr(sep + 1, '=');
    if (!eq) {
        return false;
    }
    *eq = '\0';

    char *bank = buf;
    char *lhs = sep + 1;
    char *rhs = eq + 1;

    bank = (char *)skip_ws(bank, bank + strlen(bank));
    rtrim_ws(bank);
    lhs = (char *)skip_ws(lhs, lhs + strlen(lhs));
    rtrim_ws(lhs);
    rhs = (char *)skip_ws(rhs, rhs + strlen(rhs));
    rtrim_ws(rhs);

    if (*bank == '\0' || *lhs == '\0' || *rhs == '\0') {
        return false;
    }

    uint32_t bankid = 0;
    if (!parse_u32(bank, &bankid) || bankid >= hal_gpio_bankcount()) {
        return false;
    }

    uint32_t pin = 0;
    if (!parse_u32(lhs, &pin) || pin >= hal_gpio_bank_pincount((uint8_t)bankid)) {
        return false;
    }

    hal_gpio_mode_t mode = HAL_GPIO_MODE_NOCHANGE;
    char *comma = strchr(rhs, ',');
    if (comma) {
        *comma = '\0';
        char *pull_s = comma + 1;
        pull_s = (char *)skip_ws(pull_s, pull_s + strlen(pull_s));
        rtrim_ws(pull_s);
        if (*pull_s == '\0' || !parse_mode(pull_s, &mode)) {
            return false;
        }
    }

    hal_gpio_function_t function = HAL_GPIO_FN_NOCHANGE;
    rhs = (char *)skip_ws(rhs, rhs + strlen(rhs));
    rtrim_ws(rhs);
    if (!parse_function(rhs, &function)) {
        return false;
    }

    INFO("configuring bank: %lu, pin: %lu, func: %d, mode: %d",
         (unsigned long)bankid, (unsigned long)pin, function, mode);
    hal_gpio_pin_config((uint8_t)bankid, (uint8_t)pin, function, mode);

    return true;
}

/* parse one line of PINS.TXT formatted "bank:pin=value" */
static bool parse_pin_line(const char *line, size_t len) {
    if (len == 0) {
        return true;
    }

    char buf[16];
    if (len >= sizeof(buf)) {
        ERROR("buffer overflow");
        return false;
    }

    /* create working copy */
    memcpy(buf, line, len);
    buf[len] = '\0';

    /* find separators and \0 terminate */
    char *sep = strchr(buf, ':');
    if (!sep) {
        ERROR("no : separator found");
        return false;
    }
    *sep = '\0';

    char *eq = strchr(sep + 1, '=');
    if (!eq) {
        ERROR("no = separator found");
        return false;
    }
    *eq = '\0';

    /* our found tokens */
    char *bank = buf;
    char *pin = sep + 1;
    char *val = eq + 1;

    /* trim whitespace */
    bank = (char *) skip_ws(bank, bank + strlen(bank));
    rtrim_ws(bank);
    pin = (char *) skip_ws(pin, pin + strlen(pin));
    rtrim_ws(pin);
    val = (char *) skip_ws(val, val + strlen(val));
    rtrim_ws(val);

    /* validate values */
    if (*bank == '\0' || *pin == '\0' || *val == '\0') {
        ERROR("empty bank/pin/value");
        return false;
    }

    /* parse values */
    uint32_t bankid = 0;
    if (!parse_u32(bank, &bankid) || bankid >= hal_gpio_bankcount()) {
        ERROR("failed to parse bank: %s", bank);
        return false;
    }

    uint32_t pinid = 0;
    if (!parse_u32(pin, &pinid) || pinid >= hal_gpio_bank_pincount((uint8_t)bankid)) {
        ERROR("failed to parse pin: %s", pin);
        return false;
    }

    if (!(val[0] == '0' || val[0] == '1') || val[1] != '\0') {
        ERROR("failed to parse value: %s", val);
        return false;
    }
    bool value = (val[0] == '1');

    /* is pin output? */
    hal_gpio_function_t func;
    hal_gpio_get_function(bankid, pinid, &func);
    if(func != HAL_GPIO_FN_OUTPUT) {
        /* silently ignore */
        return true;
    }

    INFO("setting GPIO bank: %lu, pin: %lu, value: %d",
         (unsigned long) bankid, (unsigned long) pinid, value);

    int res = hal_gpio_write((uint8_t)bankid, (uint8_t)pinid, value);
    if(res < 0) {
        ERROR("hal_gpio_write() error: %d", res);
    }

    return true;
}

static bool parse_file(const char *data, size_t capacity, lineparser_t *parser) {
    const char *p = data;
    const char *end = data + capacity;
    while (p < end) {
        const char *line_start = p;
        while (p < end && *p != '\n' && *p != '\r') {
            ++p;
        }

        size_t line_len = (size_t)(p - line_start);

        if (p < end && *p == '\r') {
            ++p;
        }
        if (p < end && *p == '\n') {
            ++p;
        }

        if (line_len == 0) {
            continue;
        }

        parser(line_start, line_len);

    }
    return true;
}

// FAT12 entry n starts at floor(3*n/2)
static void fat12_set_entry(char *fat, uint16_t cluster, uint16_t value) {
  uint32_t off = cluster + (cluster >> 1); // floor(3*cluster/2)

  value &= 0x0FFF;

  if ((cluster & 1) == 0) {
    // even cluster: low 8 bits at off, high 4 bits in low nibble of off+1
    fat[off]     = (char)(value & 0xFF);
    fat[off + 1]  = (fat[off + 1] & 0xF0) | (char)((value >> 8) & 0x0F);
  } else {
    // odd cluster: low 4 bits in high nibble of off, high 8 bits at off+1
    fat[off]     = (fat[off] & 0x0F) | (char)((value << 4) & 0xF0);
    fat[off + 1]  = (char)((value >> 4) & 0xFF);
  }
}

/* initialize boot sector of FAT12 fs */
static void build_boot_sector(char *bs, uint16_t volume_serial, const char volume_label[11]) {
  memset(bs, 0, DISK_BLOCK_SIZE);

  bs[0] = 0xEB; bs[1] = 0x3C; bs[2] = 0x90;
  memcpy(bs + 3, "MSDOS5.0", 8);

  put_le16(bs + 11, DISK_BLOCK_SIZE); // bytes/sector
  bs[13] = 1;                         // sectors/cluster
  put_le16(bs + 14, 1);               // reserved sectors
  bs[16] = 1;                         // FATs
  put_le16(bs + 17, ROOT_ENTRIES);    // root entries
  put_le16(bs + 19, DISK_BLOCK_NUM);  // total sectors (16-bit)
  bs[21] = 0xF8;                      // media descriptor
  put_le16(bs + 22, 1);               // sectors/FAT
  put_le16(bs + 24, 1);               // sectors/track
  put_le16(bs + 26, 1);               // heads
  put_le32(bs + 28, 0);               // hidden sectors

  bs[36] = 0x80;                      // drive number
  bs[38] = 0x29;                      // extended boot signature
  put_le32(bs + 39, volume_serial);
  memcpy(bs + 43, volume_label, 11);
  memcpy(bs + 54, "FAT12   ", 8);

  bs[510] = 0x55;
  bs[511] = 0xAA;
}

/* refresh file_cache contents from current FAT12 */
static void sync_from_fatfs(void) {
    uint8_t const *root = (uint8_t const *)msc_disk0[2];

    for (uint32_t fid = 0; fid < FILE_COUNT; ++fid) {
        uint8_t const *e = root + 32 * (fid + 1); // skip volume label

        uint16_t first_cluster = get_le16(e + 26);
        uint32_t size = get_le32(e + 28);

        /* check for changes */
        vfile_t *f = &files[fid];
        if(first_cluster != f->first_cluster || size != f->size) {
            f->dirty = true;
        }
        /* update size and position */
        f->size = size;
        f->first_cluster = first_cluster;

        if (size == 0 || first_cluster < 2) {
            f->cluster_count = 0;
            f->data_lba0 = 0;
        } else {
            f->cluster_count = (size + DISK_BLOCK_SIZE - 1) / DISK_BLOCK_SIZE;
            f->data_lba0 = DATA_LBA0 + (first_cluster - 2);
        }
        /* read data into cache from disk image */
        for (uint16_t c = 0; c < f->cluster_count; ++c) {

            uint32_t src_off = (uint32_t)c * DISK_BLOCK_SIZE;
            uint32_t dst_lba  =  f->data_lba0 + c;
            uint32_t copy_len = f->size - src_off;

            if (copy_len > DISK_BLOCK_SIZE)
                copy_len = DISK_BLOCK_SIZE;

            memcpy(file_cache[fid] + src_off, msc_disk0[dst_lba], copy_len);
        }

    }
}

/* refresh FAT12 for all files according to current file_cache contents */
void sync_to_fatfs() {
    char *fat = msc_disk0[1];
    char *root = msc_disk0[2];

    uint16_t next_cluster = 2;

    for (uint32_t fid = 0; fid < FILE_COUNT; ++fid) {
        vfile_t *f = &files[fid];

        DEBUG("fid=%lu cluster=%u lba0=%lu", fid, f->first_cluster, f->data_lba0);

        /* filesize */
        f->size = (uint32_t)strlen(file_cache[fid]);
        f->cluster_count = 0;
        f->first_cluster = next_cluster;
        f->data_lba0 = 0;

        if (f->size != 0) {
            /* at least 1 if file is non-empty */
            f->cluster_count = (f->size + DISK_BLOCK_SIZE - 1) / DISK_BLOCK_SIZE;
            f->first_cluster = next_cluster;
            f->data_lba0 = DATA_LBA0 + (f->first_cluster - 2);
            next_cluster += f->cluster_count;
        }

        // Hard limit imposed by this image geometry
        if (next_cluster > (uint16_t)(2 + DATA_CLUSTERS)) {
            // You can assert here, or truncate, or refuse to mount.
            // For safety in production, do not build an invalid image.
            while (1) { /* cluster exhaustion */ }
        }

        // Build FAT chain
        for (uint16_t c = 0; c < f->cluster_count; ++c) {
            uint16_t cluster = (uint16_t)(f->first_cluster + c);
            uint16_t val = (c + 1 < f->cluster_count) ? (uint16_t)(cluster + 1) : EOF_CLUSTER;
            fat12_set_entry(fat, cluster, val);
        }

        // Root directory entry: one per file, after the volume label
        char *e = root + 32 * (fid + 1);
        memcpy(e + 0, f->name, 11);
        e[11] = 0x20; // archive

        // NT reserved, creation time, date, last access, write time/date can be zeroed
        put_le16(e + 26, f->first_cluster);
        put_le32(e + 28, f->size);

        // Preload data sectors into the disk image
        for (uint16_t c = 0; c < f->cluster_count; ++c) {
            uint32_t src_off = (uint32_t)c * DISK_BLOCK_SIZE;
            uint32_t dst_lba  =  f->data_lba0 + c;
            uint32_t copy_len = f->size - src_off;
            if (copy_len > DISK_BLOCK_SIZE) copy_len = DISK_BLOCK_SIZE;
            memcpy(msc_disk0[dst_lba], file_cache[fid] + src_off, copy_len);
        }

        /* FAT has file now */
        f->dirty = false;
    }
}

/* helper to determine which file was accessed */
static int find_file_by_lba(uint32_t lba)
{
  if (lba < DATA_LBA0 || lba >= DISK_BLOCK_NUM) {
    return -1;
  }

  for (uint32_t i = 0; i < FILE_COUNT; ++i) {
    if (files[i].cluster_count == 0) {
      continue;
    }

    uint32_t start = files[i].data_lba0;
    uint32_t end   = start + files[i].cluster_count;

    if (lba >= start && lba < end) {
      return (int)i;
    }
  }

  return -1;
}

void msc_fs_init(void) {
    memset(msc_disk0, 0, sizeof(msc_disk0));
    memset(file_cache, 0, sizeof(file_cache));

    build_boot_sector(msc_disk0[0], 0x1234, "moGPIO     ");

    // ---- FAT ----
    char* fat = msc_disk0[1];
    fat[0] = 0xF8;
    fat[1] = 0xFF;
    fat[2] = 0xFF;

    char *root = msc_disk0[2];
    memset(root, 0, DISK_BLOCK_SIZE);

    // Volume label entry at root[0]
    memcpy(root + 0, "moGPIO     ", 11);
    root[11] = 0x08; // volume label attribute

    /* create files in cache initially */
    for (uint32_t fid = 0; fid < FILE_COUNT; ++fid) {
        files[fid].size = files[fid].generator(file_cache[fid],
                                               sizeof(file_cache[fid]));
    }

    /* refresh fat with initial file contents */
    sync_to_fatfs();

}

/**
 * TinyUSB callbacks
 */

/* handle arbitrary SCSI commands not handled by TinyUSB */
int32_t tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16], void* buffer, uint16_t bufsize) {
    (void) buffer;
    (void) bufsize;

    switch (scsi_cmd[0]) {
        default:
            tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);
            return -1;
    }
    return 0;
}

/* determine max LUN */
uint8_t tud_msc_get_maxlun_cb(void) {
  return 0;
}

/* handle SCSI_CMD_INQUIRY, v2 with full inquiry response */
uint32_t tud_msc_inquiry2_cb(uint8_t lun, scsi_inquiry_resp_t *inquiry_resp, uint32_t bufsize) {
  (void) lun;
  (void) bufsize;
  const char vid[] = "moGPIO";
  const char pid[] = "virtFS";
  const char rev[] = "1.0";

  strncpy((char*) inquiry_resp->vendor_id, vid, 8);
  strncpy((char*) inquiry_resp->product_id, pid, 16);
  strncpy((char*) inquiry_resp->product_rev, rev, 4);

  return sizeof(scsi_inquiry_resp_t); // 36 bytes
}

/* handle Test Unit Ready command
   return true allowing host to read/write this LUN e.g SD card inserted */
bool tud_msc_test_unit_ready_cb(uint8_t lun) {
    /* we only have one LUN and we are ready &writable */
    return (lun == 0);
}

/* we are always writable */
bool tud_msc_is_writable_cb(uint8_t lun) {
    (void) lun;
    return true;
}

/* handle SCSI_CMD_READ_CAPACITY_10 and SCSI_CMD_READ_FORMAT_CAPACITY
   to determine the disk size */
void tud_msc_capacity_cb(uint8_t lun, uint32_t* block_count, uint16_t* block_size) {
    (void) lun;

    *block_count = DISK_BLOCK_NUM;
    *block_size  = DISK_BLOCK_SIZE;
}

/* handle start/stop Unit command
   - start = 0 : stopped power mode, if load_eject = 1 : unload disk storage
   - start = 1 : active mode, if load_eject = 1 : load disk storage */
bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject) {
    (void) lun;
    (void) power_condition;
    (void) start;
    (void) load_eject;
    return true;
}

/* handle READ10 command - copy ramdisk's data to buffer (up to bufsize) and
   return number of bytes */
int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba,
                         uint32_t offset, void* buffer,
                         uint32_t bufsize)
{
    (void) lun;

    if (!hal_pins_configured) {
        /* parse our config once to initialize HAL pins */
        parse_file(file_cache[FILE_CONFIG_ID], files[FILE_CONFIG_ID].size, files[FILE_CONFIG_ID].parser);
        hal_pins_configured = true;
    }

    if (lba >= DISK_BLOCK_NUM)
        return -1;

    int fid = find_file_by_lba(lba);
    DEBUG("lun: %d lba: %ld offset: %ld bufsize: %ld fid: %d",
        lun, lba, offset, bufsize, fid);

    /* upon first block of a READ10 session */
    if (fid >= 0 && offset == 0) {
        /* build complete file in cache */
        files[fid].generator(file_cache[fid], sizeof(file_cache[fid]));
        /* update ramdisk */
        sync_to_fatfs();
    }

    /* read from ramdisk */
    memcpy(buffer, msc_disk0[lba] + offset, bufsize);
    return (int32_t) bufsize;
}

/* handle WRITE10 commands - copy buffer's data to ramdisk (up to bufsize) and
 * return number of bytes
 */
int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba,
                          uint32_t offset, uint8_t* buffer,
                          uint32_t bufsize)
{
    (void) lun;

    if (lba >= DISK_BLOCK_NUM) {
        return -1;
    }

    DEBUG("lun: %d lba: %ld offset: %ld bufsize: %ld",
        lun, lba, offset, bufsize);

    /* copy to ramdisk */
    memcpy(msc_disk0[lba] + offset, buffer, bufsize);

    return (int32_t)bufsize;
}

/* called after last WRITE10 completed */
void tud_msc_write10_complete_cb(uint8_t lun) {
    (void) lun;

    /* load files from freshly modified ramdisk */
    sync_from_fatfs();

    for (uint32_t fid = 0; fid < FILE_COUNT; ++fid) {
        /* process file contents if they changed */
        if(files[fid].dirty) {
            DEBUG("parsing %s", files[fid].name);
            parse_file(file_cache[fid], files[fid].size, files[fid].parser);
            files[fid].dirty = false;
        }
    }
}
