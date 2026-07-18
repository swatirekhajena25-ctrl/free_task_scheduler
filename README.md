# FreeRTOS Task Scheduler (POSIX Simulation)

A multi-task embedded systems project demonstrating priority-based
preemptive scheduling, inter-task communication, and safe resource
sharing using FreeRTOS — running on the official POSIX simulator
(no physical hardware required).

## Overview

This project implements three concurrent tasks under FreeRTOS:

- **Producer** — generates an incrementing value every 1 second and
  sends it to a shared queue.
- **Consumer** — blocks on the queue, receives each value as it
  arrives, and prints it.
- **Logger** — runs at a higher priority than Producer/Consumer,
  printing a heartbeat message every 2 seconds. Its higher priority
  demonstrates FreeRTOS's preemptive scheduling: whenever the Logger
  task becomes ready, it interrupts whichever lower-priority task is
  running.

All console output is protected by a mutex (`xSemaphoreCreateMutex`)
so concurrent prints from different tasks never interleave mid-line.

## Design

| Component | Purpose |
|---|---|
| `xQueue` | FreeRTOS queue used to pass integer values from Producer to Consumer |
| `xPrintMutex` | Binary mutex ensuring only one task writes to stdout at a time |
| Task priorities | Producer & Consumer run at `tskIDLE_PRIORITY + 1`; Logger runs at `tskIDLE_PRIORITY + 2`, so it always preempts the other two when ready |

## Build & Run

Requires `gcc`, `make`, and `pthread` (standard on any Linux
environment, including GitHub Codespaces).

```bash
make clean
make NO_TRACING=1
./build/posix_demo
```

Press `Ctrl+C` to stop — tasks run in an infinite loop by design,
same as a real embedded scheduler would.

## What I learned / debugged

Building this from a stripped-down FreeRTOS demo (rather than the
stock example) surfaced several real FreeRTOS internals:

- **`vAssertCalled`** — required by `configASSERT()` checks used
  throughout the kernel source for internal sanity checks.
- **`vApplicationGetIdleTaskMemory` / `vApplicationGetTimerTaskMemory`**
  — required because this build uses static allocation
  (`configSUPPORT_STATIC_ALLOCATION`), so the application (not the
  kernel) must supply the memory for the Idle and Timer service tasks.
- **`vApplicationDaemonTaskStartupHook`** — called once when the
  Timer service task starts.

Resolving these linker errors one at a time gave a clearer picture of
how FreeRTOS's configuration (`FreeRTOSConfig.h`) drives what the
application is responsible for providing versus what the kernel
provides automatically.

## Notes

This runs on FreeRTOS's official POSIX simulator port, which
schedules tasks using Linux threads/signals to emulate a real-time
scheduler — useful for prototyping and learning scheduling behavior
without needing physical hardware (e.g. ESP32, STM32).
