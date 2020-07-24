#ifndef _LINUX_TTY_H
#define _LINUX_TTY_H

#include <linux/os.h>
#include <linux/spinlock.h>
#include <linux/wait.h>

struct tty_struct {
    int index;
    struct tty_driver *driver;
    const struct tty_operations *ops;

    int throttled : 1;

    //===== add for usb host ====
    int closing : 1;

    void *driver_data;
    spinlock_t lock;
    struct tty_port *port;
    wait_queue_head_t read_wait;
    wait_queue_head_t write_wait;

    void (*rx_cb)(void *arg, const unsigned char *chars, size_t size);
    void *rx_arg;
};

/* circular buffer */
struct tty_buf {
    unsigned		buf_size;
    char			*buf_buf;
    char			*buf_get;
    char			*buf_put;
};

/*
* Port level information. Each device keeps its own port level information
* so provide a common structure for those ports wanting to use common support
* routines.
*
* The tty port has a different lifetime to the tty so must be kept apart.
* In addition be careful as tty -> port mappings are valid for the life
* of the tty object but in many cases port -> tty mappings are valid only
* until a hangup so don't use the wrong path.
*/

struct tty_port;

struct tty_port_operations {
    /* Return 1 if the carrier is raised */
    int(*carrier_raised)(struct tty_port *port);
    /* Control the DTR line */
    void(*dtr_rts)(struct tty_port *port, int raise);
    /* Called when the last close completes or a hangup finishes
    IFF the port was initialized. Do not use to free resources. Called
    under the port mutex to serialize against activate/shutdowns */
    void(*shutdown)(struct tty_port *port);
    /* Called under the port mutex from tty_port_open, serialized using
    the port mutex */
    int(*activate)(struct tty_port *port, struct tty_struct *tty);
    /* Called on the final put of a port */
    void(*destruct)(struct tty_port *port);
};

struct tty_port {
    struct tty_struct *tty;
    struct tty_buf tty_read_buf;
    int count;
    int avail_data;
    
    spinlock_t lock;

    // add for usb host
    const struct tty_port_operations *ops; 
    struct mutex mutex;
    unsigned int		close_delay;	    /* close port delay */
    unsigned int		closing_wait;	/* delay for output */

    unsigned initialized : 1;
    unsigned active : 1;
    unsigned console : 1;
    unsigned modem : 1;
};

#if 1 // add for usb host
extern int tty_port_install(struct tty_port *port, struct tty_driver *driver, struct tty_struct *tty);
extern int tty_port_open(struct tty_port *port, struct tty_struct *tty);
extern void tty_port_close(struct tty_port *port, struct tty_struct *tty);
extern void tty_port_hangup(struct tty_port *port);
extern void tty_port_tty_hangup(struct tty_port *port, bool check_clocal);
extern void tty_port_tty_wakeup(struct tty_port *port);

static inline struct tty_struct *tty_port_tty_get(struct tty_port *port)
{
    return port->tty;
}
#endif
void tty_port_init_h(struct tty_port *port);
void tty_port_init(struct tty_port *port);
void tty_port_destroy(struct tty_port *port);


struct tty_operations {
    int(*is_ready)(struct tty_struct * tty);  // Irene Lin
    int(*install)(struct tty_driver *driver, struct tty_struct *tty);
    int(*open)(struct tty_struct * tty);
    void(*close)(struct tty_struct * tty);
    void(*cleanup)(struct tty_struct *tty);
    int(*write)(struct tty_struct * tty,
        const unsigned char *buf, int count);
    int(*put_char)(struct tty_struct *tty, unsigned char ch);
    void(*flush_chars)(struct tty_struct *tty);
    int(*write_room)(struct tty_struct *tty);
    int(*chars_in_buffer)(struct tty_struct *tty);
    int(*break_ctl)(struct tty_struct *tty, int state);
    void (*unthrottle)(struct tty_struct * tty);
    void(*hangup)(struct tty_struct *tty);
    void(*wait_until_sent)(struct tty_struct *tty, int timeout);
};

struct tty_driver {
    const char *driver_name;
    unsigned int num;
    /*
    * Pointer to the tty data structures
    */
    struct tty_struct **ttys; // add for usb host
    struct tty_port **ports;  // add for usb host
    const struct tty_operations *ops;
};

struct tty_driver *alloc_tty_driver(unsigned int lines);
void destruct_tty_driver(struct tty_driver *driver);
int tty_register_driver(struct tty_driver *driver);
int tty_unregister_driver(struct tty_driver *driver);

static inline void tty_set_operations(struct tty_driver *driver,
    const struct tty_operations *op)
{
    driver->ops = op;
}

static inline void tty_wakeup(struct tty_struct *tty)
{
    wake_up_interruptible(&tty->write_wait);
}

void __tty_hangup(struct tty_struct *tty); // add for host
void tty_hangup(struct tty_struct *tty);

#define tty_vhangup(tty)     __tty_hangup(tty);
#define tty_kref_put(tty)
#define tty_throttled(tty)  (tty->throttled)
#if 1 // add for usb host
#define tty_register_device(driver, index, device)	(device)
#define tty_unregister_device(driver, index)
#endif

int tty_insert_flip_string(struct tty_port *port,
    const unsigned char *chars, size_t size);

void tty_flip_buffer_push(struct tty_port *port);


#endif
