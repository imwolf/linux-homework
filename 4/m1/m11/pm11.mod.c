#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x5eadf54a, "module_layout" },
	{ 0x3ec8886f, "param_ops_int" },
	{ 0x59528ec6, "remove_proc_entry" },
	{ 0x50eedeb8, "printk" },
	{ 0x7e1805b7, "create_proc_entry" },
	{ 0x9331365d, "proc_mkdir" },
	{ 0x91715312, "sprintf" },
	{ 0x37e74642, "get_jiffies_64" },
	{ 0x7d11c268, "jiffies" },
	{ 0x1d2e87c6, "do_gettimeofday" },
	{ 0x362ef408, "_copy_from_user" },
	{ 0xb4390f9a, "mcount" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "9EEBD08A0BDF3E3B8725C72");
