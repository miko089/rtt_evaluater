# rtt_evaluater


RTT Evaluator is a client–server application written in C that measures network round-trip time (RTT) using the UDP protocol. The application accurately measures network latency and computes performance metrics based on a configurable series of UDP packets.

---

## Overview

The project is composed of two main components:

- **Server**  
  The server listens on a specified UDP port and immediately echoes back any received packets without modification. It supports graceful shutdown when a termination signal is received.

- **Client**  
  The client sends a configurable number of UDP packets (minimum 1000) to the server and measures the RTT for each packet. It handles packet loss and timeouts appropriately and calculates the following metrics:
  - Minimum RTT
  - Maximum RTT
  - Average RTT
  - RTT Standard Deviation

---

## Features

- **Language:** Implemented in C for performance and portability.
- **Protocol:** Uses UDP for low-latency communication.
- **Packet Configuration:** Supports packet sizes ranging from 16 to 8192 bytes.
- **I/O Operations:** Uses non-blocking I/O to maintain responsiveness.
- **Error Handling:** Provides proper error reporting and graceful handling of network failures.
- **Compilation:** Compiles cleanly with GCC/Clang using `-Wall -Wextra`.

---

## Build and Environment

A Makefile is provided to build both the server and client applications.

### Building

To build the project, run:

```bash
make
```
This command produces two executables:
- client_app – the client application.
- server_app – the server application.

## Cleaning
To remove build artifacts, run:

```bash
make clean
```

## Usage
### Running the Server
Start the server on a specified UDP port

```bash
./server_app <port> <packet_size>
```

The server will begin listening on the specified port and will echo back any received UDP packets.

### Running the Client
Run the client by providing the server IP address, UDP port, number of packets, and packet size:

```bash
./client_app <server_ip> <port> <num_packets> <packet_size>
```

Parameters:
- SERVER_IP: The IP address of the server
- SERVER_PORT: The UDP port on which the server is listening
- NUM_PACKETS: The number of UDP packets to send (minimum 1000)
- PACKET_SIZE: The size of each UDP packet in bytes (16 to 8192)

After the test completes, the client outputs metrics similar to:
```bash
Packets sent: 1000
 Packets received: 1000
RTT: avg = 0.054 ms, stddev = 0.033 ms, min = 0.022 ms, max = 0.421 ms
```

## Error Handling
- The application uses non-blocking I/O; if packets are lost or a timeout occurs, the client continues processing while reporting the error.
- All network failures are reported with descriptive error messages.

## Requirements
- A POSIX-compliant operating system (Linux, macOS, etc.)
- GCC or Clang compiler
- Make build system

## Acknowledgements
This project was developed as part of test task for project seminar in NUP CSAI


