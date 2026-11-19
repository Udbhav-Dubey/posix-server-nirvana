# Benchmarks

Benchmarks for posix-server-nirvana: how it compares to nginx, the `listen()` backlog bug found by benchmarking, and poll() vs the old thread-per-connection version.

## Setup

- CPU: Ryzen 5 5500U, Linux (Manjaro), release build
- Tool: ApacheBench 2.3 (`ab`), `-n 50000` unless noted
- Target: `/` on the server, one static HTML file of **11,307 bytes**
- Everything runs over loopback (`127.0.0.1`), and `ab` shares the CPU with the server
- Server stdout redirected to `/dev/null` so terminal printing is not measured
- Zero failed requests in every run below

These numbers measure the server's request parse/dispatch/write path on one machine. They are not real network conditions.

## 1. The `listen()` backlog bug

The server used `listen(server_socket, 10)`. At 100 concurrent clients the accept queue overflowed and the kernel dropped connections. Clients retried after ~1 s, then ~3 s, which produced a huge tail latency while the median stayed at 1 ms.

`ab -n 50000 -c 100`:

| | Backlog 10 | Backlog fixed |
|---|---|---|
| Requests/sec | 6,548 | 21,665 |
| Worst request | 7,631 ms | 7 ms |
| Worst connect time | 1,057 ms | 3 ms |
| `TcpExtListenOverflows` / `TcpExtListenDrops` | 464 | 464 (unchanged) |

- The counters (`nstat -az TcpExtListenOverflows TcpExtListenDrops`) are cumulative since boot. The evidence is that they stopped climbing after the fix, not that 464 drops happened in one run.
- The benchmark run used a backlog of 1024. The committed code uses `SOMAXCONN` (4096 on this machine, from `/proc/sys/net/core/somaxconn`).
- A 1,064 ms connect outlier also showed up once in a 50k-request run at c=20 with the old backlog. After the fix, the worst request at c=20 was 3 ms.
- The old thread-per-connection version had the same `listen(..., 10)` line.

## 2. Against nginx

nginx 1.30.5, `worker_processes 1`, `access_log off`, `sendfile on`, serving the same 11,307-byte file. The nirvana numbers here are from before the backlog fix, at concurrency levels where it did not matter much.

| Concurrency | nirvana (req/s) | nginx (req/s) |
|---|---|---|
| c=10 | 21,941 | 20,835 |
| c=20 | 21,189 | 21,606 |

Within a few percent of nginx on a small static file. Single runs on loopback have a few percent of noise, so this is "about the same", not "faster than nginx". nginx also sends ~149 more bytes per response (extra headers).

Logging cost: an earlier run with per-request printing enabled did 19,276 req/s at c=10 vs 21,941 silent (about 14% lower). That comparison used 10k vs 50k requests, so treat it as rough.

## 3. poll() vs thread-per-connection

The old version spawns a detached `std::thread` per connection (one `recv`, one `send`, close). The current version uses a single thread with `poll()`. Both use the fixed backlog here.

Memory with 500 idle connections (`ps -o rss=,vsz=,nlwp=`):

| | Threads | RSS | VSZ (reserved address space) |
|---|---|---|---|
| poll, idle | 1 | 4,380 KB | 7,468 KB |
| poll, 500 connections | 1 | 4,420 KB | 7,468 KB |
| threaded, idle | 1 | 4,380 KB | 7,464 KB |
| threaded, 500 connections | 501 | 10,756 KB | 4,105,596 KB (~3.9 GB) |

Threaded costs ~13 KB of real RAM and ~8.2 MB of reserved address space per connection (default 8 MB thread stack). Linux commits stack pages lazily, so RSS looks cheap. The real limits at 10,000+ connections are thread count, ulimits and overcommit.

Throughput and latency:

| | poll | threaded |
|---|---|---|
| c=20 requests/sec | 21,697 | 19,913 |
| c=20 worst request | 3 ms | 6 ms |
| c=100 requests/sec | 21,665 | 17,810 |
| c=100 p99 | 6 ms | 8 ms |
| c=100 worst request | 7 ms | 13 ms |

poll is about 9% faster at c=20 and about 22% faster at c=100, and stays flat as concurrency rises. Poll and threaded numbers come from separate runs, so differences of a few percent are noise. The c=100 gap is large enough to hold.

The threaded version also logs every request under a mutex, so part of its gap is logging cost, not only threading.

## Reproduce

```bash
# build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# run silent
./build/nirvana > /dev/null 2>&1 &

# benchmark
ab -n 50000 -c 10  http://127.0.0.1:28333/
ab -n 50000 -c 20  http://127.0.0.1:28333/
ab -n 50000 -c 100 http://127.0.0.1:28333/

# did the kernel drop connections?
nstat -az TcpExtListenOverflows TcpExtListenDrops
```

Run one server at a time (both versions use port 28333).

## Caveats

- Loopback on one machine, `ab` sharing the CPU, one small static file.
- Results from separate runs differ by a few percent.
- `nstat` counters are cumulative since boot.
- epoll is not implemented yet, so there are no epoll numbers.
