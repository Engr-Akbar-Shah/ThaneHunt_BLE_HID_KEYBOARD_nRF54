#ifndef APP_KEYCODES_H
#define APP_KEYCODES_H
/* USB HID Usage Page 0x07: Keyboard/Keypad */

/* Specials */
#define HID_KEY_NONE                 0x00  /*   0 */
#define HID_KEY_ERR_ROLL_OVER        0x01  /*   1 */
#define HID_KEY_POST_FAIL            0x02  /*   2 */
#define HID_KEY_ERR_UNDEFINED        0x03  /*   3 */

/* Letters */
#define HID_KEY_A                    0x04  /*   4 */
#define HID_KEY_B                    0x05  /*   5 */
#define HID_KEY_C                    0x06  /*   6 */
#define HID_KEY_D                    0x07  /*   7 */
#define HID_KEY_E                    0x08  /*   8 */
#define HID_KEY_F                    0x09  /*   9 */
#define HID_KEY_G                    0x0A  /*  10 */
#define HID_KEY_H                    0x0B  /*  11 */
#define HID_KEY_I                    0x0C  /*  12 */
#define HID_KEY_J                    0x0D  /*  13 */
#define HID_KEY_K                    0x0E  /*  14 */
#define HID_KEY_L                    0x0F  /*  15 */
#define HID_KEY_M                    0x10  /*  16 */
#define HID_KEY_N                    0x11  /*  17 */
#define HID_KEY_O                    0x12  /*  18 */
#define HID_KEY_P                    0x13  /*  19 */
#define HID_KEY_Q                    0x14  /*  20 */
#define HID_KEY_R                    0x15  /*  21 */
#define HID_KEY_S                    0x16  /*  22 */
#define HID_KEY_T                    0x17  /*  23 */
#define HID_KEY_U                    0x18  /*  24 */
#define HID_KEY_V                    0x19  /*  25 */
#define HID_KEY_W                    0x1A  /*  26 */
#define HID_KEY_X                    0x1B  /*  27 */
#define HID_KEY_Y                    0x1C  /*  28 */
#define HID_KEY_Z                    0x1D  /*  29 */

/* Number row */
#define HID_KEY_1_EXCLAMATION        0x1E  /*  30 */
#define HID_KEY_2_AT                 0x1F  /*  31 */
#define HID_KEY_3_HASH               0x20  /*  32 */
#define HID_KEY_4_DOLLAR             0x21  /*  33 */
#define HID_KEY_5_PERCENT            0x22  /*  34 */
#define HID_KEY_6_CARET              0x23  /*  35 */
#define HID_KEY_7_AMPERSAND          0x24  /*  36 */
#define HID_KEY_8_ASTERISK           0x25  /*  37 */
#define HID_KEY_9_PAREN_LEFT         0x26  /*  38 */
#define HID_KEY_0_PAREN_RIGHT        0x27  /*  39 */

/* Controls + symbols */
#define HID_KEY_ENTER                0x28  /*  40 */
#define HID_KEY_ESCAPE               0x29  /*  41 */
#define HID_KEY_BACKSPACE            0x2A  /*  42 */
#define HID_KEY_TAB                  0x2B  /*  43 */
#define HID_KEY_SPACE                0x2C  /*  44 */
#define HID_KEY_MINUS_UNDERSCORE     0x2D  /*  45 */
#define HID_KEY_EQUAL_PLUS           0x2E  /*  46 */
#define HID_KEY_BRACKET_LEFT         0x2F  /*  47 */
#define HID_KEY_BRACKET_RIGHT        0x30  /*  48 */
#define HID_KEY_BACKSLASH_PIPE       0x31  /*  49 */
#define HID_KEY_NONUS_HASH_TILDE     0x32  /*  50 */ /* ISO key (US: near Enter) */
#define HID_KEY_SEMICOLON_COLON      0x33  /*  51 */
#define HID_KEY_APOSTROPHE_QUOTE     0x34  /*  52 */
#define HID_KEY_GRAVE_TILDE          0x35  /*  53 */
#define HID_KEY_COMMA_LESS           0x36  /*  54 */
#define HID_KEY_DOT_GREATER          0x37  /*  55 */
#define HID_KEY_SLASH_QUESTION       0x38  /*  56 */
#define HID_KEY_CAPS_LOCK            0x39  /*  57 */

/* Function keys */
#define HID_KEY_F1                   0x3A  /*  58 */
#define HID_KEY_F2                   0x3B  /*  59 */
#define HID_KEY_F3                   0x3C  /*  60 */
#define HID_KEY_F4                   0x3D  /*  61 */
#define HID_KEY_F5                   0x3E  /*  62 */
#define HID_KEY_F6                   0x3F  /*  63 */
#define HID_KEY_F7                   0x40  /*  64 */
#define HID_KEY_F8                   0x41  /*  65 */
#define HID_KEY_F9                   0x42  /*  66 */
#define HID_KEY_F10                  0x43  /*  67 */
#define HID_KEY_F11                  0x44  /*  68 */
#define HID_KEY_F12                  0x45  /*  69 */

/* Navigation / edit */
#define HID_KEY_PRINT_SCREEN         0x46  /*  70 */
#define HID_KEY_SCROLL_LOCK          0x47  /*  71 */
#define HID_KEY_PAUSE                0x48  /*  72 */
#define HID_KEY_INSERT               0x49  /*  73 */
#define HID_KEY_HOME                 0x4A  /*  74 */
#define HID_KEY_PAGE_UP              0x4B  /*  75 */
#define HID_KEY_DELETE               0x4C  /*  76 */ /* Forward Delete */
#define HID_KEY_END                  0x4D  /*  77 */
#define HID_KEY_PAGE_DOWN            0x4E  /*  78 */
#define HID_KEY_RIGHT_ARROW          0x4F  /*  79 */
#define HID_KEY_LEFT_ARROW           0x50  /*  80 */
#define HID_KEY_DOWN_ARROW           0x51  /*  81 */
#define HID_KEY_UP_ARROW             0x52  /*  82 */

/* Keypad */
#define HID_KEY_NUM_LOCK             0x53  /*  83 */
#define HID_KEY_KP_SLASH             0x54  /*  84 */
#define HID_KEY_KP_ASTERISK          0x55  /*  85 */
#define HID_KEY_KP_MINUS             0x56  /*  86 */
#define HID_KEY_KP_PLUS              0x57  /*  87 */
#define HID_KEY_KP_ENTER             0x58  /*  88 */
#define HID_KEY_KP_1_END             0x59  /*  89 */
#define HID_KEY_KP_2_DOWN            0x5A  /*  90 */
#define HID_KEY_KP_3_PGDN            0x5B  /*  91 */
#define HID_KEY_KP_4_LEFT            0x5C  /*  92 */
#define HID_KEY_KP_5                 0x5D  /*  93 */
#define HID_KEY_KP_6_RIGHT           0x5E  /*  94 */
#define HID_KEY_KP_7_HOME            0x5F  /*  95 */
#define HID_KEY_KP_8_UP              0x60  /*  96 */
#define HID_KEY_KP_9_PGUP            0x61  /*  97 */
#define HID_KEY_KP_0_INSERT          0x62  /*  98 */
#define HID_KEY_KP_DOT_DELETE        0x63  /*  99 */
#define HID_KEY_NONUS_BACKSLASH      0x64  /* 100 */ /* ISO key near left Shift */
#define HID_KEY_APPLICATION          0x65  /* 101 */ /* Menu */
#define HID_KEY_POWER                0x66  /* 102 */
#define HID_KEY_KP_EQUAL             0x67  /* 103 */

/* Extended function keys */
#define HID_KEY_F13                  0x68  /* 104 */
#define HID_KEY_F14                  0x69  /* 105 */
#define HID_KEY_F15                  0x6A  /* 106 */
#define HID_KEY_F16                  0x6B  /* 107 */
#define HID_KEY_F17                  0x6C  /* 108 */
#define HID_KEY_F18                  0x6D  /* 109 */
#define HID_KEY_F19                  0x6E  /* 110 */
#define HID_KEY_F20                  0x6F  /* 111 */
#define HID_KEY_F21                  0x70  /* 112 */
#define HID_KEY_F22                  0x71  /* 113 */
#define HID_KEY_F23                  0x72  /* 114 */
#define HID_KEY_F24                  0x73  /* 115 */

/* Edit/GUI cluster */
#define HID_KEY_EXECUTE              0x74  /* 116 */
#define HID_KEY_HELP                 0x75  /* 117 */
#define HID_KEY_MENU                 0x76  /* 118 */
#define HID_KEY_SELECT               0x77  /* 119 */
#define HID_KEY_STOP                 0x78  /* 120 */
#define HID_KEY_AGAIN                0x79  /* 121 */
#define HID_KEY_UNDO                 0x7A  /* 122 */
#define HID_KEY_CUT                  0x7B  /* 123 */
#define HID_KEY_COPY                 0x7C  /* 124 */
#define HID_KEY_PASTE                0x7D  /* 125 */
#define HID_KEY_FIND                 0x7E  /* 126 */
#define HID_KEY_MUTE                 0x7F  /* 127 */
#define HID_KEY_VOLUME_UP            0x80  /* 128 */
#define HID_KEY_VOLUME_DOWN          0x81  /* 129 */
#define HID_KEY_LOCKING_CAPS_LOCK    0x82  /* 130 */
#define HID_KEY_LOCKING_NUM_LOCK     0x83  /* 131 */
#define HID_KEY_LOCKING_SCROLL_LOCK  0x84  /* 132 */
#define HID_KEY_KP_COMMA             0x85  /* 133 */
#define HID_KEY_KP_EQUAL_AS400       0x86  /* 134 */

/* International / language keys */
#define HID_KEY_INTL1_RO             0x87  /* 135 */
#define HID_KEY_INTL2_KANASWAP       0x88  /* 136 */
#define HID_KEY_INTL3_YEN            0x89  /* 137 */
#define HID_KEY_INTL4_HENKAN         0x8A  /* 138 */
#define HID_KEY_INTL5_MUHENKAN       0x8B  /* 139 */
#define HID_KEY_INTL6                0x8C  /* 140 */
#define HID_KEY_INTL7                0x8D  /* 141 */
#define HID_KEY_INTL8                0x8E  /* 142 */
#define HID_KEY_INTL9                0x8F  /* 143 */
#define HID_KEY_LANG1_HANGEUL        0x90  /* 144 */
#define HID_KEY_LANG2_HANJA          0x91  /* 145 */
#define HID_KEY_LANG3_KATAKANA       0x92  /* 146 */
#define HID_KEY_LANG4_HIRAGANA       0x93  /* 147 */
#define HID_KEY_LANG5_ZENKAKU        0x94  /* 148 */
#define HID_KEY_LANG6                0x95  /* 149 */
#define HID_KEY_LANG7                0x96  /* 150 */
#define HID_KEY_LANG8                0x97  /* 151 */
#define HID_KEY_LANG9                0x98  /* 152 */

/* System/edit misc */
#define HID_KEY_ALTERNATE_ERASE      0x99  /* 153 */
#define HID_KEY_SYSREQ_ATTENTION     0x9A  /* 154 */
#define HID_KEY_CANCEL               0x9B  /* 155 */
#define HID_KEY_CLEAR                0x9C  /* 156 */
#define HID_KEY_PRIOR                0x9D  /* 157 */
#define HID_KEY_RETURN               0x9E  /* 158 */
#define HID_KEY_SEPARATOR            0x9F  /* 159 */
#define HID_KEY_OUT                  0xA0  /* 160 */
#define HID_KEY_OPER                 0xA1  /* 161 */
#define HID_KEY_CLEAR_AGAIN          0xA2  /* 162 */
#define HID_KEY_CRSEL_PROPS          0xA3  /* 163 */
#define HID_KEY_EXSEL                0xA4  /* 164 */

/* Modifiers (also appear as usages 0xE0–0xE7) */
#define HID_KEY_LCTRL                0xE0  /* 224 */
#define HID_KEY_LSHIFT               0xE1  /* 225 */
#define HID_KEY_LALT                 0xE2  /* 226 */
#define HID_KEY_LGUI                 0xE3  /* 227 */
#define HID_KEY_RCTRL                0xE4  /* 228 */
#define HID_KEY_RSHIFT               0xE5  /* 229 */
#define HID_KEY_RALT                 0xE6  /* 230 */
#define HID_KEY_RGUI                 0xE7  /* 231 */

/* Boot report modifier bit masks (first byte) */
#define HID_MOD_LCTRL                (1u << 0)
#define HID_MOD_LSHIFT               (1u << 1)
#define HID_MOD_LALT                 (1u << 2)
#define HID_MOD_LGUI                 (1u << 3)
#define HID_MOD_RCTRL                (1u << 4)
#define HID_MOD_RSHIFT               (1u << 5)
#define HID_MOD_RALT                 (1u << 6)
#define HID_MOD_RGUI                 (1u << 7)


#endif // APP_KEYCODES_H
