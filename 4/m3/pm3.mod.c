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
	{ 0xe914e41e, "strcpy" },
	{ 0x7e1805b7, "create_proc_entry" },
	{ 0x9331365d, "proc_mkdir" },
	{ 0x91715312, "sprintf" },
	{ 0xf1e98c74, "avenrun" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0xba497f13, "loops_per_jiffy" },
	{ 0xb742fd7, "simple_strtol" },
	{ 0x362ef408, "_copy_from_user" },
	{ 0xb4390f9a, "mcount" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "E765527BA06D51620E088ED");
