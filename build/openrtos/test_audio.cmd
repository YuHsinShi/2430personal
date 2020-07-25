@echo off

set ARMLITECODEC=1
set CFG_PROJECT=%~n0
set PRESETTING=
rem set AUTOBUILD=1

call build.cmd

@if not defined NO_PAUSE pause
