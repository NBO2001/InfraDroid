#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

// --- CONFIGURAÇÃO ---
// SUBSTITUA AQUI PELOS VALORES DO SEU 'lsusb'
#define USB_VENDOR_ID  0x10c4  // Exemplo (Silicon Labs)
#define USB_PRODUCT_ID 0xea60  // Exemplo (CP210x)

#define MIN(a,b) (((a) <= (b)) ? (a) : (b))
#define BULK_EP_OUT 0x01 // Geralmente o Endpoint 1 é saída (pode variar)
#define MAX_PKG_SIZE 1024 // Tamanho do buffer

// Estrutura para manter o estado do dispositivo
struct usb_ir_device {
    struct usb_device *udev;
    struct usb_class_driver class;
    unsigned char *bulk_in_buffer;
    size_t bulk_in_size;
};

// --- FUNÇÃO DE ESCRITA NO SYSFS ---
// É aqui que a mágica acontece. Quando alguém escreve no arquivo, essa função roda.
static ssize_t ir_write(struct file *f, const char __user *buf, size_t count, loff_t *off) {
    struct usb_interface *interface;
    struct usb_ir_device *dev;
    int retval = 0;
    int wrote = 0;
    char *cmd_buffer;

    // Recupera o ponteiro do dispositivo
    interface = f->private_data;
    dev = usb_get_intfdata(interface);

    // Aloca memória no Kernel para receber o comando
    cmd_buffer = kmalloc(count + 1, GFP_KERNEL);
    if (!cmd_buffer) return -ENOMEM;

    // Copia os dados do Usuário (User Space) para o Kernel
    if (copy_from_user(cmd_buffer, buf, count)) {
        kfree(cmd_buffer);
        return -EFAULT;
    }
    cmd_buffer[count] = '\0'; // Garante o fim da string

    // LOG no Kernel (dmesg) para debug
    printk(KERN_INFO "IR_DRIVER: Recebido comando: %s\n", cmd_buffer);

    // --- ENVIO USB ---
    // Envia o comando textual via Bulk Transfer para o ESP32
    // Nota: O pipe usb_sndbulkpipe calcula o endereço correto
    retval = usb_bulk_msg(dev->udev, usb_sndbulkpipe(dev->udev, BULK_EP_OUT),
                          cmd_buffer, count, &wrote, 5000); // 5000ms timeout

    if (retval) {
        printk(KERN_ERR "IR_DRIVER: Erro no envio USB: %d\n", retval);
    } else {
        printk(KERN_INFO "IR_DRIVER: %d bytes enviados ao hardware\n", wrote);
    }

    kfree(cmd_buffer);
    return count;
}

// Definição das operações do arquivo (open, release, write...)
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .write = ir_write, // Aponta para nossa função acima
};

// --- CONFIGURAÇÃO USB ---
static int ir_probe(struct usb_interface *interface, const struct usb_device_id *id) {
    struct usb_ir_device *dev;
    int retval;

    dev = kzalloc(sizeof(*dev), GFP_KERNEL);
    if (!dev) return -ENOMEM;

    dev->udev = interface_to_usbdev(interface);
    dev->class.name = "usb/ir_device%d"; // Cria /dev/usb/ir_device0 (opcional)
    dev->class.fops = &fops;

    usb_set_intfdata(interface, dev);

    // Registra o dispositivo USB
    retval = usb_register_dev(interface, &dev->class);
    if (retval < 0) {
        printk(KERN_ERR "IR_DRIVER: Falha ao obter minor number\n");
        return retval;
    }

    printk(KERN_INFO "IR_DRIVER: Dispositivo ESP32 IR conectado!\n");
    return 0;
}

static void ir_disconnect(struct usb_interface *interface) {
    struct usb_ir_device *dev;
    dev = usb_get_intfdata(interface);
    usb_deregister_dev(interface, &dev->class);
    kfree(dev);
    printk(KERN_INFO "IR_DRIVER: Dispositivo desconectado.\n");
}

// Tabela de dispositivos suportados
static struct usb_device_id ir_table[] = {
    { USB_DEVICE(USB_VENDOR_ID, USB_PRODUCT_ID) },
    {}
};
MODULE_DEVICE_TABLE(usb, ir_table);

static struct usb_driver ir_driver = {
    .name = "ir_driver_custom",
    .probe = ir_probe,
    .disconnect = ir_disconnect,
    .id_table = ir_table,
};

// --- INICIALIZAÇÃO DO MÓDULO ---
static int __init ir_init(void) {
    return usb_register(&ir_driver);
}

static void __exit ir_exit(void) {
    usb_deregister(&ir_driver);
}

module_init(ir_init);
module_exit(ir_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sua Equipe");
MODULE_DESCRIPTION("Driver USB Customizado para Emissor IR ESP32");