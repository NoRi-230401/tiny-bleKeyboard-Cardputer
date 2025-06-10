#ifndef M5CARDPUTER_KB_KEYS_DEF_H
#define M5CARDPUTER_KB_KEYS_DEF_H

const uint8_t M5_SHIFT = 0x80;

const uint8_t M5_KEY_LEFT_CTRL = 0x80;
const uint8_t M5_KEY_LEFT_SHIFT = 0x81;
const uint8_t M5_KEY_LEFT_ALT = 0x82;

const uint8_t M5_KEY_FN = 0xff;

// const uint8_t KEY_OPT = 0x00;
// const uint8_t KEY_RIGHT_GUI = 0x87;   
const uint8_t M5_KEY_OPT = 0x87;   // modified by NoRi 2025-05-30
//-----------------------------------------------------------------------

const uint8_t M5_KEY_BACKSPACE = 0x2a;
const uint8_t M5_KEY_TAB = 0x2b;
const uint8_t M5_KEY_ENTER = 0x28;

const uint8_t _kb_asciimap[128] = {
    0x00,           // NUL
    0x00,           // SOH
    0x00,           // STX
    0x00,           // ETX
    0x00,           // EOT
    0x00,           // ENQ
    0x00,           // ACK
    0x00,           // BEL
    M5_KEY_BACKSPACE,  // BS	Backspace
    M5_KEY_TAB,        // TAB	Tab
    M5_KEY_ENTER,      // LF	Enter
    0x00,           // VT
    0x00,           // FF
    0x00,           // CR
    0x00,           // SO
    0x00,           // SI
    0x00,           // DEL
    0x00,           // DC1
    0x00,           // DC2
    0x00,           // DC3
    0x00,           // DC4
    0x00,           // NAK
    0x00,           // SYN
    0x00,           // ETB
    0x00,           // CAN
    0x00,           // EM
    0x00,           // SUB
    0x00,           // ESC
    0x00,           // FS
    0x00,           // GS
    0x00,           // RS
    0x00,           // US

    0x2c,          //  ' '
    0x1e | M5_SHIFT,  // !
    0x34 | M5_SHIFT,  // "
    0x20 | M5_SHIFT,  // #
    0x21 | M5_SHIFT,  // $
    0x22 | M5_SHIFT,  // %
    0x24 | M5_SHIFT,  // &
    0x34,          // '
    0x26 | M5_SHIFT,  // (
    0x27 | M5_SHIFT,  // )
    0x25 | M5_SHIFT,  // *
    0x2e | M5_SHIFT,  // +
    0x36,          // ,
    0x2d,          // -
    0x37,          // .
    0x38,          // /
    0x27,          // 0
    0x1e,          // 1
    0x1f,          // 2
    0x20,          // 3
    0x21,          // 4
    0x22,          // 5
    0x23,          // 6
    0x24,          // 7
    0x25,          // 8
    0x26,          // 9
    0x33 | M5_SHIFT,  // :
    0x33,          // ;
    0x36 | M5_SHIFT,  // <
    0x2e,          // =
    0x37 | M5_SHIFT,  // >
    0x38 | M5_SHIFT,  // ?
    0x1f | M5_SHIFT,  // @
    0x04 | M5_SHIFT,  // A
    0x05 | M5_SHIFT,  // B
    0x06 | M5_SHIFT,  // C
    0x07 | M5_SHIFT,  // D
    0x08 | M5_SHIFT,  // E
    0x09 | M5_SHIFT,  // F
    0x0a | M5_SHIFT,  // G
    0x0b | M5_SHIFT,  // H
    0x0c | M5_SHIFT,  // I
    0x0d | M5_SHIFT,  // J
    0x0e | M5_SHIFT,  // K
    0x0f | M5_SHIFT,  // L
    0x10 | M5_SHIFT,  // M
    0x11 | M5_SHIFT,  // N
    0x12 | M5_SHIFT,  // O
    0x13 | M5_SHIFT,  // P
    0x14 | M5_SHIFT,  // Q
    0x15 | M5_SHIFT,  // R
    0x16 | M5_SHIFT,  // S
    0x17 | M5_SHIFT,  // T
    0x18 | M5_SHIFT,  // U
    0x19 | M5_SHIFT,  // V
    0x1a | M5_SHIFT,  // W
    0x1b | M5_SHIFT,  // X
    0x1c | M5_SHIFT,  // Y
    0x1d | M5_SHIFT,  // Z
    0x2f,          // [
    0x31,          // bslash
    0x30,          // ]
    0x23 | M5_SHIFT,  // ^
    0x2d | M5_SHIFT,  // _
    0x35,          // `
    0x04,          // a
    0x05,          // b
    0x06,          // c
    0x07,          // d
    0x08,          // e
    0x09,          // f
    0x0a,          // g
    0x0b,          // h
    0x0c,          // i
    0x0d,          // j
    0x0e,          // k
    0x0f,          // l
    0x10,          // m
    0x11,          // n
    0x12,          // o
    0x13,          // p
    0x14,          // q
    0x15,          // r
    0x16,          // s
    0x17,          // t
    0x18,          // u
    0x19,          // v
    0x1a,          // w
    0x1b,          // x
    0x1c,          // y
    0x1d,          // z
    0x2f | M5_SHIFT,  // {
    0x31 | M5_SHIFT,  // |
    0x30 | M5_SHIFT,  // }
    0x35 | M5_SHIFT,  // ~
    0              // DEL
};

#endif
