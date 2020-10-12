#include <driver/pit.h>
#include <sys/system.h>
#include <kprintf.h>

#define TIMER0 0x40
#define TIMER1 0x41
#define TIMER2 0x42
#define TIMER_CTL 0x43

#define DEFAULTHZ 1193180

static uint32_t _hz_;

void timer_install(uint32_t hz) {
    _hz_ = hz;
    uint32_t divisor = DEFAULTHZ / hz;
    outportb(TIMER_CTL, 0x36);
    outportb(TIMER0, divisor & 0xFF);
    outportb(TIMER0, (divisor >> 8) & 0xFF);
}

#define CLOCKSZ 25

static void clock() {
    static int ticks = 0;
    static int sec = -1;
    if (ticks % _hz_ == 0) {
        sec++;
        ticks = 0;
        int row, col;
        get_cursor(&row, &col);
        // set_cursor(MAXROW - 1, 0);
        set_cursor(0, MAXCOL - CLOCKSZ);
        kprintf(KPL_DEBUG, "clock: %d sec passed", sec);
        set_cursor(row, col);
    }
    ticks++;
}

void do_timer(isrp_t *p) {
    clock();
}
