// Copyright 2015 The Go Authors.  All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

//Requires Windows Vista, Windows Server 2008 or later
#define _WIN32_WINNT 0x0600
#define WIN64_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>

#include <stdio.h>
#include <stdlib.h>

static INIT_ONCE runtime_init_once = INIT_ONCE_STATIC_INIT;
static CRITICAL_SECTION runtime_init_cs;

static HANDLE runtime_init_wait;
static int runtime_init_done;

// Pre-initialize the runtime synchronization objects
BOOL CALLBACK 
_cgo_preinit_runtime(PINIT_ONCE init_once_fn, PVOID param, PVOID *context) {
   runtime_init_wait = CreateEvent(NULL, TRUE, FALSE, NULL);
   if (runtime_init_wait == NULL) {
		fprintf(stderr, "runtime: failed to create runtime initialization wait event.\n");
		abort();
   }

   InitializeCriticalSection(&runtime_init_cs);
   return TRUE;
}

void
_cgo_maybe_run_preinit() {
   if (!InitOnceExecuteOnce(&runtime_init_once, _cgo_preinit_runtime, NULL, NULL)) {
		fprintf(stderr, "runtime: failed to pre-initialize the runtime.\n");
		abort();
   }
}

void
x_cgo_sys_thread_create(void (*func)(void*), void* arg) {
	uintptr_t thandle;

	thandle = _beginthread(func, 0, arg);
	if(thandle == -1) {
		fprintf(stderr, "runtime: failed to create new OS thread (%d)\n", errno);
		abort();
	}
}

int
_cgo_is_runtime_initialized() {
   EnterCriticalSection(&runtime_init_cs);
   int status = runtime_init_done;
   LeaveCriticalSection(&runtime_init_cs);
   return status;
}

void
_cgo_wait_runtime_init_done() {
   _cgo_maybe_run_preinit();
	while (!_cgo_is_runtime_initialized()) {
      WaitForSingleObject(runtime_init_wait, INFINITE);
	}
}

void
x_cgo_notify_runtime_init_done(void* dummy) {
   _cgo_maybe_run_preinit();
   
   EnterCriticalSection(&runtime_init_cs);
	runtime_init_done = 1;
   LeaveCriticalSection(&runtime_init_cs);
   
   if (!SetEvent(runtime_init_wait)) {
		fprintf(stderr, "runtime: failed to signal runtime initialization complete.\n");
		abort();
	}
}

