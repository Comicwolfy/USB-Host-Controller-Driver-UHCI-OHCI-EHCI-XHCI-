#include <stdint.h>
#include <stddef.h>
#include "base_kernel.h"

// --- XHCI PCI Configuration Space Register Offsets (Type 0 Header) ---
// The XHCI controller is a PCI device. Its base address is found in a BAR.
#define XHCI_PCI_BAR0   0x10 // Base Address Register 0 - points to MMIO space

// --- XHCI Capability Register (CAPLENGTH) ---
#define XHCI_CAPLENGTH_OFFSET       0x00 // Capability Register Length & HCI Version
#define XHCI_HCSPARAMS1_OFFSET      0x04 // HCI Structural Parameters 1
#define XHCI_HCCPARAMS1_OFFSET      0x08 // HCI Capability Parameters 1
#define XHCI_DBOFF_OFFSET           0x14 // Doorbell Offset
#define XHCI_RTSOFF_OFFSET          0x18 // Runtime Register Space Offset

// --- XHCI Operational Registers (Memory-mapped) ---
// These are accessed via the BAR address + offset
#define XHCI_USBCMD_OFFSET          0x00 // USB Command Register
#define XHCI_USBSTS_OFFSET          0x04 // USB Status Register
#define XHCI_PAGESIZE_OFFSET        0x08 // Page Size Register
#define XHCI_DNCTRL_OFFSET          0x14 // Device Notification Control Register
#define XHCI_CRCR_OFFSET            0x18 // Command Ring Control Register
#define XHCI_DCBAAP_OFFSET          0x30 // Device Context Base Address Array Pointer
#define XHCI_CONFIG_OFFSET          0x38 // Configure Register

// USB Command Register (USBCMD) bits
#define XHCI_USBCMD_RS          0x00000001 // Run/Stop
#define XHCI_USBCMD_HCRST       0x00000002 // Host Controller Reset
#define XHCI_USBCMD_INTE        0x00000004 // Interrupter Enable
#define XHCI_USBCMD_HSEE        0x00000008 // Host System Error Enable

// USB Status Register (USBSTS) bits
#define XHCI_USBSTS_HCH         0x00000001 // Host Controller Halted
#define XHCI_USBSTS_HSE         0x00000004 // Host System Error
#define XHCI_USBSTS_EINT        0x00000008 // Event Interrupt
#define XHCI_USBSTS_PCD         0x00000010 // Port Change Detect
#define XHCI_USBSTS_FATAL_ERROR (XHCI_USBSTS_HSE | XHCI_USBSTS_HCH) // Simplified fatal error check

// --- Global Extension ID and XHCI MMIO Base ---
static int xhci_ext_id = -1;
static volatile uint8_t* xhci_mmio_base = NULL;
static uint8_t xhci_pci_bus = 0xFF;
static uint8_t xhci_pci_slot = 0xFF;
static uint8_t xhci_pci_func = 0xFF;

// --- MMIO Access Helpers (assuming 32-bit registers) ---
// These are not generic I/O port functions; they are for memory-mapped registers.
static inline uint32_t mmio_read_dword(volatile uint8_t* base_addr, uint32_t offset) {
    return *(volatile uint32_t*)(base_addr + offset);
}

static inline void mmio_write_dword(volatile uint8_t* base_addr, uint32_t offset, uint32_t value) {
    *(volatile uint32_t*)(base_addr + offset) = value;
}

// --- XHCI Command Handlers ---

// cmd_usb_scan: Scans for XHCI controller and prints basic info
void cmd_usb_scan(const char* args) {
    terminal_writestring("USB: Scanning for XHCI controller...\n");

    // This relies on the PCI enumerator to find the XHCI controller
    // XHCI controllers typically have Class 0C (Serial Bus Controller), Subclass 03 (USB), ProgIF 30 (XHCI)
    // Or you can look for Vendor ID / Device ID if you know them (e.g., Intel 0x8086 / 0x1E31)

    // A more robust scan would iterate all PCI devices using pci_read_word/byte
    // and check Class/Subclass/ProgIF or Vendor/Device ID.
    // For this example, we assume discovery was already performed during init.

    if (xhci_mmio_base == NULL) {
        terminal_writestring("USB: XHCI Controller not found or not initialized.\n");
        return;
    }

    terminal_writestring("USB: XHCI Controller found at ");
    char buf[10]; // For printing hex addresses

    terminal_writestring("PCI ");
    uint8_to_hex_str(xhci_pci_bus, buf); terminal_writestring(buf);
    terminal_writestring(":");
    uint8_to_hex_str(xhci_pci_slot, buf); terminal_writestring(buf);
    terminal_writestring(":");
    uint8_to_hex_str(xhci_pci_func, buf); terminal_writestring(buf);
    terminal_writestring(".\n");

    terminal_writestring("     MMIO Base: 0x");
    uint64_to_hex_str((uint64_t)xhci_mmio_base, buf); terminal_writestring(buf); // Assuming uint64_to_hex_str exists
    terminal_writestring("\n");

    // Read Capability Registers (relative to xhci_mmio_base)
    uint8_t cap_length = mmio_read_dword(xhci_mmio_base, XHCI_CAPLENGTH_OFFSET) & 0xFF;
    uint32_t hcc_params1 = mmio_read_dword(xhci_mmio_base, XHCI_HCCPARAMS1_OFFSET);
    uint32_t hci_version = (mmio_read_dword(xhci_mmio_base, XHCI_CAPLENGTH_OFFSET) >> 16);

    terminal_writestring("     Cap Length: 0x");
    uint8_to_hex_str(cap_length, buf); terminal_writestring(buf);
    terminal_writestring(", HCI Version: 0x");
    uint16_to_hex_str(hci_version, buf); terminal_writestring(buf); // HCI version is 16-bit
    terminal_writestring("\n");

    // Read Operational Registers (relative to xhci_mmio_base + cap_length)
    volatile uint8_t* op_regs_base = xhci_mmio_base + cap_length;
    uint32_t usb_sts = mmio_read_dword(op_regs_base, XHCI_USBSTS_OFFSET);

    terminal_writestring("     USBSTS: 0x");
    uint32_to_hex_str(usb_sts, buf); terminal_writestring(buf); // Assuming uint32_to_hex_str exists
    terminal_writestring("\n");

    if (usb_sts & XHCI_USBSTS_HCH) {
        terminal_writestring("     Status: Halted\n");
    } else {
        terminal_writestring("     Status: Running\n");
    }
    if (usb_sts & XHCI_USBSTS_HSE) {
        terminal_writestring("     Status: Host System Error!\n");
    }
}

// Minimal XHCI Reset command (for debugging)
void cmd_usb_reset(const char* args) {
    if (xhci_mmio_base == NULL) {
        terminal_writestring("USB: XHCI Controller not initialized.\n");
        return;
    }
    terminal_writestring("USB: Resetting XHCI controller...\n");
    volatile uint8_t* op_regs_base = xhci_mmio_base + (mmio_read_dword(xhci_mmio_base, XHCI_CAPLENGTH_OFFSET) & 0xFF);

    // Set Host Controller Reset (HCRST) bit in USBCMD
    mmio_write_dword(op_regs_base, XHCI_USBCMD_OFFSET, XHCI_USBCMD_HCRST);

    // Wait for HCRST to clear (indicating reset complete) and HCH (Halted) to set
    // A timeout mechanism would be needed for robustness in a real driver
    while (mmio_read_dword(op_regs_base, XHCI_USBCMD_OFFSET) & XHCI_USBCMD_HCRST) { /* spin */ }
    while (!(mmio_read_dword(op_regs_base, XHCI_USBSTS_OFFSET) & XHCI_USBSTS_HCH)) { /* spin */ }

    // Clear all status bits by writing 1 to them
    mmio_write_dword(op_regs_base, XHCI_USBSTS_OFFSET, 0xFFFFFFFF);

    terminal_writestring("USB: XHCI Controller reset complete.\n");
    cmd_usb_scan(NULL); // Display status after reset
}


// --- XHCI Driver Initialization ---
int usb_xhci_extension_init(void) {
    terminal_writestring("USB: XHCI Extension Initializing...\n");

    // 1. Find the XHCI controller via PCI
    // This relies on pci_read_word/byte from the PCI extension.
    // The PCI enumerator would typically pass the BAR address to the driver,
    // but here we'll re-scan for it directly.
    uint32_t bar0_val = 0;
    int found_xhci = 0;
    for (uint16_t bus = 0; bus < 2; bus++) { // Scan first few buses
        for (uint8_t slot = 0; slot < 32; slot++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint16_t vendor_id = pci_read_word((uint8_t)bus, slot, func, PCI_VENDOR_ID);
                if (vendor_id == 0xFFFF) continue; // No device or function

                uint8_t class_code = pci_read_byte((uint8_t)bus, slot, func, PCI_CLASS_CODE);
                uint8_t subclass = pci_read_byte((uint8_t)bus, slot, func, PCI_SUBCLASS);
                uint8_t prog_if = pci_read_byte((uint8_t)bus, slot, func, PCI_PROG_IF);

                // XHCI Class: 0x0C (Serial Bus Controller), Subclass: 0x03 (USB), ProgIF: 0x30 (XHCI)
                if (class_code == 0x0C && subclass == 0x03 && prog_if == 0x30) {
                    xhci_pci_bus = (uint8_t)bus;
                    xhci_pci_slot = slot;
                    xhci_pci_func = func;
                    bar0_val = pci_read_dword((uint8_t)bus, slot, func, XHCI_PCI_BAR0);
                    found_xhci = 1;
                    break;
                }
            }
            if (found_xhci) break;
        }
        if (found_xhci) break;
    }

    if (!found_xhci) {
        terminal_writestring("USB: XHCI Controller not found on PCI bus.\n");
        return 1; // Failure
    }

    // Determine MMIO Base Address from BAR0
    // For 32-bit BARs, bit 0 is type indicator (0 for memory space). Bits 31:4 are address.
    // For 64-bit BARs, it's BAR0 + BAR1.
    // Assuming 32-bit memory-mapped BAR for simplicity.
    // BAR0 bit 0 = 0 (Memory Space), bit 1-2 = type (00=32bit, 10=64bit).
    // Let's assume 64-bit BAR for typical XHCI controllers for correctness.
    uint64_t bar_address = 0;
    if (bar0_val & 0x04) { // Bit 2 set means 64-bit BAR
        uint32_t bar1_val = pci_read_dword(xhci_pci_bus, xhci_pci_slot, xhci_pci_func, XHCI_PCI_BAR0 + 4);
        bar_address = (uint64_t)bar1_val << 32 | (bar0_val & ~0xF); // Mask off lower 4 bits (type)
    } else { // 32-bit BAR
        bar_address = (uint64_t)(bar0_val & ~0xF);
    }

    xhci_mmio_base = (volatile uint8_t*)bar_address;

    terminal_writestring("USB: XHCI Controller MMIO base at 0x");
    char hex_buf[20]; uint64_to_hex_str(bar_address, hex_buf); terminal_writestring(hex_buf); terminal_writestring("\n");


    // 2. Perform a Host Controller Reset (HCRST)
    terminal_writestring("USB: Performing Host Controller Reset...\n");
    volatile uint8_t* op_regs_base = xhci_mmio_base + (mmio_read_dword(xhci_mmio_base, XHCI_CAPLENGTH_OFFSET) & 0xFF);

    // Set HCRST bit in USBCMD
    mmio_write_dword(op_regs_base, XHCI_USBCMD_OFFSET, XHCI_USBCMD_HCRST);

    // Wait for HCRST to clear and HCH to set
    int timeout = 1000000; // Arbitrary timeout
    while ((mmio_read_dword(op_regs_base, XHCI_USBCMD_OFFSET) & XHCI_USBCMD_HCRST) && timeout > 0) {
        timeout--;
        asm volatile("pause"); // Hint to CPU for spin-wait
    }
    if (timeout <= 0) {
        terminal_writestring("USB: HCRST timeout!\n");
        xhci_mmio_base = NULL; return 1;
    }

    timeout = 1000000;
    while (!((mmio_read_dword(op_regs_base, XHCI_USBSTS_OFFSET) & XHCI_USBSTS_HCH)) && timeout > 0) {
        timeout--;
        asm volatile("pause");
    }
    if (timeout <= 0) {
        terminal_writestring("USB: HCH set timeout!\n");
        xhci_mmio_base = NULL; return 1;
    }

    // Clear all status bits by writing 1 to them
    mmio_write_dword(op_regs_base, XHCI_USBSTS_OFFSET, 0xFFFFFFFF);
    terminal_writestring("USB: Reset successful.\n");

    // 3. (Simplified) Enable Controller and Power On
    // For a real driver, this is where device context array, command ring, event ring,
    // interrupters, etc., would be set up.
    // For basic functionality, we just enable the run bit.

    // Enable Run/Stop bit (RS) in USBCMD
    mmio_write_dword(op_regs_base, XHCI_USBCMD_OFFSET, XHCI_USBCMD_RS);

    // Wait for HCH to clear (indicating controller is running)
    timeout = 1000000;
    while ((mmio_read_dword(op_regs_base, XHCI_USBSTS_OFFSET) & XHCI_USBSTS_HCH) && timeout > 0) {
        timeout--;
        asm volatile("pause");
    }
    if (timeout <= 0) {
        terminal_writestring("USB: Controller did not start!\n");
        xhci_mmio_base = NULL; return 1;
    }

    terminal_writestring("USB: XHCI Extension Initialized successfully. Controller running.\n");

    // Register commands
    register_command("usb_scan", cmd_usb_scan, "Scan for XHCI controller info", xhci_ext_id);
    register_command("usb_reset", cmd_usb_reset, "Reset XHCI controller", xhci_ext_id);

    return 0; // Success
}

// --- XHCI Extension Cleanup ---
void usb_xhci_extension_cleanup(void) {
    terminal_writestring("USB: XHCI Extension Cleaning up...\n");
    if (xhci_mmio_base) {
        volatile uint8_t* op_regs_base = xhci_mmio_base + (mmio_read_dword(xhci_mmio_base, XHCI_CAPLENGTH_OFFSET) & 0xFF);
        // Stop the controller
        mmio_write_dword(op_regs_base, XHCI_USBCMD_OFFSET, 0); // Clear RS bit
        // Wait for HCH to set (indicating halt)
        while (!(mmio_read_dword(op_regs_base, XHCI_USBSTS_OFFSET) & XHCI_USBSTS_HCH)) { /* spin */ }
    }
    terminal_writestring("USB: XHCI Extension Cleanup complete.\n");
}

// --- Automatic Registration Function ---
__attribute__((section(".ext_register_fns")))
void __usb_xhci_auto_register(void) {
    xhci_ext_id = register_extension("USB_XHCI", "1.0",
                                     usb_xhci_extension_init,
                                     usb_xhci_extension_cleanup);
    if (xhci_ext_id >= 0) {
        load_extension(xhci_ext_id);
    } else {
        terminal_writestring("Failed to register XHCI USB Extension (auto)!\n");
    }
}
