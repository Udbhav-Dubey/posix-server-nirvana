# posix-server-nirvana

A minimal event driven HTTP server written in C++ using POSIX sockets.

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

This repository represents **version 1.5** of the server, a working baseline that will be improved and refactored in future iterations.

---

## What the server currently does

- Listens on a TCP socket using POSIX APIs
- Handles basic HTTP/1.1 requests
- Discovers MP3 files from the working directory at startup
- Streams MP3 files over HTTP
- Exposes a JSON API listing available songs
- Serves lyrics files when present
- Handles multiple clients with help of poll system call

---

## Implemented Components

### Networking

- Manual socket setup using `socket`, `bind`, `listen`, `accept`, `poll`
- `SO_REUSEADDR` for reliable restarts
- Explicit handling of `SIGPIPE`
- `listen()` backlog set to `SOMAXCONN` (was hardcoded to 10, see Benchmarking)

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

### Event-Driven Nature

- I have shifted the server from thread per client model to event driven server using poll
- Now it reacts when there is some event and client_sock is made non_blocking so multiple clients can connect

---

## Benchmarking

Full numbers, setup and reproduction steps are in [BENCHMARKS.md](BENCHMARKS.md). Summary:

**Setup:** Release build, Ryzen 5 5500U, ApacheBench (`ab -n 50000`) against `/` on loopback, document length 11,307 bytes, one small static file. `ab` shares the CPU with the server.

### Against nginx (1 worker, same file)

| Concurrency | nirvana (req/s) | nginx (req/s) |
|---|---|---|
| c=10 | 21,941 | 20,835 |
| c=20 | 21,189 | 21,606 |

About the same as nginx on a small static file, with zero failed requests and p99 of 1 ms. Single runs on loopback have a few percent of noise.

### The `listen()` backlog bug (found by benchmarking, now fixed)

At c=100 the server used to collapse. The cause was `listen(fd, 10)`: the accept queue was only 10 deep, so the kernel dropped connections and clients retried after 1 s, 3 s and so on. Confirmed with the kernel counters `TcpExtListenOverflows` / `TcpExtListenDrops`, which stopped climbing after the fix.

| `ab -n 50000 -c 100` | Backlog 10 | Backlog `SOMAXCONN` |
|---|---|---|
| Requests/sec | 6,548 | 21,665 |
| Worst request | 7,631 ms | 7 ms |
| Worst connect time | 1,057 ms | 3 ms |

### poll() vs the old thread-per-connection version

| | poll | threaded |
|---|---|---|
| Threads with 500 idle connections | 1 | 501 |
| Reserved memory (VSZ) with 500 idle connections | ~7.5 MB | ~3.9 GB |
| c=100 requests/sec | 21,665 | 17,810 |

### Observations

- Throughput stays flat (~21-22k req/s) from c=10 to c=100, with zero failed requests.
- The backlog bug barely moved the median. It showed up as a 7.6 s worst case, so always look at max and percentiles, not just the mean.
- These numbers measure the request parse/dispatch/write path on loopback, not real network conditions.

---

## What this project is *not*

This project is intentionally **not**:

- a production-ready HTTP server
- fully HTTP-compliant
- asynchronous or event-driven with epoll (yet)
- built on top of any web framework

The goal is learning and understanding, not abstraction or performance tuning.

---

## Building & Running

```bash
# execute this in directory parent Directory of project
cmake -S . -B build
cmake --build build
./build/nirvana # to start it
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

If anybody wants to upgrade the html they can so just make sure the JavaScript code in html works same

---

## Planned Improvements

Future iterations of this project will focus on:

- Proper HTTP request parsing
- Improved routing abstractions
- Better protocol correctness
- epoll instead of poll
- Lyrics sync
- thread pool for songs
- buffer management

This repository serves as the baseline before those changes.

---

## Why this exists

This project exists to understand how networking and servers work **under the hood** by building them manually and confronting real limitations directly.

---

This project originally started as a personal experiment to serve a few Nirvana tracks stored locally. The name is a small homage to that origin.

![nirvana logo](logo.png)
