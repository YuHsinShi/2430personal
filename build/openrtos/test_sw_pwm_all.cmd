@echo off

set CFG_PROJECT=%~n0

set ALT_CPU=1
call test_sw_pwm.cmd

@if not defined NO_PAUSE pause
