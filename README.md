# posix-server-nirvana

A minimal HTTP server written in C++ using POSIX sockets.

This project began as a small exercise while reading *Beej's Guide to Network Programming* and gradually evolved into a learning-focused server capable of serving local MP3 files, exposing a small JSON API, and handling multiple clients.

---

## Origin & Motivation

This project started as a simple goal:
- create a TCP server  
- accept connections  
- respond to basic HTTP requests  

While experimenting, I wanted the server to serve a few Nirvana tracks stored locally on my machine.  

That requirement immediately pushed the project beyond a trivial example and introduced real problems such as:
- routing requests to different endpoints  
- serving binary data correctly  
- organizing server-side logic  
- handling multiple simultaneous clients  

Hitting those walls is what drove the project forward.

This repository represents **version 1** of the server — a working baseline that will be improved and refactored in future iterations.

---

## What the server currently does

- Listens on a TCP socket using POSIX APIs  
- Handles basic HTTP/1.1 requests  
- Discovers MP3 files from the working directory at startup  
- Streams MP3 files over HTTP  
- Exposes a JSON API listing available songs  
- Serves lyrics files when present  
- Handles multiple clients using a thread-per-connection model  
- Measures per-request handling time  

---

## Implemented Components

### Networking
- Manual socket setup using `socket`, `bind`, `listen`, `accept`
- `SO_REUSEADDR` for reliable restarts
- Explicit handling of `SIGPIPE`

### HTTP Handling
- Manual request parsing (path extraction)
- Explicit response construction with headers and body
- Support for binary and text responses

### Routing
- Path-based routing implemented in code
- Endpoints such as:
  - `/api/songs`
  - `/play/<id>`
  - `/download/<id>`
  - `/lyrics/<id>`

### File Handling
- Directory scanning using `std::filesystem`
- Binary MP3 streaming
- Optional lyrics file support

### Concurrency
- Thread-per-client model using `std::thread`
- Basic synchronization for logging output

---

## What this project is *not*

This project is intentionally **not**:
- a production-ready HTTP server  
- fully HTTP-compliant  
- asynchronous or event-driven  
- built on top of any web framework  

The goal is learning and understanding, not abstraction or performance tuning.

---

## Building & Running
```bash
g++ -std=c++17 server.cpp -pthread
./a.out
```

The server listens on:
```
http://localhost:28333
```

---

## About the HTML

The HTML file included in this repository was **not written by me** and was generated with the help of an LLM.

It exists only as a minimal interface to interact with the server during testing.  

The focus of this project is networking and systems programming, not frontend development.

---

## Planned Improvements

Future iterations of this project will focus on:
- Proper HTTP request parsing  
- Cleaner separation of concerns (headers and source files)  
- Improved routing abstractions  
- More robust error handling  
- Better protocol correctness  

This repository serves as the baseline before those changes.

---

## Why this exists

This project exists to understand how networking and servers work **under the hood** by building them manually and confronting real limitations directly.

---

<sub>
This project originally started as a personal experiment to serve a few Nirvana tracks stored locally.  
The name is a small homage to that origin.
</sub>


![logo](logo.png)
