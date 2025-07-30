#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/io.h>       // 添加资源地址处理头文件
#include <linux/stdlib.h>   // 添加system()函数声明

#define DRV_NAME "my_dt_driver"
#define EXPECTED_IRQ 66     // 严格匹配设备树配置

struct my_device_data {
    struct device *dev;
    int irq;
    void __iomem *regs;
};

static irqreturn_t my_irq_handler(int irq, void *dev_id)
{
    struct my_device_data *data = dev_id;
    dev_info(data->dev, "IRQ %d handled (regs@%pa)\n", 
             irq, &data->regs);
    return IRQ_HANDLED;
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
    
    /* 3. 获取中断资源 */
    irq = platform_get_irq(pdev, 0);
    if (irq != EXPECTED_IRQ) {
        dev_err(dev, "IRQ mismatch! DT:%d Driver:%d\n",
                EXPECTED_IRQ, irq);
        return -EINVAL;
    }
    data->irq = irq;
    
    /* 4. 映射寄存器 */
    data->regs = devm_ioremap_resource(dev, res);
    if (IS_ERR(data->regs)) {
        dev_err(dev, "Failed to map registers\n");
        return PTR_ERR(data->regs);
    }
    
    dev_info(dev, "Device probed at 0x%llx (IRQ %d)\n",
             (unsigned long long)res->start, irq);
    
    /* 5. 注册中断 */
    ret = devm_request_irq(dev, irq, my_irq_handler,
                          IRQF_TRIGGER_HIGH,
                          DRV_NAME, data);
    if (ret) {
        dev_err(dev, "IRQ %d register failed: %d\n", irq, ret);
        
        /* 改进的错误诊断 */
        if (ret == -EINVAL) {
            dev_err(dev, "Invalid parameters for IRQ %d\n", irq);
        } else if (ret == -EBUSY) {
            dev_err(dev, "IRQ %d conflict detected\n", irq);
            pr_info("Current interrupt assignments:\n");
            pr_info("================================\n");
            /* 使用内核API替代system()调用 */
            if (proc_create_single("interrupts", 0, NULL, show_interrupts) == NULL) {
                pr_err("Cannot display interrupt info\n");
            }
        }
        return ret;
    }
    
    platform_set_drvdata(pdev, data);
    dev_info(dev, "Driver initialized successfully\n");
    return 0;
}

static int my_remove(struct platform_device *pdev)
{
    dev_info(&pdev->dev, "Driver removed\n");
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
MODULE_DESCRIPTION("Optimized DT driver for i.MX6ULL");
