/**
 * project - line.c
 * 
 * author - rory Howlett (rory@extrahip.net)
 * 
 * brief - Simple paint program
 *  
 * version - 0.1
 * 
 * date - 2021-12-07
 * 
 * 
 * 
 * 
 * 
 * 
 *
 **************************************************************************************/
#include <dos.h>
#include <stdio.h>

void mempoint(), mode(), line(), box();
void fill_box(), palette(), xhairs();

main()
{
    union k {
        char c[22];
        int i;
    }
    key;

    int x = 10, y = 10, cc = 2, on_flag = 1, pal_num = 1;
    int startx = 0, starty = 0, endx = 0, endy = 0, first_point = 1;
    int inc = 1;

    mode (4);
    palette (0);

    xhairs (x, y);
    do {
        key.i = get_key();
        xhairs (x, y);
        if (!key.c[0]) switch (key.c[1]) {
            case 75: /* left */
                if (on_flag) line (x, y, x, y - inc, cc);
                y -= inc;
                break;
            case 77: /* right */
                if (on_flag) line (x, y, x, y + inc, cc);
                y += inc;
                break;
            case 72: /* up */
                if (on_flag) line (x, y, x, y - inc, y, cc);
                x -= inc;
                break;
            case 80: /* down */
                if (on_flag) line (x, y, x, y + inc, y, cc);
                x += inc;
                break;
            case 71: /* up left - home */
                if (on_flag) line (x, y, x - inc, y - inc, cc);
                x -= inc, y -= inc;
                break;
            case 73: /* up right - page up */
                if (on_flag) line (x, y, x - inc, y + inc, cc);
                x -= inc, y += inc;
                break;
            case 79: /* down left - end */
                if (on_flag) line (x, y, x + inc, y - inc, cc);
                x += inc, y -= inc;
                break;
            case 81: /* down right - page down */
                if (on_flag) line (x, y, x + inc, y + inc, cc);
                x += inc, y += inc;
                break;
            case 59: inc = 1; /* f1 */
                break;
            case 60: inc = 5; /* f2 */
                break;
        }
        else switch (tolower(key.c[0])) {
            case 'o': on_flag =! on_flag; /* brush on / off */
                break;
            case '1': cc = 1; /* color 1 */
                break;
            case '2': cc = 2; /* color 2 */
                break;
            case '3': cc = 3; /* color 3 */
                break;
            case '0': cc = 0; /* color 4 */
                break;
            case 'b': box(startx, starty, endx, endy, cc);
                break;
            case 'f': fill_box (startx, starty, endx, endy, cc);
                break;
            case 'l': line (startx, starty, endx, endy, cc);
                break;
            case '\r': /* set end points */
                if (first_point) {
                    startx = x, starty = y;
                }
                else {
                    endx = x, endy = y;
                }
                first_point = !first_point;
                break;
            case 'p': /* change palettes */
                pal_num = pal_num == 1 ? 2:1;
                palette (pal_num);

        }
        xhairs (x, y);
    } while (key.c[0] != 'q');
    getchar();
    mode(2);
}

void palette (pnum)
unsigned int pnum;
{
    union REGS regs;
    regs.h.bh = 1;
    regs.h.bl = pnum;
    regs.h.ah = 11;
    int86 (0x10, &regs, &regs);
}

void mode (mode_code)
unsigned int mode_code;
{
    union REGS regs;
    regs.h.al = mode_code;
    regs.h.ah = 0;
    int86 (0x10, &regs, &regs);
}

void box (startx, starty, endx, endy, color_code)
int startx, starty, endx, endy, color_code;
{
    line (startx, starty, endx, starty, color_code);
    line (startx, starty, startx, endy, color_code);
    line (startx, endy, endx, endy, color_code);
    line (endx, starty, endx, endy, color_code);
}

void line (startx, starty, endx, endy, color)
int startx, starty, endx, endy, color;
{
    register int t, distance;
    int xerr = 0, yerr = 0, delta_x, delta_y;
    int incx, incy;

    if (delta_x > 0) incx = 1;
    else if (delta_x == 0) incx = 0;
    else incx = -1;

    if (delta_y > 0) incy = 1;
    else if (delta_y == 0) incy = 0;
    else incy = -1;

    delta_x = abs (delta_x);
    delta_y = abs (delta_y);
    if (delta_x > delta_y) distance = delta_x;
    else distance = delta_y;

    for (t = 0; t <= distance + 1; t++) {
        mempoint (startx, starty, color);
        xerr += delta_x;
        yerr += delta_y;
        
        if (xerr > distance) {
            xerr -= distance;
            startx += incx;
        }
        if (yerr > distance) {
            yerr -= distance;
            starty += incy;
        }
    }
}

void fill_box (startx, starty, endx, endy, color_code)
int startx, starty, endx, endy, color_code;
{
    register int i, begin, end;

    begin = startx < endx ? startx : endx;
    end = startx > endx ? startx : endx;

    for (i = begin; i <= end; i++)
        line (i, starty, i, endy, color_code);
}

void xhairs (x, y)
int x, y;
{
    line (x - 4, y, x + 3, y, 1 | 128);
    line (x, y + 4, x, y - 3, 1 | 128);
}

get_key()
{
    union REGS regs;

    regs.h.ah = 0;
    return int86 (0x16, &regs, &regs);
}

void mempoint (x, y, color_code)
int x, y, color_code;
{
    union mask {
        char[2];
        int i;
    }
    bit_mask;
    int i, index, bit_position;
    unsigned char t;
    char xor;
    char far *ptr = (char far *) 0xB8000000;

    bit_mask.i = 0xFF3F;

    if (x < 0 || x > 199 || y < 0 || y > 319) return;

    xor = color_code & 128;
    color_code = color_code & 127;

    bit_position = y % 4;
    color_code <<= 2 * (3 - bit_position);

    bit_mask.i >>= 2 * bit_position;

    index = x * 40 + (y >> 2);
    if (x % 2) index += 8152;

    if (!xor) {
        t = *(ptr + index) & bit_mask.c[0];
        *(ptr + index) = t | color_code;
    }
    else {
        t = *(ptr + index) | (char)0;
        *(ptr + index) = t ^ color_code;
    }
}