#include <stdint.h>
#include <stddef.h>
#include "base_kernel.h" // For common includes

// Existing hex conversion functions (if you have them)
void uint16_to_hex_str(uint16_t val, char* buf) {
    int i = 0;
    if (val == 0) { buf[0] = '0'; i = 1; }
    else { uint16_t temp = val; while (temp > 0) { uint8_t rem = temp % 16; if (rem < 10) buf[i++] = rem + '0'; else buf[i++] = rem + 'A' - 10; temp /= 16; } }
    buf[i] = '\0';
    for (int start = 0, end = i - 1; start < end; start++, end--) { char tmp = buf[start]; buf[start] = buf[end]; buf[end] = tmp; }
    if (i == 0) { buf[0] = '0'; buf[1] = '\0'; }
}

void uint8_to_hex_str(uint8_t val, char* buf) {
    uint16_to_hex_str((uint16_t)val, buf);
}

// New: uint32_to_hex_str
void uint32_to_hex_str(uint32_t val, char* buf) {
    int i = 0;
    if (val == 0) { buf[0] = '0'; i = 1; }
    else { uint32_t temp = val; while (temp > 0) { uint8_t rem = temp % 16; if (rem < 10) buf[i++] = rem + '0'; else buf[i++] = rem + 'A' - 10; temp /= 16; } }
    buf[i] = '\0';
    for (int start = 0, end = i - 1; start < end; start++, end--) { char tmp = buf[start]; buf[start] = buf[end]; buf[end] = tmp; }
    if (i == 0) { buf[0] = '0'; buf[1] = '\0'; }
}

// New: uint64_to_hex_str
void uint64_to_hex_str(uint64_t val, char* buf) {
    int i = 0;
    if (val == 0) { buf[0] = '0'; i = 1; }
    else { uint64_t temp = val; while (temp > 0) { uint8_t rem = temp % 16; if (rem < 10) buf[i++] = rem + '0'; else buf[i++] = rem + 'A' - 10; temp /= 16; } }
    buf[i] = '\0';
    for (int start = 0, end = i - 1; start < end; start++, end--) { char tmp = buf[start]; buf[start] = buf[end]; buf[end] = tmp; }
    if (i == 0) { buf[0] = '0'; buf[1] = '\0'; }
}
