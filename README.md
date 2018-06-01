# UART_driver

Linux device driver for the UART terminals on the emulated QEMU VersatilePB board. This is a simplified character driver which reads and writes to the UART data register by polling.

Since it is ARM, all I/O access is memory mapped.

Function ioremap in the initialization function is used to assign virtual addresses to the memory regions.

Kernel functions ioread32 and iowrite32 are used to read and write to these addresses respectively.
