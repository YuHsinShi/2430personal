@echo off

set CFG_PROJECT=%~n0

set ALT_CPU=1
call test_pattern_gen.cmd

@if not defined NO_PAUSE pause
