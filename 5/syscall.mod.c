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
	{ 0x268cc6a2, "sys_close" },
	{ 0xd85df7d4, "boot_cpu_data" },
	{ 0xba497f13, "loops_per_jiffy" },
	{ 0x50eedeb8, "printk" },
	{ 0x131ff057, "current_task" },
	{ 0x2f287f0d, "copy_to_user" },
	{ 0x1d2e87c6, "do_gettimeofday" },
	{ 0xb4390f9a, "mcount" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "D23A6DC8CE800D0880DFAA4");
