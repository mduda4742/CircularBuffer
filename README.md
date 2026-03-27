# IPC Circular Buffer System

A multi-process Linux system for real-time signal generation and analysis using POSIX Shared Memory and Semaphores.

## Project Overview
This system demonstrates a low-latency Producer-Consumer architecture. A virtual sensor (producer) generates sinusoidal data into a shared circular buffer, while independent monitor processes (consumers) perform real-time analysis (SMA and Frequency estimation).

## Key Features
* **Zero-copy Communication**: Data exchange via `shm_open` and `mmap`.
* **Synchronization**: POSIX semaphores manage concurrent access.
* **Lock Contention Management**: Consumers use local buffering to minimize critical section time.
* **Robust DSP**: Frequency estimation using Mean-Crossing detection to handle DC offsets.

## Build
```bash
mkdir build && cd build
cmake ..
make

Usage

Start vSensor first to initialize the shared memory segment. Run each program in a separate terminal window.
1. Signal Generator (vSensor)
./vSensor <f_signal> <f_sampling> <dc_offset> <buffer_size>

    f_signal: Target frequency of the sine wave (Hz).

    f_sampling: Sampling rate (samples per second).

    dc_offset: DC offset of sine wave

    buffer_size: Capacity of the circular buffer in the shared memory.

2. Moving Average Monitor (monitor_avg)
./monitor_avg <refresh_rate> <num_samples>

    refresh_rate: Output update frequency (Hz).

    num_samples: Size of the sliding window for SMA calculation.

3. Frequency Monitor (monitor_f)

./monitor_f <refresh_rate> <num_samples>

    refresh_rate: Output update frequency (Hz).

    num_samples: Size of the sliding window for frequency estimation.

Cleanup

All modules handle SIGINT (Ctrl+C). Exiting vSensor automatically unlinks the shared memory segment from the Linux system.

