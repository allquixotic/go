:: Copyright 2016 The Go Authors. All rights reserved.
:: Use of this source code is governed by a BSD-style
:: license that can be found in the LICENSE file.
@echo off

setlocal

:: GOPATH to cwd
set GOPATH=%cd%

:: Setup GO env
go env > env.bat
call env.bat
del env.bat

:: Setup cross compiler args
set CCARGS=-Ipkg\%GOOS%_%GOARCH% -lntdll -lws2_32
set BIN=.\testp.exe

set STATUS=0

:: Installing first will create the header files we want.
go install -buildmode=c-archive libgo
%CC% %GOGCCFLAGS% -o testp main_windows.c pkg\%GOOS%_%GOARCH%\libgo.a %CCARGS%

%BIN% arg1 arg2
if not errorlevel 0 (
    echo FAIL test1a
    set STATUS=1
)
del pkg\%GOOS%_%GOARCH%\libgo.a pkg\%GOOS%_%GOARCH%\libgo.h testp.exe

:: Test building libgo other than installing it.
:: Header files are now present.
go build -buildmode=c-archive src\libgo\libgo.go
%CC% %GOGCCFLAGS% -o testp main_windows.c libgo.a %CCARGS%

%BIN% arg1 arg2
if not errorlevel 0 (
    echo FAIL test1a
    set STATUS=1
)
del libgo.a libgo.h testp.exe

:: Test building as a package

go build -buildmode=c-archive libgo
%CC% %GOGCCFLAGS% -o testp main_windows.c libgo.a %CCARGS%

%BIN% arg1 arg2
if not errorlevel 0 (
    echo FAIL test1b
    set STATUS=1
)
del libgo.a libgo.h testp.exe
rmdir /s /q pkg > nul

exit %STATUS%
