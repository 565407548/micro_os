#define KB_DATA 0x60
#define KB_STATUS 0x64
#define KB_CONTROL 0x64
#define LED_CODE 0xED
#define KB_ACK 0xFA

#define NR_SCAN_CODES 128 
#define MAP_COLS 3
#define KB_IN_BYTES 512

//break code = make code | 0x80 
#define FLAG_BREAK 0x80

//0x100-0x1FF表示不可打印字符
#define FLAG_NP 0x100//unable print flag
#define ESC (FLAG_NP+1)
#define BACKSPACE (FLAG_NP+2)
#define TAB (FLAG_NP+3)
#define ENTER (FLAG_NP+4)
#define WIN (FLAG_NP+5)
#define POINTER (FLAG_NP+6)

#define SHIFT_L (FLAG_NP+7)
#define SHIFT_R (FLAG_NP+8)
#define CTRL_L (FLAG_NP+9)
#define CTRL_R (FLAG_NP+10)
#define ALT_L (FLAG_NP+11)
#define ALT_R (FLAG_NP+12)

#define F1 (FLAG_NP+16)
#define F2 (FLAG_NP+17)
#define F3 (FLAG_NP+18)
#define F4 (FLAG_NP+19)
#define F5 (FLAG_NP+20)
#define F6 (FLAG_NP+21)
#define F7 (FLAG_NP+22)
#define F8 (FLAG_NP+23)
#define F9 (FLAG_NP+24)
#define F10 (FLAG_NP+25)
#define F11 (FLAG_NP+26)
#define F12 (FLAG_NP+27)
#define Fn (FLAG_NP+28)

#define CAPS_LOCK (FLAG_NP+33)
#define NUM_LOCK (FLAG_NP+34)
#define SCROLL_LOCK (FLAG_NP+35)

#define PAD_DIV (FLAG_NP+40)
#define PAD_MUL (FLAG_NP+41)
#define PAD_MINUS (FLAG_NP+42)
#define PAD_PLUS (FLAG_NP+43)
#define PAD_ENTER (FLAG_NP+44)

#define PAD_DEL (FLAG_NP+46)
#define PAD_INS (FLAG_NP+47)
#define PAD_END (FLAG_NP+48)
#define PAD_DOWN (FLAG_NP+49)
#define PAD_PAGEDOWN  (FLAG_NP+50)
#define PAD_LEFT (FLAG_NP+51)
#define PAD_MID (FLAG_NP+52)
#define PAD_RIGHT (FLAG_NP+53)
#define PAD_HOME (FLAG_NP+54)
#define PAD_UP (FLAG_NP+55)
#define PAD_PAGEUP  (FLAG_NP+56)

#define PAD_DOT (FLAG_NP+58)
#define PAD_0 (FLAG_NP+59)
#define PAD_1 (FLAG_NP+60)
#define PAD_2 (FLAG_NP+61)
#define PAD_3 (FLAG_NP+62)
#define PAD_4 (FLAG_NP+63)
#define PAD_5 (FLAG_NP+64)
#define PAD_6 (FLAG_NP+65)
#define PAD_7 (FLAG_NP+66)
#define PAD_8 (FLAG_NP+67)
#define PAD_9 (FLAG_NP+68)

#define INSERT (FLAG_NP+70)
#define HOME (FLAG_NP+71)
#define END (FLAG_NP+72)

#define DELETE (FLAG_NP+74)
#define PAGEUP (FLAG_NP+75)
#define PAGEDOWN (FLAG_NP+76)

#define LEFT (FLAG_NP+77)
#define RIGHT (FLAG_NP+78)
#define UP (FLAG_NP+79)
#define DOWN (FLAG_NP+80)

#define GUI_L  (FLAG_NP+85)
#define GUI_R (FLAG_NP+86)
#define  APPS (FLAG_NP+87)

#define PRINTSCREEN (FLAG_NP+90)
#define PAUSEBREAK (FLAG_NP+91)
//to 58

#define FLAG_AUX_MASK 0x1FF//清楚下列标志掩码
#define FLAG_SHIFT_L 0x200
#define FLAG_SHIFT_R 0x400
#define FLAG_ALT_L 0x800
#define FLAG_ALT_R 0x1000
#define FLAG_CTRL_L 0x2000
#define FLAG_CTRL_R 0x4000
#define FLAG_PAD 0x8000



