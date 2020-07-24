#ifndef ITH_PWM_H
#define ITH_PWM_H

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_pwm PWM
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * PWM definition.
 */
typedef enum
{
    ITH_PWM1 = ITH_TIMER1,  ///< PWM #1
    ITH_PWM2 = ITH_TIMER2,  ///< PWM #2
    ITH_PWM3 = ITH_TIMER3,  ///< PWM #3
    ITH_PWM4 = ITH_TIMER4,  ///< PWM #4
    ITH_PWM5 = ITH_TIMER5,  ///< PWM #5
    ITH_PWM6 = ITH_TIMER6,  ///< PWM #6
    ITH_PWM7 = ITH_TIMER7,  ///< PWM #7
    ITH_PWM8 = ITH_TIMER8   ///< PWM #8
} ITHPwm;

/**
 * Initialize PWM module.
 *
 * @param pwm The PWM.
 * @param freq The frequence.
 * @param duty The duty cycle.
 */
void ithPwmInit(ITHPwm pwm, unsigned int freq, unsigned int duty);

/**
 * Resets PWM module.
 *
 * @param pwm The PWM.
 * @param pin the GPIO pin.
 * @param gpio_mode the GPIO mode.
 */
void ithPwmReset(ITHPwm pwm, unsigned int pin, unsigned int gpio_mode);

/**
 * Sets duty cycle of PWM.
 *
 * @param pwm The PWM.
 * @param duty The duty cycle to set.
 */
void ithPwmSetDutyCycle(ITHPwm pwm, unsigned int duty);

/**
 * Enables PWM.
 *
 * @param pwm The PWM.
 * @param pin the GPIO pin.
 * @param gpio_mode the GPIO mode.
 */
void ithPwmEnable(ITHPwm pwm, unsigned int pin, unsigned int gpio_mode);

/**
 * Disables PWM.
 *
 * @param pwm The PWM.
 * @param pin the GPIO pin.
 */
void ithPwmDisable(ITHPwm pwm, unsigned int pin);

#ifdef __cplusplus
}
#endif

#endif // ITH_PWM_H
/** @} */ // end of ith_pwm
/** @} */ // end of ith