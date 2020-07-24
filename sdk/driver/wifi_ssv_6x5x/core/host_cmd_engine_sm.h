/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _HOST_CMD_ENGINE_SM_H_
#define _HOST_CMD_ENGINE_SM_H_

/**
 * SM_STATE - Declaration of a state machine function
 * @machine: State machine name
 * @state: State machine state
 *
 * This macro is used to declare a state machine function. It is used in place
 * of a C function definition to declare functions to be run when the state is
 * entered by calling SM_ENTER or SM_ENTER_GLOBAL.
 */
#define SM_STATE(machine, state) \
void sm_ ## machine ## _ ## state ## _Enter(STATE_MACHINE_DATA *sm, void *data)


/**
 * SM_ENTRY - State machine function entry point
 * @machine: State machine name
 * @state: State machine state
 *
 * This macro is used inside each state machine function declared with
 * SM_STATE. SM_ENTRY should be in the beginning of the function body, but
 * after declaration of possible local variables. This macro prints debug
 * information about state transition and update the state machine state.
 */
#define SM_ENTRY(machine, state) \
if (sm->machine ## _state != machine ## _ ## state) { \
	LOG_TRACE(SSV_HCMD, ": " #machine " entering state " #state"\n"); \
} \
else	\
	return;	\
sm->machine ## _state = machine ## _ ## state;



/**
 * SM_ENTER - Enter a new state machine state
 * @machine: State machine name
 * @state: State machine state
 *
 * This macro expands to a function call to a state machine function defined
 * with SM_STATE macro. SM_ENTER is used in a state machine step function to
 * move the state machine to a new state.
 */
#define SM_ENTER(machine, state, data) \
sm_ ## machine ## _ ## state ## _Enter(gHCmdEngInfo, data)





/**
 * SM_STEP - Declaration of a state machine step function
 * @machine: State machine name
 *
 * This macro is used to declare a state machine step function. It is used in
 * place of a C function definition to declare a function that is used to move
 * state machine to a new state based on state variables. This function uses
 * SM_ENTER and SM_ENTER_GLOBAL macros to enter new state.
 */
#define SM_STEP(machine) \
static void sm_ ## machine ## _Step(STATE_MACHINE_DATA *sm)

/**
 * SM_STEP_RUN - Call the state machine step function
 * @machine: State machine name
 *
 * This macro expands to a function call to a state machine step function
 * defined with SM_STEP macro.
 */
#define SM_STEP_RUN(machine) sm_ ## machine ## _Step(gHCmdEngInfo)

#endif /* _HOST_CMD_ENGINE_SM_H_ */

#define SM_GET_STATE(machine) gHCmdEngInfo->machine ## _state
