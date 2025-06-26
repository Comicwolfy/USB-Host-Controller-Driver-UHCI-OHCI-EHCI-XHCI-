// In the user's includes/base_kernel.h:

// Add declarations for common utility functions (if not already present):
void uint16_to_hex_str(uint16_t val, char* buf); // Potentially already here from PCI
void uint8_to_hex_str(uint8_t val, char* buf);   // Potentially already here from PCI

void uint32_to_hex_str(uint32_t val, char* buf); // <--- ADD THIS LINE
void uint64_to_hex_str(uint64_t val, char* buf); // <--- ADD THIS LINE
