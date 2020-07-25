@echo off

set CFG_PROJECT=%~n0

set ALT_CPU=1
call test_rsl_slave.cmd

@if not defined NO_PAUSE pause
