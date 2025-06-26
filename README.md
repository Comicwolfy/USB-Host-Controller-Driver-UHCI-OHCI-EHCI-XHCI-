# USB Host Controller Driver (XHCI) Extension for Core-Kernel:

This document is all about the USB Host Controller Driver (XHCI) Extension. It's a needed addition to Core-Kernel.
Getting Started (Overview)

So, the Universal Serial Bus (USB) Host Controller Interface (XHCI) is like the modern way to handle all your USB stuff. These XHCI controllers, which are pretty common in today's computers, help us talk super fast to all sorts of USB things – like your keyboard, mouse, storage devices, and other special gadgets. Adding this driver really makes the kernel much better at working with hardware!

Now, just a heads-up: building a full USB system, including XHCI, is actually pretty tricky. That's because the USB rules are quite complicated, and these controllers have some fancy ways of working (like using special memory areas and advanced ways of handling interruptions). Plus, you need smart software to manage finding, setting up, and sending data to devices.
What It Does (Functionality)

## This extension gives us a basic XHCI driver, and it can do a few main things:

    Finds the Controller: It uses our existing PCI Bus Enumerator to find the XHCI controller on the computer's PCI bus. These controllers usually have specific codes that help us spot them.

    Starts It Up (Just the Basics): It runs a small set of commands to get the XHCI controller into a basic working state. This includes giving the controller a quick reset and telling it to start running.

    Reports Its Status: It offers simple commands to check and show you how the XHCI controller is doing.

## A Quick Note on What It Doesn't Do Yet: This driver is really just a very simple starting point. It definitely does not do these things yet:

    Find All USB Devices: It can't currently find or set up individual USB devices (like your flash drives, keyboards, or mice) that you plug into the computer's ports.

    Send Data: It doesn't support the different ways USB devices send and receive data (like Control, Bulk, Interrupt, or Isochronous transfers), so you can't actually exchange information with devices yet.

    Handle Interruptions Properly: The driver isn't fully set up to handle the special way XHCI controllers send interruptions (like something called "Message Signaled Interrupts"). It mostly relies on just checking things regularly or simple acknowledgements. A complete driver would need a more advanced system for managing interruptions.

    Manage Memory Fully: It doesn't have a fancy way to handle all the complex memory structures the XHCI controller uses internally.

So, this driver is really just a first step to show you how Core-Kernel can talk directly to an XHCI controller. We'll need a lot more work to get a full USB system going!
Commands You Can Use

## This extension adds a couple of handy commands for you to use in the kernel's command line:

    usb_scan

        What it does: It looks for the XHCI controller and then shows you where it is on the PCI bus, its memory address, and its current status.

        How to use it: Just type usb_scan.

        What you might see:

        USB: XHCI Controller found at PCI 0:1:0.
             MMIO Base: 0xFED00000
             Cap Length: 0x10, HCI Version: 0x100
             USBSTS: 0x0
             Status: Running


        (Keep in mind, the exact details you see might change depending on if you're using a virtual machine or a real computer.)

    usb_reset

        What it does: This command tells the XHCI controller to do a hardware reset. Be careful with this one, as it will clear out the controller's current settings!

        How to use it: Just type usb_reset.

## How to Get It Running (Integration)

### To add the USB Host Controller Driver (XHCI) Extension to your Core-Kernel, here's what you need to do:

    Put the Code File in Place:

        Take the usb_xhci_extension.c file and put it in your kernel's src/extensions/ directory.

    Update Your Makefile:

        Open up the Makefile (it's in your Core-Kernel project's main folder).

        Find the C_SOURCES list (that's where all the C files for compiling are) and add the path to your new extension's file.

        # Example from your Makefile
        C_SOURCES += src/extensions/usb_xhci_extension.c # Add this line!

        Also, you need to update the run and debug commands in your Makefile. This tells QEMU to include an XHCI controller when it starts your kernel.

        # Example from your Makefile
        run: all
        	qemu-system-i386 -kernel $(KERNEL_BIN) -usb -device qemu-xhci

        debug: all
        	qemu-system-i386 -s -S -kernel $(KERNEL_BIN) -usb -device qemu-xhci

    Shared Helper Functions (Super Important!):
    The usb_xhci_extension.c code uses some helper functions (uint32_to_hex_str, uint64_to_hex_str) to change numbers into those hex strings. To keep things neat and reusable across your kernel, it's really recommended that these functions are:

        Declared: In your includes/base_kernel.h file.

        Defined: In a separate, common utility file (like src/utils.c).

        Included: That src/utils.c file also needs to be added to your Makefile's C_SOURCES list.

        Clean Up: Once you set up these shared helper functions, you should remove any local or old versions of uint32_to_hex_str and uint64_to_hex_str from usb_xhci_extension.c (and any other modules that might have them). This way, everyone uses the same, official versions, which is much better!

After you follow these steps and successfully rebuild your kernel, the usb_scan and usb_reset commands will be ready to go in your Core-Kernel's command line!
