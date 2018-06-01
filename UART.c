/* A simple character device, "UART", that allows a character to be read */
/* or written from UART */

/* Necessary includes for device drivers */
#include <stdint.h>
#include <linux/init.h>
// #include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h> /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h> /* O_ACCMODE */
#include <asm/system.h> /* cli(), *_flags */
#include <asm/uaccess.h> /* copy_from/to_user */
#include<asm/io.h> /* for ioread and iowrite*/

MODULE_LICENSE("Dual BSD/GPL");


/* Declaration of memory.c functions */
int memory_open(struct inode *inode, struct file *filp);
int memory_release(struct inode *inode, struct file *filp);
ssize_t memory_read(pl011_T* uart,  struct file *filp, char *buf, size_t count, loff_t *f_pos);
ssize_t memory_write(pl011_T* uart, struct file *filp, char *buf, size_t count, loff_t *f_pos);
void memory_exit(void);
int memory_init(void);

/* Structure that declares the usual file */
/* access functions */
struct file_operations memory_fops = {
  read:  memory_read,
  write:  memory_write,
  open:  memory_open,
  release:  memory_release
};


/* Declaration of the init and exit functions */
module_init(memory_init);
module_exit(memory_exit);


/* Global variables of the driver */
/* Major number */
int memory_major = 60;
/* Buffer to store data */
char *memory_buffer;

#define UART_2_BASE_ADDR 0x101f2000
   typedef volatile struct
  {
  uint32_t DR;		/* data register */
  uint32_t RSR_ECR;	/* receive status or error clear */
  uint8_t reserved1[0x10];/* reserved space */
  const uint32_t FR;	/* flag register */
  uint8_t reserved2[0x4];/* reserved space */
  uint32_t LPR;		/* IrDA low-poer */
  uint32_t IBRD;	/* integer baud register */
  uint32_t FBRD;	/* fractional baud register */
  uint32_t LCR_H;	/* line control register */
  uint32_t CR;		/* control register */
  uint32_t IFLS;	/* interrupt FIFO level select */
  uint32_t IMSC;	/* interrupt mask set/clear */
  const uint32_t RIS;	/* raw interrupt register */
  const uint32_t MIS;	/* masked interrupt register */
  uint32_t ICR;		/* interrupt clear register */
  uint32_t DMACR;	/* DMA control register */
  } pl011_T;
 
 enum 
 {
  TXFE = 0x80,
  RXFF = 0x40,
 };

  
pl011_T * const UART1 = (pl011_T *)UART_2_BASE_ADDR; // constant pointer to pl011

int memory_init(void)
{

 pUart = ioremap(UART_2_BASE_ADDR, 4095);
 if (pUart==NULL)
 {
     printk("Couldn't remap UART 2 MMIO\n");
     return -EIO;
 }

 int result;

  /* Registering device */
  result = register_chrdev(memory_major, "memory", &memory_fops);
  if (result < 0) 
  {
    printk(KERN_ERR "mem_dev: cannot obtain major number %d\n", memory_major);
    return result;
  }

  /* Allocating memory for the buffer */
  memory_buffer = kmalloc(1, GFP_KERNEL);
  printk("I got: %zu bytes of memory\n", ksize(memory_buffer)); 
  if (!memory_buffer) { 
    result = -ENOMEM;
    goto fail; 
  } 
  memset(memory_buffer, 0, 1);

  printk(KERN_ERR "Inserting memory module\n"); 
  return 0;

  fail: 
    memory_exit(); 
    return result;

}

/* Memory device exit function */

void memory_exit(void) 
{
  /* Freeing the major number */
  unregister_chrdev(memory_major, "memory");

  /* Freeing buffer memory */
  if (memory_buffer) {
    kfree(memory_buffer);
  }

  printk(KERN_ERR "Removing memory module\n");

}
 
 /* Memory device open function */
int memory_open(struct inode *inode, struct file *filp)
{ 
	
  numberOpens++;
  printk(KERN_INFO "Device has been opened %d time(s)\n", numberOpens);

  /* Success */
  return 0;
}

/* Memory device release function */
int memory_release(struct inode *inode, struct file *filp)
{
 
  /* Success */
  return 0;
}


/* Memory device read function */
ssize_t memory_read(pl011_T* uart,  struct file *filp, char *buf, 
                    size_t count, loff_t *f_pos)
{
 ssize_t retval = 0; 
  while((ioread32(uart->FR) & RXFF) == 1)
{   
  memory_buffer = ioread8(uart->DR); 
   
  if (copy_to_user(buf,memory_buffer,1)) 
  {
	retval = -EFAULT;
	goto out;
 
  }
 
  if (*f_pos == 0) {
 	*f_pos += 1;
 	retval = 1;
  }
 else
	retval = 0;

out:
return retval;
}
}


/* Memory device write function */
ssize_t memory_write( pl011_T* uart, struct file *filp, char *buf,
                      size_t count, loff_t *f_pos) 
{
 while((ioread32(uart->FR) & TXFE) == 1)
 {   

  ssize_t retval = 0;
  char *temp;

  temp = buf + count - 1;

  if (copy_from_user(memory_buffer,temp,1)) {
	retval = -EFAULT;
	goto out;
 }
 retval = 1;

iowrite8(memory_buffer, (uart->DR));


 out:
  return retval;
}

}
