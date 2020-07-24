#ifndef HR_TIMER_H
#define HR_TIMER_H

#include <linux/list.h>

#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC			1
#endif

enum hrtimer_mode {
    HRTIMER_MODE_ABS = 0x00,  /* Time value is absolute */
    HRTIMER_MODE_REL = 0x01,  /* Time value is relative to now */
    HRTIMER_MODE_PINNED = 0x02,
    HRTIMER_MODE_SOFT = 0x04,

    HRTIMER_MODE_ABS_PINNED = HRTIMER_MODE_ABS | HRTIMER_MODE_PINNED,
    HRTIMER_MODE_REL_PINNED = HRTIMER_MODE_REL | HRTIMER_MODE_PINNED,

    HRTIMER_MODE_ABS_SOFT = HRTIMER_MODE_ABS | HRTIMER_MODE_SOFT,
    HRTIMER_MODE_REL_SOFT = HRTIMER_MODE_REL | HRTIMER_MODE_SOFT,

    HRTIMER_MODE_ABS_PINNED_SOFT = HRTIMER_MODE_ABS_PINNED | HRTIMER_MODE_SOFT,
    HRTIMER_MODE_REL_PINNED_SOFT = HRTIMER_MODE_REL_PINNED | HRTIMER_MODE_SOFT,
};

/*
* Return values for the callback function
*/
enum hrtimer_restart {
    HRTIMER_NORESTART,	/* Timer is not restarted */
    HRTIMER_RESTART,	/* Timer must be restarted */
};

/*
 * Values to track state of the timer
 */
#define HRTIMER_STATE_INACTIVE	0x00
#define HRTIMER_STATE_ENQUEUED	0x01

struct hrtimer {
    struct list_head    list;
#define HR_TIMER_MAX_TIMEOUT_VAL        0x7FFFFFFF
    uint32_t    expires;
    uint32_t    start_time;
    uint32_t    interval;
    enum hrtimer_restart(*function)(struct hrtimer *);
    uint8_t     state;
    uint8_t     mode;    /* absolute or relative to now */
    uint8_t     reserved[2];
};



void ite_hrtimer_init(struct hrtimer *timer, int clock_id, enum hrtimer_mode mode);
void ite_hrtimer_start(struct hrtimer *timer, uint32_t tim, const enum hrtimer_mode mode);
int ite_hrtimer_try_to_cancel(struct hrtimer *timer);
bool ite_hrtimer_active(const struct hrtimer *timer);
int ite_hrtimer_cancel(struct hrtimer *timer);

#define hrtimer_init            ite_hrtimer_init
#define hrtimer_start           ite_hrtimer_start
#define hrtimer_try_to_cancel   ite_hrtimer_try_to_cancel
#define hrtimer_active          ite_hrtimer_active
#define hrtimer_cancel          ite_hrtimer_cancel



#endif // HR_TIMER_H
