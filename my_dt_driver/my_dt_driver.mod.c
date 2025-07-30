#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x9a454969, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0xfff289f3, __VMLINUX_SYMBOL_STR(single_release) },
	{ 0xc2910861, __VMLINUX_SYMBOL_STR(seq_read) },
	{ 0x5971e172, __VMLINUX_SYMBOL_STR(seq_lseek) },
	{ 0x762124b9, __VMLINUX_SYMBOL_STR(platform_driver_unregister) },
	{ 0x8b93010a, __VMLINUX_SYMBOL_STR(__platform_driver_register) },
	{ 0x4eb1a083, __VMLINUX_SYMBOL_STR(seq_puts) },
	{ 0xc89de85, __VMLINUX_SYMBOL_STR(seq_printf) },
	{ 0x59435cb7, __VMLINUX_SYMBOL_STR(single_open) },
	{ 0x929dbad3, __VMLINUX_SYMBOL_STR(PDE_DATA) },
	{ 0xefd442fe, __VMLINUX_SYMBOL_STR(dev_err) },
	{ 0x19d3b7d8, __VMLINUX_SYMBOL_STR(sysfs_create_group) },
	{ 0x521b39ba, __VMLINUX_SYMBOL_STR(proc_create_data) },
	{ 0xfe2d8537, __VMLINUX_SYMBOL_STR(proc_mkdir) },
	{ 0x10e44450, __VMLINUX_SYMBOL_STR(devm_ioremap_resource) },
	{ 0x478bed9b, __VMLINUX_SYMBOL_STR(platform_get_irq) },
	{ 0xb29ebba7, __VMLINUX_SYMBOL_STR(platform_get_resource) },
	{ 0x879e479b, __VMLINUX_SYMBOL_STR(devm_kmalloc) },
	{ 0xb81960ca, __VMLINUX_SYMBOL_STR(snprintf) },
	{ 0xefd6cf06, __VMLINUX_SYMBOL_STR(__aeabi_unwind_cpp_pr0) },
	{ 0xb1ea493e, __VMLINUX_SYMBOL_STR(sysfs_remove_group) },
	{ 0xba3b4911, __VMLINUX_SYMBOL_STR(_dev_info) },
	{ 0xd01119a9, __VMLINUX_SYMBOL_STR(proc_remove) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("of:N*T*Cvendor,my-device");
MODULE_ALIAS("of:N*T*Cvendor,my-deviceC*");

MODULE_INFO(srcversion, "A0F27D5960B7B1A8C6C93ED");
