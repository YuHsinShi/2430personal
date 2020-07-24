#ifndef _MODULE_H_
#define _MODULE_H_


#include "ite/itp.h"
#include "errno.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* <linux/export.h>   */
#define THIS_MODULE ((struct module *)0)  

/* <linux/kmod.h>   */
static inline int request_module(const char *name, ...) { return 0; /*-ENOSYS;*/ }
static inline int request_module_nowait(const char *name, ...) { return 0; /*-ENOSYS;*/ }
#define try_then_request_module(x, mod...) (x)


/* <linux/module.h>   */
struct module;
struct notifier_block;


static inline struct module *__module_address(unsigned long addr)
{
    return NULL;
}

static inline struct module *__module_text_address(unsigned long addr)
{
    return NULL;
}

static inline bool is_module_address(unsigned long addr)
{
    return false;
}

static inline bool is_module_percpu_address(unsigned long addr)
{
    return false;
}

static inline bool __is_module_percpu_address(unsigned long addr, unsigned long *can_addr)
{
    return false;
}

static inline bool is_module_text_address(unsigned long addr)
{
    return false;
}

/* Get/put a kernel symbol (calls should be symmetric) */
#define symbol_get(x) ({ extern typeof(x) x __attribute__((weak)); &(x); })
#define symbol_put(x) do { } while (0)
#define symbol_put_addr(x) do { } while (0)

static inline void __module_get(struct module *module)
{
}

static inline bool try_module_get(struct module *module)
{
    return true;
}

static inline void module_put(struct module *module)
{
}

#define module_name(mod) "kernel"

/* For kallsyms to ask for address resolution.  NULL means not found. */
static inline const char *module_address_lookup(unsigned long addr,
    unsigned long *symbolsize,
    unsigned long *offset,
    char **modname,
    char *namebuf)
{
    return NULL;
}

static inline int lookup_module_symbol_name(unsigned long addr, char *symname)
{
    return -ERANGE;
}

static inline int lookup_module_symbol_attrs(unsigned long addr, unsigned long *size, unsigned long *offset, char *modname, char *name)
{
    return -ERANGE;
}

static inline int module_get_kallsym(unsigned int symnum, unsigned long *value,
    char *type, char *name,
    char *module_name, int *exported)
{
    return -ERANGE;
}

static inline unsigned long module_kallsyms_lookup_name(const char *name)
{
    return 0;
}

static inline int module_kallsyms_on_each_symbol(int(*fn)(void *, const char *,
struct module *,
    unsigned long),
    void *data)
{
    return 0;
}

static inline int register_module_notifier(struct notifier_block *nb)
{
    /* no events will happen anyway, so this can always succeed */
    return 0;
}

static inline int unregister_module_notifier(struct notifier_block *nb)
{
    return 0;
}

#define module_put_and_exit(code) do_exit(code)

static inline void print_modules(void)
{
}

static inline bool module_requested_async_probing(struct module *module)
{
    return false;
}



#ifdef __cplusplus
}
#endif

#endif // _MODULE_H_
