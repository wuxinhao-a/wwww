#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/version.h>
#include <linux/sysfs.h>
#include <linux/device.h>

#define DRV_NAME "my_dt_driver"
#define EXPECTED_IRQ 66
#define PROC_DIR_NAME "my_driver"
#define PROC_FILE_NAME "registers"

struct my_device_data {
    struct device *dev;
    int irq;
    void __iomem *regs;
    struct resource *mem_res;
    struct proc_dir_entry *proc_dir;
};

// 先声明 sysfs_status_show 函数
static ssize_t sysfs_status_show(struct device *dev, 
                                 struct device_attribute *attr, 
                                 char *buf);

// 定义 sysfs 属性
static struct device_attribute dev_attr_status = 
    __ATTR(status, 0444, sysfs_status_show, NULL);

// 定义 sysfs 属性组
static struct attribute *dev_attrs[] = {
    &dev_attr_status.attr,
    NULL
};

static const struct attribute_group dev_attr_group = {
    .attrs = dev_attrs,
};

// SysFS 属性显示函数实现
static ssize_t sysfs_status_show(struct device *dev, 
                                 struct device_attribute *attr, 
                                 char *buf)
{
    struct platform_device *pdev = to_platform_device(dev);
    struct my_device_data *data = platform_get_drvdata(pdev);
    ssize_t count = 0;
    
    if (!data) {
        return snprintf(buf, PAGE_SIZE, "Error: No device data available\n");
    }
    
    // 显示设备信息
    count += snprintf(buf + count, PAGE_SIZE - count, 
                     "Driver: %s\n", DRV_NAME);
    count += snprintf(buf + count, PAGE_SIZE - count, 
                     "Device: %s\n", dev_name(data->dev));
    
    // 显示寄存器信息
    if (data->mem_res) {
        count += snprintf(buf + count, PAGE_SIZE - count, 
                         "\nMemory Resources:\n");
        count += snprintf(buf + count, PAGE_SIZE - count, 
                         "  Physical Address: 0x%08llx\n", 
                         (unsigned long long)data->mem_res->start);
        count += snprintf(buf + count, PAGE_SIZE - count, 
                         "  Size: %lld bytes\n", resource_size(data->mem_res));
        count += snprintf(buf + count, PAGE_SIZE - count, 
                         "  Mapped Address: %pK\n", data->regs);
    }
    
    // 显示中断信息
    count += snprintf(buf + count, PAGE_SIZE - count, 
                     "\nInterrupt:\n");
    count += snprintf(buf + count, PAGE_SIZE - count, 
                     "  IRQ: %d (Registration Disabled)\n", data->irq);
    
    return count;
}

static int proc_registers_show(struct seq_file *m, void *v)
{
    struct my_device_data *data = m->private;
    
    if (!data) {
        seq_puts(m, "Error: No device data available\n");
        return 0;
    }
    
    // 显示设备信息
    seq_printf(m, "Driver: %s\n", DRV_NAME);
    seq_printf(m, "Device: %s\n", dev_name(data->dev));
    
    // 显示寄存器信息
    if (data->mem_res) {
        seq_printf(m, "\nMemory Resources:\n");
        seq_printf(m, "  Physical Address: 0x%08llx\n", 
                   (unsigned long long)data->mem_res->start);
        seq_printf(m, "  Size: %lld bytes\n", resource_size(data->mem_res));
        seq_printf(m, "  Mapped Address: %pK\n", data->regs);
    }
    
    // 显示中断信息
    seq_printf(m, "\nInterrupt:\n");
    seq_printf(m, "  IRQ: %d (Registration Disabled)\n", data->irq);
    
    return 0;
}

static int proc_registers_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_registers_show, PDE_DATA(inode));
}

static const struct file_operations proc_registers_fops = {
    .owner = THIS_MODULE,
    .open = proc_registers_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release,
};

static int create_proc_entry(struct my_device_data *data)
{
    // 创建 /proc 目录
    data->proc_dir = proc_mkdir(PROC_DIR_NAME, NULL);
    if (!data->proc_dir) {
        dev_err(data->dev, "Failed to create /proc/%s\n", PROC_DIR_NAME);
        return -ENOMEM;
    }
    
    // 创建 /proc 文件
    struct proc_dir_entry *proc_file;
    
    // 使用 proc_create_data 并传递数据指针
    proc_file = proc_create_data(PROC_FILE_NAME, 0444, data->proc_dir,
                                 &proc_registers_fops, data);
    if (!proc_file) {
        proc_remove(data->proc_dir);
        dev_err(data->dev, "Failed to create /proc/%s/%s\n", 
                PROC_DIR_NAME, PROC_FILE_NAME);
        return -ENOMEM;
    }
    
    dev_info(data->dev, "Created /proc/%s/%s for debugging\n", 
             PROC_DIR_NAME, PROC_FILE_NAME);
    return 0;
}

static int my_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct my_device_data *data;
    struct resource *res;
    int irq, ret;
    
    /* 1. 分配设备私有数据 */
    data = devm_kzalloc(dev, sizeof(*data), GFP_KERNEL);
    if (!data)
        return -ENOMEM;
    
    /* 2. 获取内存资源 */
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!res) {
        dev_err(dev, "No memory resource\n");
        return -ENXIO;
    }
    data->mem_res = res;
    
    /* 3. 获取中断资源 */
    irq = platform_get_irq(pdev, 0);
    data->irq = irq;
    data->dev = dev;
    
    /* 4. 映射寄存器 */
    data->regs = devm_ioremap_resource(dev, res);
    if (IS_ERR(data->regs)) {
        dev_err(dev, "Failed to map registers\n");
        return PTR_ERR(data->regs);
    }
    
    dev_info(dev, "Device probed at 0x%llx (IRQ %d - registration disabled)\n",
             (unsigned long long)res->start, irq);
    
    /* 5. 创建 proc 接口 */
    ret = create_proc_entry(data);
    if (ret) {
        dev_err(dev, "Failed to create proc interface (%d)\n", ret);
        return ret;
    }
    
    /* 6. 创建 sysfs 接口 */
    ret = sysfs_create_group(&dev->kobj, &dev_attr_group);
    if (ret) {
        dev_err(dev, "Failed to create sysfs group (%d)\n", ret);
        if (data->proc_dir) {
            proc_remove(data->proc_dir);
        }
        return ret;
    }
    
    platform_set_drvdata(pdev, data);
    dev_info(dev, "Driver initialized successfully (without IRQ)\n");
    dev_info(dev, "Access device info via:\n");
    dev_info(dev, "  /proc/%s/%s\n", PROC_DIR_NAME, PROC_FILE_NAME);
    dev_info(dev, "  /sys/devices/platform/%s/status\n", dev_name(dev));
    
    return 0;
}

static int my_remove(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct my_device_data *data = platform_get_drvdata(pdev);
    
    if (data) {
        // 移除 proc 文件
        if (data->proc_dir) {
            proc_remove(data->proc_dir);
            dev_info(dev, "Removed /proc/%s\n", PROC_DIR_NAME);
        }
    }
    
    // 移除 sysfs 属性
    sysfs_remove_group(&dev->kobj, &dev_attr_group);
    
    dev_info(dev, "Driver removed (IRQ was never registered)\n");
    return 0;
}

static const struct of_device_id my_of_match[] = {
    { .compatible = "vendor,my-device" },
    {},
};
MODULE_DEVICE_TABLE(of, my_of_match);

static struct platform_driver my_driver = {
    .driver = {
        .name = DRV_NAME,
        .of_match_table = my_of_match,
    },
    .probe = my_probe,
    .remove = my_remove,
};
module_platform_driver(my_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("DT driver for i.MX6ULL with proc and sysfs interfaces");
MODULE_VERSION("1.5");
