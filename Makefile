# In the user's Makefile:

# Find the C_SOURCES variable and add the XHCI extension file:
C_SOURCES += src/extensions/usb_xhci_extension.c # <-- Add this line


# Find the 'run' and 'debug' targets and add QEMU's USB XHCI controller:
# Note: You might want to remove other -net options if focusing purely on USB here,
# or combine them as needed for your QEMU setup.
run: all
	qemu-system-i386 -kernel $(KERNEL_BIN) -usb -device qemu-xhci # Add USB XHCI controller

debug: all
	qemu-system-i386 -s -S -kernel $(KERNEL_BIN) -usb -device qemu-xhci # Add USB XHCI controller
