#include <device/tty.h>
#include <device/console.h>
#include <device/kb.h>
#include <thread/sync.h>
#include <arch/x86.h>
#include <common/debug.h>

#define KB_BUFFER_SIZE 64

struct TTY {
    // the console associated to this tty
    console_t *my_console;
    // my tty number (0 ~ NR_TTY - 1)
    int tty_no;

    key_info_t kb_in_buffer[KB_BUFFER_SIZE];
    uint32_t kb_inbuf_head;
    size_t kb_inbuf_num;
    sem_t kb_sem;
};

static tty_t ttys[NR_TTY];


void tty_putkey(key_info_t ki) {
    ASSERT(get_int_status() == INTERRUPT_OFF);
    if (__keyflags(ki) == KIF_ALT) {
        key_code_e keycode = __keycode(ki);
        if (keycode >= KEYCODE_F1 && keycode <= KEYCODE_F5) {
            select_console(keycode - KEYCODE_F1);
            return;
        }
    }
    int tty_no = get_curr_console_tty();
    if (tty_no == -1) {
        return;
    }
    tty_t *tty = &(ttys[tty_no]);
    if (tty->kb_inbuf_num == KB_BUFFER_SIZE) {
        return;
    }
    int buf_tail = (tty->kb_inbuf_head + tty->kb_inbuf_num) % KB_BUFFER_SIZE;
    tty->kb_in_buffer[buf_tail] = ki;
    (tty->kb_inbuf_num)++;
    sem_up(&(tty->kb_sem));
}

void tty_flush_key_buffer(int tty_no) {
    // INT_STATUS old_status = disable_int();
    // tty_t *tty = &(ttys[tty_no]);
    // tty->kb_inbuf_head = 0;
    // tty->kb_inbuf_num = 0;
    // tty->kb_sem.val = 0;
    // set_int_status(old_status);
}

key_info_t tty_getkey(int tty_no) {
    tty_t *tty = &(ttys[tty_no]);
    sem_down(&(tty->kb_sem));
    INT_STATUS old_status = disable_int();
    ASSERT(tty->kb_inbuf_num > 0);
    ASSERT(tty->kb_inbuf_num == tty->kb_sem.val + 1);
    key_info_t ki = tty->kb_in_buffer[tty->kb_inbuf_head];
    tty->kb_inbuf_head = (tty->kb_inbuf_head + 1) % KB_BUFFER_SIZE;
    (tty->kb_inbuf_num)--;
    set_int_status(old_status);
    return ki;
}


void tty_init() {
    for (int i = 0; i < NR_TTY; i++) {
        ttys[i].tty_no = i;
        ttys[i].kb_inbuf_head = 0;
        ttys[i].kb_inbuf_num = 0;
        sem_init(&(ttys[i].kb_sem), 0);
        ttys[i].my_console = get_console(i);
    }
    select_console(0);
    kb_init();
}


int tty_puts(
    int tty_no, const char *str, size_t count, color_e bg, color_e fg
) {
    if (tty_no < 0 || tty_no >= NR_TTY) {
        return 0;
    }
    return console_puts(ttys[tty_no].my_console, str, count, bg, fg);
}
