#include <device/tty.h>
#include <device/kb.h>
#include <thread/sync.h>
#include <arch/x86.h>
#include <common/debug.h>
#include <common/utils.h>
#include <lib/kprintf.h>

#define KB_BUFFER_SIZE 64
#define SYS_READ_BUF_SIZE 128

static int tty_ready = 0;

typedef struct TTY {
    // the console associated to this tty
    console_t *my_console;
    // my tty number (0 ~ NR_TTY - 1)
    int tty_no;

    key_info_t kb_in_buffer[KB_BUFFER_SIZE];
    uint32_t kb_inbuf_head;
    size_t kb_inbuf_num;
    sem_t kb_sem;

    // sys_read_buf
    char rbuf[SYS_READ_BUF_SIZE];
    uint32_t rbuf_head;
    size_t rbuf_num;
    mutex_t rbuf_lock;
} tty_t ;

static tty_t ttys[NR_TTY];

// echo the input char
static void tty_echo_char(console_t *con, key_info_t ki) {
    ASSERT(get_int_status() == INTERRUPT_OFF);
    key_code_e kcode = __keycode(ki);
    uint32_t kf = __keyflags(ki);
    // console_kprintf(KPL_DEBUG, "{%X}", ki);
    if (KEYCODE_BACKSPACE == kcode) {
        console_erase_char(con);
    } else if (KEYCODE_ENTER == kcode) {
        console_puts_nolock(con, "\n", 1, CONS_BLACK, CONS_GRAY, False);
    } else if (kcode >= KEYCODE_A && kcode <= KEYCODE_RIGHTBRACKET) {
        char c[1];
        c[0] = get_printable_char(ki);
        if (c[0] != '\0') {
            console_puts_nolock(con, c, 1, CONS_BLACK, CONS_GRAY, False);
        }
    } else {
        // console_puts_nolock(con, "<HELLO>", 8, CONS_BLACK, CONS_GRAY, False);
    }
}

static key_info_t __tty_getkey(tty_t *tty) {
    sem_down(&(tty->kb_sem));
    // INT_STATUS old_status = disable_int();
    IRQ_set_mask(INT_KB);
    ASSERT(tty->kb_inbuf_num > 0);
    ASSERT(tty->kb_inbuf_num == tty->kb_sem.val + 1);
    key_info_t ki = tty->kb_in_buffer[tty->kb_inbuf_head];
    tty->kb_inbuf_head = (tty->kb_inbuf_head + 1) % KB_BUFFER_SIZE;
    (tty->kb_inbuf_num)--;
    IRQ_clear_mask(INT_KB);
    // set_int_status(old_status);
    return ki;
}

// put chars into tty->rbuf until '\n' or full
static void tty_put_rbuf(tty_t *tty) {
    ASSERT(0 == tty->rbuf_num);

    while (1) {
        key_info_t ki = __tty_getkey(tty);
        key_code_e keycode = __keycode(ki);
        if (KEYCODE_BACKSPACE == keycode) {
            if (tty->rbuf_num > 0) {
                (tty->rbuf_num)--;
            }
        } else {
            char c = get_printable_char(ki);
            if ('\0' == c) {
                continue;
            }
            int rbuf_tail = (tty->rbuf_head + tty->rbuf_num) % SYS_READ_BUF_SIZE;
            tty->rbuf[rbuf_tail] = c;
            (tty->rbuf_num)++;
            if ('\n' == c || SYS_READ_BUF_SIZE == tty->rbuf_num) {
                break;
            }
        }
    }
}

void tty_putkey(key_info_t ki) {
    ASSERT(get_int_status() == INTERRUPT_OFF);

    // CTRL-[1-6] is used to change TTY
    if (__keyflags(ki) == (KIF_CTRL)) {
        key_code_e keycode = __keycode(ki);
        if (keycode >= KEYCODE_ALPHA1 && keycode <= KEYCODE_ALPHA6) {
            select_console(keycode - KEYCODE_ALPHA1);
            return;
        }
    }

    int tty_no = get_curr_console_tty();
    ASSERT(0 <= tty_no && tty_no < NR_TTY);
    tty_t *tty = &(ttys[tty_no]);

    // kb buffer
    if (tty->kb_inbuf_num == KB_BUFFER_SIZE) {
        return;
    }
    int buf_tail = (tty->kb_inbuf_head + tty->kb_inbuf_num) % KB_BUFFER_SIZE;
    tty->kb_in_buffer[buf_tail] = ki;
    (tty->kb_inbuf_num)++;
    sem_up(&(tty->kb_sem));
    tty_echo_char(tty->my_console, ki);
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
    return __tty_getkey(tty);
}

key_info_t tty_getkey_curr() {
    tty_getkey(get_current_thread()->tty_no);
}

int tty_read(int tty_no, char *buf, size_t count) {
    if (tty_no < 0 || tty_no >= NR_TTY) {
        return -1;
    }
    tty_t *tty = &(ttys[tty_no]);

    mutex_lock(&(tty->rbuf_lock));
    if (0 == tty->rbuf_num) {
        tty_put_rbuf(tty);
    }

    ASSERT(tty->rbuf_num > 0);
    count = MIN(count, tty->rbuf_num);
    for (uint32_t i = 0; i < count; i++) {
        buf[i] = tty->rbuf[(tty->rbuf_head + i) % SYS_READ_BUF_SIZE];
    }
    tty->rbuf_head = (tty->rbuf_head + count) % SYS_READ_BUF_SIZE;
    tty->rbuf_num -= count;
    mutex_unlock(&(tty->rbuf_lock));

    return count;
}

int tty_read_curr(char *buf, size_t count) {
    return tty_read(get_current_thread()->tty_no, buf, count);
}

void tty_init() {
    for (int i = 0; i < NR_TTY; i++) {
        ttys[i].tty_no = i;
        // init kb_infbuf
        ttys[i].kb_inbuf_head = 0;
        ttys[i].kb_inbuf_num = 0;
        sem_init(&(ttys[i].kb_sem), 0);
        // init rbuf
        ttys[i].rbuf_head = 0;
        ttys[i].rbuf_num = 0;
        mutex_init(&(ttys[i].rbuf_lock));
        // init console
        ttys[i].my_console = init_console(i);
    }
    select_console(0);
    kb_init();
    tty_ready = 1;
}


int tty_puts(int tty_no, const char *str, size_t count, color_e bg, color_e fg) {
    if (!tty_ready || tty_no < 0 || tty_no >= NR_TTY) {
        return -1;
    }
    return console_puts(ttys[tty_no].my_console, str, count, bg, fg, True);
}

int tty_puts_curr(const char *str, size_t count, color_e bg, color_e fg) {
    return tty_puts(get_curr_console_tty(), str, count, bg, fg);
}
