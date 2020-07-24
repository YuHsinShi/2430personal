#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <linux/os.h>
#include <linux/errno.h>
#include <linux/spinlock.h>
#include <linux/util.h>
#include <linux/wait.h>
#include <linux/err.h>

#include "tty.h"
#include "ite/ite_acm.h"

#if 1
#define __func_enter__ 
#define __func_leave__ 
#else
#define __func_enter__ ithPrintf("%s enter \n", __func__);
#define __func_leave__ ithPrintf("%s leave \n", __func__);
#endif

#define TTY_READ_BUF_SIZE       16*1024

static struct tty_struct g_tty[4];  /* 0/1 for gadget, 2/3 for host */

/* do once and never be released */
struct tty_driver *alloc_tty_driver(unsigned int lines)
{
    int err = 0;
    struct tty_driver *driver;

    if (!lines)
        return ERR_PTR(-EINVAL);

    driver = kzalloc(sizeof(struct tty_driver), GFP_KERNEL);
    if (!driver)
        return ERR_PTR(-ENOMEM);

    driver->num = lines;

    driver->ttys = kcalloc(lines, sizeof(*driver->ttys), GFP_KERNEL);
    if (!driver->ttys) {
        err = -ENOMEM;
        goto err_free_all;
    }

    driver->ports = kcalloc(lines, sizeof(*driver->ports), GFP_KERNEL);
    if (!driver->ports) {
        err = -ENOMEM;
        goto err_free_all;
    }

    return driver;

err_free_all:
    kfree(driver->ports);
    kfree(driver->ttys);
    kfree(driver);
    return ERR_PTR(err);
}

void destruct_tty_driver(struct tty_driver *driver)
{
    kfree(driver->ports);
    kfree(driver->ttys);
    kfree(driver);
}

int tty_register_driver(struct tty_driver *driver)
{
    struct tty_struct *tty = NULL;

    /* for usb gadget */
    if (!strcmp(driver->driver_name, "g_serial"))
        tty = g_tty + TTY_USBD_ACM;

    /* for usb host */
    if (!strcmp(driver->driver_name, "usbserial"))
        tty = g_tty + TTY_USB_MODEM;

    if (!tty) {
        printf("tty_register_driver(): no match!! \n");
        return -1;
    }

    tty->index = 0;   // means minor (port index), currently only one port
    tty->driver = driver;
    tty->ops = driver->ops;
    init_waitqueue_head(&tty->write_wait);
    init_waitqueue_head(&tty->read_wait);

    return 0;
}

int tty_unregister_driver(struct tty_driver *driver)
{
    struct tty_struct *tty = NULL;

    if (!strcmp(driver->driver_name, "g_serial"))
        tty = g_tty + TTY_USBD_ACM;

    if (!strcmp(driver->driver_name, "usbserial"))
        tty = g_tty + TTY_USB_MODEM;

    if (!tty) {
        printf("tty_unregister_driver(): no match!! \n");
        return -1;
    }

    tty->driver = NULL;
    tty->ops = NULL;

    return 0;
}

/*-------------------------------------------------------------------------*/

/* Circular Buffer */

/*
* tty_buf_alloc
*
* Allocate a circular buffer and all associated memory.
*/
static int tty_buf_alloc(struct tty_buf *tb, unsigned size)
{
    tb->buf_buf = kmalloc(size, GFP_KERNEL);
    if (tb->buf_buf == NULL)
        return -ENOMEM;

    tb->buf_size = size;
    tb->buf_put = tb->buf_buf;
    tb->buf_get = tb->buf_buf;

    return 0;
}

/*
* tty_buf_free
*
* Free the buffer and all associated memory.
*/
static void tty_buf_free(struct tty_buf *tb)
{
    kfree(tb->buf_buf);
    tb->buf_buf = NULL;
}

/*
* tty_buf_clear
*
* Clear out all data in the circular buffer.
*/
static void tty_buf_clear(struct tty_buf *tb)
{
    tb->buf_get = tb->buf_put;
    /* equivalent to a get of all data available */
}

/*
* tty_buf_data_avail
*
* Return the number of bytes of data written into the circular
* buffer.
*/
static unsigned tty_buf_data_avail(struct tty_buf *tb)
{
    return (tb->buf_size + tb->buf_put - tb->buf_get) % tb->buf_size;
}

/*
* tty_buf_space_avail
*
* Return the number of bytes of space available in the circular
* buffer.
*/
static unsigned tty_buf_space_avail(struct tty_buf *tb)
{
    return (tb->buf_size + tb->buf_get - tb->buf_put - 1) % tb->buf_size;
}

/*
* tty_buf_put
*
* Copy data data from a user buffer and put it into the circular buffer.
* Restrict to the amount of space available.
*
* Return the number of bytes copied.
*/
static unsigned
tty_buf_put(struct tty_buf *tb, const char *buf, unsigned count)
{
    unsigned len;

    len = tty_buf_space_avail(tb);
    if (count > len)
        count = len;

    if (count == 0)
        return 0;

    len = tb->buf_buf + tb->buf_size - tb->buf_put;
    if (count > len) {
        memcpy(tb->buf_put, buf, len);
        memcpy(tb->buf_buf, buf + len, count - len);
        tb->buf_put = tb->buf_buf + count - len;
    }
    else {
        memcpy(tb->buf_put, buf, count);
        if (count < len)
            tb->buf_put += count;
        else /* count == len */
            tb->buf_put = tb->buf_buf;
    }

    return count;
}

/*
* tty_buf_get
*
* Get data from the circular buffer and copy to the given buffer.
* Restrict to the amount of data available.
*
* Return the number of bytes copied.
*/
static unsigned
tty_buf_get(struct tty_buf *tb, char *buf, unsigned count)
{
    unsigned len;

    len = tty_buf_data_avail(tb);
    if (count > len)
        count = len;

    if (count == 0)
        return 0;

    len = tb->buf_buf + tb->buf_size - tb->buf_get;
    if (count > len) {
        memcpy(buf, tb->buf_get, len);
        memcpy(buf + len, tb->buf_buf, count - len);
        tb->buf_get = tb->buf_buf + count - len;
    }
    else {
        memcpy(buf, tb->buf_get, count);
        if (count < len)
            tb->buf_get += count;
        else /* count == len */
            tb->buf_get = tb->buf_buf;
    }

    return count;
}

/*-------------------------------------------------------------------------*/

#if 1 // // add for usb host
int tty_port_install(struct tty_port *port, struct tty_driver *driver,
        struct tty_struct *tty)
{
    tty->port = port;
    driver->ttys[tty->index] = tty;
    return 0;
}

int tty_port_open(struct tty_port *port, struct tty_struct *tty)
{
    unsigned long flags;

    spin_lock_irqsave(&port->lock, flags);
    port->tty = tty;
    spin_unlock_irqrestore(&port->lock, flags);

    //mutex_lock(&port->mutex);

    if (!port->initialized) {
        if (port->ops->activate) {
            int retval = port->ops->activate(port, tty);
            if (retval) {
                //mutex_unlock(&port->mutex);
                return retval;
            }
        }
        port->initialized = 1;
    }
    //mutex_unlock(&port->mutex);

    //tty_port_block_til_ready()
    if (port->ops->dtr_rts)
        port->ops->dtr_rts(port, 1);

    if (port->ops->carrier_raised)
        port->ops->carrier_raised(port);

    port->active = 1;
    
    return 0;
}

/**
 * tty_port_tty_hangup - helper to hang up a tty
 *
 * @port: tty port
 * @check_clocal: hang only ttys with CLOCAL unset?  (true)
 */
void tty_port_tty_hangup(struct tty_port *port, bool check_clocal)
{
    struct tty_struct *tty = tty_port_tty_get(port);
    __func_enter__

    printf("tty_port_tty_hangup() \n");
    if (tty && (!check_clocal /*|| !C_CLOCAL(tty)*/))
        tty_hangup(tty);
    tty_kref_put(tty);
    __func_leave__
}

/**
 * tty_port_tty_wakeup - helper to wake up a tty
 *
 * @port: tty port
 */
void tty_port_tty_wakeup(struct tty_port *port)
{
    struct tty_struct *tty = tty_port_tty_get(port);

    if (tty) {
        tty_wakeup(tty);
        tty_kref_put(tty);
    }
}

/** tty_chars_in_buffer	-	characters pending */
static int tty_chars_in_buffer(struct tty_struct *tty)
{
    if (tty->ops->chars_in_buffer)
        return tty->ops->chars_in_buffer(tty);
    else
        return 0;
}

/**	tty_wait_until_sent	-	wait for I/O to finish */
static void tty_wait_until_sent(struct tty_struct *tty, long timeout)
{
    int ret;

    if (!timeout)
        timeout = 10*1000*1000;

    printf("wait until sent, timeout=%ld\n", timeout);

    ret = wait_event_interruptible_timeout(tty->write_wait,
        !tty_chars_in_buffer(tty), timeout);
    if (ret <= 0)  // timeout
        return;

    if (ret == timeout)
        timeout = 0;

    if (tty->ops->wait_until_sent)
        tty->ops->wait_until_sent(tty, timeout);
}

static void tty_port_shutdown(struct tty_port *port, struct tty_struct *tty)
{
    //mutex_lock(&port->mutex);
    __func_enter__

    if (port->initialized) {
        port->initialized = 0;
        /*
         * Drop DTR/RTS if HUPCL is set. This causes any attached
         * modem to hang up the line.
         */
        if (port->ops->dtr_rts)
            port->ops->dtr_rts(port, 0);

        if (port->ops->shutdown)
            port->ops->shutdown(port);
    }
    __func_leave__

    //mutex_unlock(&port->mutex);
}

void tty_port_hangup(struct tty_port *port)
{
    struct tty_struct *tty;
    unsigned long flags;
    __func_enter__

    spin_lock_irqsave(&port->lock, flags);
    port->count = 0;
    tty = port->tty;
    port->tty = NULL;
    spin_unlock_irqrestore(&port->lock, flags);
    port->active = 0;
    tty_port_shutdown(port, tty);
    __func_leave__
}

void tty_port_close(struct tty_port *port, struct tty_struct *tty)
{
    __func_enter__
    if (port->initialized)
        tty_wait_until_sent(tty, port->closing_wait);
    
    tty_port_shutdown(port, tty);
    port->active = 0;
    __func_leave__
}

static void release_tty(struct tty_struct *tty)
{
    __func_enter__
    // tty_driver_remove_tty
    tty->driver->ttys[tty->index] = NULL;

    // release_one_tty
    if (tty->ops->cleanup)
        tty->ops->cleanup(tty);

    __func_leave__
}

/* without read buffer for usb modem use */
void tty_port_init_h(struct tty_port *port)
{
    memset(port, 0, sizeof(*port));
    spin_lock_init(&port->lock);

    // add for usb host
    //mutex_init(&port->mutex);  
    port->close_delay = (50 * HZ) / 100;
    port->closing_wait = (3000 * HZ) / 100;

    port->modem = 1;
}

#endif

void tty_port_init(struct tty_port *port)
{
    memset(port, 0, sizeof(*port));
    if (tty_buf_alloc(&port->tty_read_buf, TTY_READ_BUF_SIZE))
        printf("tty buffer alloc fail!! %s:%d\n", __func__, __LINE__);
    spin_lock_init(&port->lock);

    // add for usb host
    //mutex_init(&port->mutex);  
    port->close_delay = (50 * HZ) / 100;
    port->closing_wait = (3000 * HZ) / 100;
}

void tty_port_destroy(struct tty_port *port)
{
    __func_enter__
    if (!port->modem)
        tty_buf_free(&port->tty_read_buf);

    __func_leave__
    // add for usb host
    //mutex_destroy(&port->mutex);  
}

/* in critical section */
int tty_insert_flip_string(struct tty_port *port,
    const unsigned char *chars, size_t size)
{
    if (port->modem) {
        struct tty_struct *tty = port->tty;
        if (tty && tty->rx_cb)
            tty->rx_cb(tty->rx_arg, chars, size);
    }
    else {
        int copied = 0;
        do {
            int goal = min_t(size_t, size - copied, TTY_READ_BUF_SIZE);
            int space = tty_buf_space_avail(&port->tty_read_buf);

            if (unlikely(space == 0)) {
                port->tty->throttled = 1;
                //ithPrintf("Rx throttled! \n");
                break;
            }
            space = tty_buf_put(&port->tty_read_buf, chars, goal);
            copied += space;
            chars += space;
        } while (unlikely(size > copied));
        return copied;
    }
}

void tty_flip_buffer_push(struct tty_port *port)
{
    if (!port->modem) {
        port->avail_data = tty_buf_data_avail(&port->tty_read_buf);

        if (port->avail_data)
            wake_up(&port->tty->read_wait);
    }
}

// add for usb host
void __tty_hangup(struct tty_struct *tty)
{
    __func_enter__
    
    if (tty->ops->hangup) 
        tty->ops->hangup(tty);

    __func_leave__
}

// for usb acm device
void tty_hangup(struct tty_struct *tty)
{
    struct tty_port *port = tty->driver_data;

    tty_buf_clear(&port->tty_read_buf);
    port->avail_data = -1;
    wake_up(&tty->read_wait);
}

/*-------------------------------------------------------------------------*/
#define check_status(x)  { if(status) printf(" %s() error code: %d \n", __func__ , status); }

int iteAcmTtyIsReadyEx(int index)
{
    struct tty_struct *tty = &g_tty[index];

    if (!tty->driver)
        return 0;

    return tty->ops->is_ready(tty);
}

int iteAcmTtyOpenEx(int index, void (*rx_cb)(void *arg, unsigned char *chars, size_t size), void *arg)
{
    int status = 0;
    struct tty_struct *tty = &g_tty[index];

    if (!tty->driver) {
        status = -1;
        goto end;
    }

    spin_lock_init(&tty->lock);

    //<< add for usb host
    tty->rx_cb = rx_cb;
    tty->rx_arg = arg;
    
    if (tty->ops->install) 
        tty->ops->install(tty->driver, tty);
    //>> add for usb host

    status = tty->ops->open(tty);

end:
    check_status(status);
    return status;
}

int iteAcmTtyCloseEx(int index)
{
    int status = 0;
    struct tty_struct *tty = &g_tty[index];

    if (!tty->driver) {
        status = -1;
        goto end;
    }

    if (tty->ops->close)
        tty->ops->close(tty);

    release_tty(tty);

end:
    check_status(status);
    return status;
}

// for usb acm device
int iteAcmTtyReadEx(int index, char *buf, int len)
{
    int count;
    struct tty_struct *tty = &g_tty[index];
    struct tty_port *port = tty->driver_data;
    
    if (!tty->driver)
        return 0;

    wait_event(tty->read_wait, (port->avail_data || (port->tty == NULL)));

    if (port->tty == NULL)
        return 0;

    if (port->avail_data == -1) /* hang up */
        return 0;

    spin_lock_irq(&tty->lock);
    count = tty_buf_data_avail(&port->tty_read_buf);
    if (count > 0) {
        if (count > len)
            count = len;

        count = tty_buf_get(&port->tty_read_buf, buf, count);
        if (tty->throttled) {
            tty->throttled = 0;
            tty->ops->unthrottle(tty);
        }
    }
    port->avail_data = tty_buf_data_avail(&port->tty_read_buf);
    spin_unlock_irq(&tty->lock);

    return count;
}

int iteAcmTtyWriteEx(int index, const unsigned char *buf, int len)
{
    int count = 0, ret;
    struct tty_struct *tty = &g_tty[index];
    unsigned int timeout = 10 * 1000; // ms

    if (!tty->driver)
        return -1;

    count = tty->ops->write(tty, buf, len);
    while (count < len) {
        ret = wait_event_interruptible_timeout(tty->write_wait,
            tty->ops->write_room(tty), timeout);
        if (ret <= 0) { // timeout
            printf("iteAcmTtyWriteEx(%d, %p, %d) timeout! \n", index, buf, len);
            goto end;
        }
        count += tty->ops->write(tty, buf, len - count);
    }

end:
    return count;
}

int iteAcmTtyGetWriteRoomEx(int index)
{
    int room = 0;
    struct tty_struct *tty = &g_tty[index];

    if (!tty->driver)
        return -1;

    room = tty->ops->write_room(tty);

    return room;
}

int iteAcmTtyPutCharEx(int index, unsigned char c)
{
    int status = 0;
    struct tty_struct *tty = &g_tty[index];

    if (!tty->driver)
        return -1;

    status = tty->ops->put_char(tty, c);

    return status;
}

void iteAcmTtyFlushCharsEx(int index)
{
    struct tty_struct *tty = &g_tty[index];

    if (!tty->driver)
        return;

    tty->ops->flush_chars(tty);

    return;
}
