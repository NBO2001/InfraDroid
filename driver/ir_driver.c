#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/errno.h>

// vendor / if
#define USB_VENDOR_ID  0x10c4  // CP210x
#define USB_PRODUCT_ID 0xea60  // CP210x

#define BULK_EP_OUT 0x01
#define BULK_EP_IN  0x81
#define MAX_BUFFER_SIZE 1024

// comandos cp210x
#define CP210X_IFC_ENABLE   0x00
#define CP210X_SET_BAUD     0x1E
#define CP210X_REQ_TYPE     0x41 // Vendor | Interface | Out

struct usb_ir {
    struct usb_device *udev;
    struct class *ir_class;
};

// --- FUNÇÃO AUXILIAR: CONFIGURAR BAUD RATE (115200) ---
static int cp210x_configure(struct usb_device *udev) {
    int retval;
    u32 *baud_ptr;
    u32 baud_rate = 115200; // Velocidade desejada

    // 1. Habilitar UART (Interface Enable)
    // Envia valor 0x0001 (Enable) para o comando 0x00
    retval = usb_control_msg(udev, usb_sndctrlpipe(udev, 0),
                             CP210X_IFC_ENABLE, CP210X_REQ_TYPE,
                             0x0001, 0, NULL, 0, 1000);
    if (retval < 0) {
        printk(KERN_ERR "IR_DRIVER: Falha ao habilitar UART CP210x (%d)\n", retval);
        return retval;
    }

    // 2. Configurar Baud Rate
    // O CP210x espera um ponteiro para um inteiro de 32 bits
    baud_ptr = kmalloc(sizeof(u32), GFP_KERNEL);
    if (!baud_ptr) return -ENOMEM;

    *baud_ptr = baud_rate;

    retval = usb_control_msg(udev, usb_sndctrlpipe(udev, 0),
                             CP210X_SET_BAUD, CP210X_REQ_TYPE,
                             0, 0, baud_ptr, sizeof(u32), 1000);

    kfree(baud_ptr);

    if (retval < 0) {
        printk(KERN_ERR "IR_DRIVER: Falha ao setar Baud Rate (%d)\n", retval);
        return retval;
    }

    printk(KERN_INFO "IR_DRIVER: CP210x configurado para 115200 baud!\n");
    return 0;
}

// --- TRANSMISSÃO ---
static ssize_t transmit_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *udev = interface_to_usbdev(intf);
    int retval;
    int wrote;
    char *cmd_buffer;

    cmd_buffer = kmalloc(count + 1, GFP_KERNEL);
    if (!cmd_buffer) return -ENOMEM;

    memcpy(cmd_buffer, buf, count);
    cmd_buffer[count] = '\0';

    // printk(KERN_INFO "IR_DRIVER: Enviando %zu bytes...\n", count);

    retval = usb_bulk_msg(udev, usb_sndbulkpipe(udev, BULK_EP_OUT),
                          cmd_buffer, count, &wrote, 1000);

    kfree(cmd_buffer);
    if (retval) return retval;
    return count;
}

// --- RECEPÇÃO ---
static ssize_t receive_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *udev = interface_to_usbdev(intf);
    int retval;
    int read_count;
    char *read_buffer;

    read_buffer = kmalloc(MAX_BUFFER_SIZE, GFP_KERNEL);
    if (!read_buffer) return -ENOMEM;

    retval = usb_bulk_msg(udev, usb_rcvbulkpipe(udev, BULK_EP_IN),
                          read_buffer, MAX_BUFFER_SIZE, &read_count, 2000);

    if (retval) {
        kfree(read_buffer);
        if (retval == -ETIMEDOUT) return sprintf(buf, "TIMEOUT\n");
        return retval;
    }

    memcpy(buf, read_buffer, read_count);
    buf[read_count] = '\0';
    kfree(read_buffer);
    return read_count;
}

static DEVICE_ATTR_WO(transmit);
static DEVICE_ATTR_RO(receive);

static struct class *ir_class;

// --- PROBE (ONDE A CONFIGURAÇÃO ACONTECE) ---
static int ir_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
    struct device *dev;
    struct usb_device *udev = interface_to_usbdev(interface);
    int retval;

    // AQUI: Configura o chip para 115200 antes de qualquer coisa
    retval = cp210x_configure(udev);
    if (retval) return retval;

    // Cria Sysfs
    dev = device_create(ir_class, &interface->dev, MKDEV(0, 0), NULL, "ir_module");
    if (IS_ERR(dev)) return PTR_ERR(dev);

    retval = device_create_file(dev, &dev_attr_transmit);
    if (retval) goto error;
    retval = device_create_file(dev, &dev_attr_receive);
    if (retval) goto error;

    usb_set_intfdata(interface, dev);
    printk(KERN_INFO "IR_DRIVER: Conectado e Configurado @ 115200!\n");
    return 0;

error:
    device_destroy(ir_class, MKDEV(0, 0));
    return retval;
}

static void ir_disconnect(struct usb_interface *interface)
{
    struct device *dev = usb_get_intfdata(interface);
    device_remove_file(dev, &dev_attr_receive);
    device_remove_file(dev, &dev_attr_transmit);
    device_destroy(ir_class, MKDEV(0, 0));
    printk(KERN_INFO "IR_DRIVER: Desconectado.\n");
}

static struct usb_device_id ir_table[] = {
    { USB_DEVICE(USB_VENDOR_ID, USB_PRODUCT_ID) },
    {}
};
MODULE_DEVICE_TABLE(usb, ir_table);

static struct usb_driver ir_driver = {
    .name = "ir_driver_cp210x",
    .probe = ir_probe,
    .disconnect = ir_disconnect,
    .id_table = ir_table,
};

static int __init ir_init(void)
{
    ir_class = class_create(THIS_MODULE, "infrared");
    if (IS_ERR(ir_class)) return PTR_ERR(ir_class);
    return usb_register(&ir_driver);
}

static void __exit ir_exit(void)
{
    usb_deregister(&ir_driver);
    class_destroy(ir_class);
}

module_init(ir_init);
module_exit(ir_exit);
MODULE_LICENSE("GPL");
