#include "logger.h"
#include "hal_gpio.h"
#include "msc_fs.h"


void tud_mount_cb(void) {}

void tud_umount_cb(void) {}

void tud_suspend_cb(bool remote_wakeup_en) {
    (void)remote_wakeup_en;
    INFO("suspending... remote wakeup: %d", remote_wakeup_en);
}

void tud_resume_cb(void) {
    INFO("resumed...");
}

int main(void) {
    board_init();

#ifdef HAVE_LOGGING
    stdio_init_all();
#endif

    hal_gpio_init();
    msc_fs_init();
    tusb_init();

    INFO("moGPIO initialized");

    while (1) {
        tud_task();
    }
}
