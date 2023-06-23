#include <stdarg.h>

#include "kernel.h"

#include "internal.h"

enum flags { PAD_ZERO = 1, PAD_RIGHT = 2 };

#define PRINT_BUF_LEN 64

static void __printk_char(ring_buf_t* out, char c) {
    if (out)
        ring_buf_put(out, c);
    else
        console_putchar(c);
}

static int __printk_str(ring_buf_t* out,
                        const char* string,
                        int width,
                        int flags) {
    int pc = 0, padchar = ' ';

    if (width > 0) {
        int len = 0;
        const char* ptr;
        for (ptr = string; *ptr; ++ptr)
            ++len;
        if (len >= width)
            width = 0;
        else
            width -= len;
        if (flags & PAD_ZERO)
            padchar = '0';
    }
    if (!(flags & PAD_RIGHT)) {
        for (; width > 0; --width) {
            __printk_char(out, padchar);
            ++pc;
        }
    }
    for (; *string; ++string) {
        __printk_char(out, *string);
        ++pc;
    }
    for (; width > 0; --width) {
        __printk_char(out, padchar);
        ++pc;
    }

    return pc;
}

static int __printk_int(ring_buf_t* out,
                        long long i,
                        int base,
                        int sign,
                        int width,
                        int flags,
                        int letbase) {
    char print_buf[PRINT_BUF_LEN];
    char* s;
    int t, neg = 0, pc = 0;
    unsigned long long u = i;

    if (i == 0) {
        print_buf[0] = '0';
        print_buf[1] = '\0';
        return __printk_str(out, print_buf, width, flags);
    }

    if (sign && base == 10 && i < 0) {
        neg = 1;
        u = -i;
    }

    s = print_buf + PRINT_BUF_LEN - 1;
    *s = '\0';

    while (u) {
        t = u % base;
        if (t >= 10)
            t += letbase - '0' - 10;
        *--s = t + '0';
        u /= base;
    }

    if (neg) {
        if (width && (flags & PAD_ZERO)) {
            __printk_char(out, '-');
            ++pc;
            --width;
        } else {
            *--s = '-';
        }
    }

    return pc + __printk_str(out, s, width, flags);
}

int vsprintf(ring_buf_t* out, const char* format, va_list ap) {
    int width, flags;
    int pc = 0;
    char scr[2];
    union {
        char c;
        char* s;
        int i;
        unsigned int u;
        long li;
        unsigned long lu;
        long long lli;
        unsigned long long llu;
        short hi;
        unsigned short hu;
        signed char hhi;
        unsigned char hhu;
        void* p;
    } u;

    for (; *format != 0; ++format) {
        if (*format == '%') {
            ++format;
            width = flags = 0;
            if (*format == '\0')
                break;
            if (*format == '%')
                goto out;
            if (*format == '-') {
                ++format;
                flags = PAD_RIGHT;
            }
            while (*format == '0') {
                ++format;
                flags |= PAD_ZERO;
            }
            if (*format == '*') {
                width = va_arg(ap, int);
                format++;
            } else {
                for (; *format >= '0' && *format <= '9'; ++format) {
                    width *= 10;
                    width += *format - '0';
                }
            }
            switch (*format) {
                case ('d'):
                    u.i = va_arg(ap, int);
                    pc += __printk_int(out, u.i, 10, 1, width, flags, 'a');
                    break;

                case ('b'):
                    u.i = va_arg(ap, int);
                    pc += __printk_int(out, u.i, 2, 1, width, flags, 'a');
                    break;

                case ('u'):
                    u.u = va_arg(ap, unsigned int);
                    pc += __printk_int(out, u.u, 10, 0, width, flags, 'a');
                    break;

                case ('p'):
                    u.llu = va_arg(ap, unsigned long);
                    pc += __printk_int(out, u.llu, 16, 0, width, flags, 'a');
                    break;

                case ('x'):
                    u.u = va_arg(ap, unsigned int);
                    pc += __printk_int(out, u.u, 16, 0, width, flags, 'a');
                    break;

                case ('X'):
                    u.u = va_arg(ap, unsigned int);
                    pc += __printk_int(out, u.u, 16, 0, width, flags, 'A');
                    break;

                case ('c'):
                    u.c = va_arg(ap, int);
                    scr[0] = u.c;
                    scr[1] = '\0';
                    pc += __printk_str(out, scr, width, flags);
                    break;

                case ('s'):
                    u.s = va_arg(ap, char*);
                    pc += __printk_str(out, u.s ? u.s : "(null)", width, flags);
                    break;
                case ('l'):
                    ++format;
                    switch (*format) {
                        case ('d'):
                            u.li = va_arg(ap, long);
                            pc += __printk_int(out, u.li, 10, 1, width, flags,
                                               'a');
                            break;

                        case ('u'):
                            u.lu = va_arg(ap, unsigned long);
                            pc += __printk_int(out, u.lu, 10, 0, width, flags,
                                               'a');
                            break;

                        case ('x'):
                            u.lu = va_arg(ap, unsigned long);
                            pc += __printk_int(out, u.lu, 16, 0, width, flags,
                                               'a');
                            break;

                        case ('X'):
                            u.lu = va_arg(ap, unsigned long);
                            pc += __printk_int(out, u.lu, 16, 0, width, flags,
                                               'A');
                            break;

                        case ('l'):
                            ++format;
                            switch (*format) {
                                case ('d'):
                                    u.lli = va_arg(ap, long long);
                                    pc += __printk_int(out, u.lli, 10, 1, width,
                                                       flags, 'a');
                                    break;

                                case ('u'):
                                    u.llu = va_arg(ap, unsigned long long);
                                    pc += __printk_int(out, u.llu, 10, 0, width,
                                                       flags, 'a');
                                    break;

                                case ('x'):
                                    u.llu = va_arg(ap, unsigned long long);
                                    pc += __printk_int(out, u.llu, 16, 0, width,
                                                       flags, 'a');
                                    break;

                                case ('X'):
                                    u.llu = va_arg(ap, unsigned long long);
                                    pc += __printk_int(out, u.llu, 16, 0, width,
                                                       flags, 'A');
                                    break;

                                default:
                                    break;
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                case ('h'):
                    ++format;
                    switch (*format) {
                        case ('d'):
                            u.hi = va_arg(ap, int);
                            pc += __printk_int(out, u.hi, 10, 1, width, flags,
                                               'a');
                            break;

                        case ('u'):
                            u.hu = va_arg(ap, unsigned int);
                            pc += __printk_int(out, u.lli, 10, 0, width, flags,
                                               'a');
                            break;

                        case ('x'):
                            u.hu = va_arg(ap, unsigned int);
                            pc += __printk_int(out, u.lli, 16, 0, width, flags,
                                               'a');
                            break;

                        case ('X'):
                            u.hu = va_arg(ap, unsigned int);
                            pc += __printk_int(out, u.lli, 16, 0, width, flags,
                                               'A');
                            break;

                        case ('h'):
                            ++format;
                            switch (*format) {
                                case ('d'):
                                    u.hhi = va_arg(ap, int);
                                    pc += __printk_int(out, u.hhi, 10, 1, width,
                                                       flags, 'a');
                                    break;

                                case ('u'):
                                    u.hhu = va_arg(ap, unsigned int);
                                    pc += __printk_int(out, u.lli, 10, 0, width,
                                                       flags, 'a');
                                    break;

                                case ('x'):
                                    u.hhu = va_arg(ap, unsigned int);
                                    pc += __printk_int(out, u.lli, 16, 0, width,
                                                       flags, 'a');
                                    break;

                                case ('X'):
                                    u.hhu = va_arg(ap, unsigned int);
                                    pc += __printk_int(out, u.lli, 16, 0, width,
                                                       flags, 'A');
                                    break;

                                default:
                                    break;
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
        } else {
        out:
            if (*format == '\n')
                __printk_char(out, '\r');
            __printk_char(out, *format);
            ++pc;
        }
    }
    if (out)
        ring_buf_put(out, '\0');
    return pc;
}
