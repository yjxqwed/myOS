#include <driver/pit.h>
#include <arch/x86.h>
#include <kprintf.h>

#define TIMER0 0
#define TIMER1 1
#define TIMER2 2

#define TIMER0_PORT 0x40
#define TIMER1_PORT 0x41
#define TIMER2_PORT 0x42
#define TIMER_CTL_PORT 0x43  // write only

// below are PIT work modes

// interrupt on terminal count
#define INT_TERMINAL_CNT 0
// hardware retriggerable one-shot
#define HW_RETRIGGERABLE_ONE_SHOT 1
// rate generator
#define RATE_GEN 2
// square wave generator
#define SQR_WV_GEN 3
// software triggered strobe
#define SW_TRIGGERED_STROBE 4
// hardware triggered strobe
#define HW_TRIGGERED_STROBE 5

// below are PIT read/write access modes
#define LATCH_CNT 0
#define LOW_BYTE_ONLY 1
#define HIGH_BYTE_ONLY 2
#define LOW_HIGH_BYTE 3

// below are BCD/binary modes

// 16-bit binary
#define CNT_MODE_BIN 0
// 4-bit decimal
#define CNT_MODE_BCD 1


// PIT input frequency
#define PIT_INPT_FREQ 1193180

static uint32_t _hz_;

void timer_install(uint32_t hz) {
    _hz_ = hz;
    uint32_t divisor = PIT_INPT_FREQ / hz;
    // outportb(TIMER_CTL_PORT, 0x36);
    outportb(TIMER_CTL_PORT, (uint8_t)(
        TIMER0 << 6 | LOW_HIGH_BYTE << 4 | RATE_GEN << 1 | CNT_MODE_BIN
    ));
    outportb(TIMER0_PORT, divisor & 0xFF);
    outportb(TIMER0_PORT, (divisor >> 8) & 0xFF);
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
    // clock();
}
