/**
 * @brief   An introductory character driver. This module maps to /dev/simple_driver and
 * comes with a helper C program that can be run in Linux user space to communicate with
 * this the LKM.
 *
 * Modified from Derek Molloy (http://www.derekmolloy.ie/)
 */

#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <linux/uaccess.h>
#include <linux/slab.h>      // kmalloc/kfree
#include <linux/list.h>      // listas encadeadas do kernel

#define  DEVICE_NAME "simple_driver" ///< The device will appear at /dev/simple_driver using this value
#define  CLASS_NAME  "simple_class"        ///< The device class -- this is a character device driver
#define MAX_DATA 1024

MODULE_LICENSE("GPL");            ///< The license type -- this affects available functionality
MODULE_AUTHOR("Author Name");    ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("A generic Linux char driver.");  ///< The description -- see modinfo
MODULE_VERSION("0.2");            ///< A version number to inform users

static int    majorNumber;                  ///< Stores the device number -- determined automatically
static char   message[256] = {0};           ///< Memory for the string that is passed from userspace
static short  size_of_message;              ///< Used to remember the size of the string stored
static int    numberOpens = 0;              ///< Counts the number of times the device is opened
static struct class *charClass  = NULL; ///< The device-driver class struct pointer
static struct device *charDevice = NULL; ///< The device-driver device struct pointer

// cabeça da lista global
static LIST_HEAD(msg_list);

static char result_buf[MAX_DATA];   // resultado da última operação
static int result_size = 0;
static uint32_t key[4];             // chave XTEA
static char operation[4];           // "enc" ou "dec"

struct msg_node {
    struct list_head list;
    char *data;
    size_t len;
};

// The prototype functions for the character driver -- must come before the struct definition
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

/** @brief Devices are represented as file structure in the kernel. The file_operations structure from
 *  /linux/fs.h lists the callback functions that you wish to associated with your file operations
 *  using a C99 syntax structure. char devices usually implement open, read, write and release calls
 */
static struct file_operations fops =
{
	.open = dev_open,
	.read = dev_read,
	.write = dev_write,
	.release = dev_release,
};

/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point.
 *  @return returns 0 if successful
 */
static int __init simple_init(void){
	printk(KERN_INFO "Simple Driver: Initializing the LKM\n");

	// Try to dynamically allocate a major number for the device -- more difficult but worth it
	majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
	if (majorNumber<0){
		printk(KERN_ALERT "Simple Driver failed to register a major number\n");
		return majorNumber;
	}
	
	printk(KERN_INFO "Simple Driver: registered correctly with major number %d\n", majorNumber);

	// Register the device class
	charClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(charClass)){                // Check for error and clean up if there is
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Simple Driver: failed to register device class\n");
		return PTR_ERR(charClass);          // Correct way to return an error on a pointer
	}
	
	printk(KERN_INFO "Simple Driver: device class registered correctly\n");

	// Register the device driver
	charDevice = device_create(charClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
	if (IS_ERR(charDevice)){               // Clean up if there is an error
		class_destroy(charClass);           // Repeated code but the alternative is goto statements
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Simple Driver: failed to create the device\n");
		return PTR_ERR(charDevice);
	}
	
	printk(KERN_INFO "Simple Driver: device class created correctly\n"); // Made it! device was initialized
		
	return 0;
}

static void __exit simple_exit(void){
    struct msg_node *node, *tmp;

    // esvazia a lista antes de sair
    list_for_each_entry_safe(node, tmp, &msg_list, list) {
        list_del(&node->list);
        kfree(node->data);
        kfree(node);
    }

    device_destroy(charClass, MKDEV(majorNumber, 0));
    class_unregister(charClass);
    class_destroy(charClass);
    unregister_chrdev(majorNumber, DEVICE_NAME);
    printk(KERN_INFO "Simple Driver: goodbye from the LKM!\n");
}

/** @brief The device open function that is called each time the device is opened
 *  This will only increment the numberOpens counter in this case.
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_open(struct inode *inodep, struct file *filep){
	numberOpens++;
	printk(KERN_INFO "Simple Driver: device has been opened %d time(s)\n", numberOpens);
	return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
    struct msg_node *node;
    int ret;

    if (list_empty(&msg_list)) {
        printk(KERN_INFO "Simple Driver: no messages to read\n");
        return 0; // nada para ler
    }

    node = list_first_entry(&msg_list, struct msg_node, list);

    if (len < node->len)
        return -EINVAL; // buffer do user pequeno demais

    if (copy_to_user(buffer, node->data, node->len)) {
        return -EFAULT;
    }

    ret = node->len;

    // Remove da lista e libera
    list_del(&node->list);
    kfree(node->data);
    kfree(node);

    printk(KERN_INFO "Simple Driver: sent %d chars to user\n", ret);
    return ret;
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
    struct msg_node *node;

    // Aloca nó
    node = kmalloc(sizeof(*node), GFP_KERNEL);
    if (!node) return -ENOMEM;

    node->data = kmalloc(len + 1, GFP_KERNEL);
    if (!node->data) {
        kfree(node);
        return -ENOMEM;
    }

    if (copy_from_user(node->data, buffer, len)) {
        kfree(node->data);
        kfree(node);
        return -EFAULT;
    }

    node->data[len] = '\0';  // string terminada
    node->len = len;

    // adiciona no fim da lista
    list_add_tail(&node->list, &msg_list);

    printk(KERN_INFO "Simple Driver: received %zu chars, stored in list\n", len);
    return len;
}

/** @brief The device release function that is called whenever the device is closed/released by
 *  the userspace program
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_release(struct inode *inodep, struct file *filep){
	printk(KERN_INFO "Simple Driver: device successfully closed\n");
	return 0;
}

/** @brief A module must use the module_init() module_exit() macros from linux/init.h, which
 *  identify the initialization function at insertion time and the cleanup function (as
 *  listed above)
 */
module_init(simple_init);
module_exit(simple_exit);

// Funções de cifra/decifra XTEA
static void encipher(uint32_t num_rounds, uint32_t v[2], const uint32_t key[4]){
    uint32_t i, v0=v[0], v1=v[1], sum=0, delta=0x9E3779B9;
    for(i=0; i<num_rounds; i++){
        v0 += (((v1<<4) ^ (v1>>5)) + v1) ^ (sum + key[sum & 3]);
        sum += delta;
        v1 += (((v0<<4) ^ (v0>>5)) + v0) ^ (sum + key[(sum>>11) & 3]);
    }
    v[0]=v0; v[1]=v1;
}

static void decipher(uint32_t num_rounds, uint32_t v[2], const uint32_t key[4]){
    uint32_t i, v0=v[0], v1=v[1], delta=0x9E3779B9, sum=delta* num_rounds;
    for(i=0; i<num_rounds; i++){
        v1 -= (((v0<<4) ^ (v0>>5)) + v0) ^ (sum + key[(sum>>11) & 3]);
        sum -= delta;
        v0 -= (((v1<<4) ^ (v1>>5)) + v1) ^ (sum + key[sum & 3]);
    }
    v[0]=v0; v[1]=v1;
}
