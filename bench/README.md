# Benchmarking `httpkit` ⚡️

In order to ensure that `httpkit` stays fast, here you'll find a number of small
servers written in other languages.

They should all do the same thing:

1. Log down the time, method and path
2. Reply with the path

They will all be called with the same command:

```sh
wrk2 \
	--threads=12 \
	--connections=400 \
	--duration=30s \
	--rate 30K
	https://localhost:8080/bench-it-chewie!
```

### Ruby/Rack

```sh
ostera/httpkit λ wrk2 --threads=12 --connections=400 --duration=30s --rate 30K http://localhost:8080/bench-it-chewie!
Running 30s test @ http://localhost:8080/bench-it-chewie!
  12 threads and 400 connections
  Thread calibration: mean lat.: 4611.970ms, rate sampling interval: 16515ms
  Thread calibration: mean lat.: 4602.015ms, rate sampling interval: 16154ms
  Thread calibration: mean lat.: 4654.559ms, rate sampling interval: 16613ms
  Thread calibration: mean lat.: 4516.088ms, rate sampling interval: 16179ms
  Thread calibration: mean lat.: 4601.379ms, rate sampling interval: 16269ms
  Thread calibration: mean lat.: 4900.919ms, rate sampling interval: 16449ms
  Thread calibration: mean lat.: 4552.994ms, rate sampling interval: 16523ms
  Thread calibration: mean lat.: 9223372036854776.000ms, rate sampling interval: 10ms
  Thread calibration: mean lat.: 9223372036854776.000ms, rate sampling interval: 10ms
  Thread calibration: mean lat.: 9223372036854776.000ms, rate sampling interval: 10ms
  Thread calibration: mean lat.: 9223372036854776.000ms, rate sampling interval: 10ms
  Thread calibration: mean lat.: 4669.880ms, rate sampling interval: 16556ms
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    17.89s     5.12s   27.25s    57.47%
    Req/Sec     0.11      3.31   108.00     99.89%
  24349 requests in 30.18s, 4.41MB read
  Socket errors: connect 0, read 0, write 0, timeout 4276
Requests/sec:    806.78
Transfer/sec:    149.70KB
```

### Node/Http

```sh
ostera/httpkit λ wrk2 --threads=12 --connections=400 --duration=30s --rate 30K http://localhost:8080/bench-it-chewie!
Running 30s test @ http://localhost:8080/bench-it-chewie!
  12 threads and 400 connections
  Thread calibration: mean lat.: 3976.372ms, rate sampling interval: 13983ms
  Thread calibration: mean lat.: 4418.043ms, rate sampling interval: 15417ms
  Thread calibration: mean lat.: 4419.606ms, rate sampling interval: 15417ms
  Thread calibration: mean lat.: 4413.743ms, rate sampling interval: 15409ms
  Thread calibration: mean lat.: 4421.132ms, rate sampling interval: 15417ms
  Thread calibration: mean lat.: 4418.656ms, rate sampling interval: 15417ms
  Thread calibration: mean lat.: 4422.277ms, rate sampling interval: 15417ms
  Thread calibration: mean lat.: 4420.156ms, rate sampling interval: 15409ms
  Thread calibration: mean lat.: 4420.646ms, rate sampling interval: 15409ms
  Thread calibration: mean lat.: 3972.669ms, rate sampling interval: 13885ms
  Thread calibration: mean lat.: 4421.550ms, rate sampling interval: 15417ms
  Thread calibration: mean lat.: 3966.959ms, rate sampling interval: 13877ms
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    16.14s     4.73s   24.72s    56.45%
    Req/Sec   456.33      0.75   458.00    100.00%
  155767 requests in 30.07s, 17.38MB read
  Socket errors: connect 0, read 1, write 0, timeout 0
Requests/sec:   5179.50
Transfer/sec:    591.80KB
```

### Golang/Http

The golang standard library `http` module seems flexible enough for us to build
an incredibly fast echo server! Closer to 3 times faster than node's, and around
2000 requests per second more than `httpkit`.

If the superior type-safety offered by `httpkit` is not what you're looking for,
have a look at this:

```sh
ostera/httpkit λ wrk2 --threads=12 --connections=400 --duration=30s --rate 30K http://localhost:8080/bench-it-chewie!
Running 30s test @ http://localhost:8080/bench-it-chewie!
  12 threads and 400 connections
  Thread calibration: mean lat.: 2694.872ms, rate sampling interval: 9592ms
  Thread calibration: mean lat.: 2674.729ms, rate sampling interval: 9551ms
  Thread calibration: mean lat.: 2644.030ms, rate sampling interval: 9437ms
  Thread calibration: mean lat.: 2680.816ms, rate sampling interval: 9568ms
  Thread calibration: mean lat.: 2667.644ms, rate sampling interval: 9502ms
  Thread calibration: mean lat.: 2691.502ms, rate sampling interval: 9560ms
  Thread calibration: mean lat.: 2490.139ms, rate sampling interval: 8781ms
  Thread calibration: mean lat.: 2652.057ms, rate sampling interval: 9461ms
  Thread calibration: mean lat.: 2665.217ms, rate sampling interval: 9519ms
  Thread calibration: mean lat.: 2687.348ms, rate sampling interval: 9584ms
  Thread calibration: mean lat.: 2697.834ms, rate sampling interval: 9560ms
  Thread calibration: mean lat.: 2684.353ms, rate sampling interval: 9519ms
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    11.02s     3.20s   17.10s    58.01%
    Req/Sec     1.09k    23.03     1.12k    62.50%
  399390 requests in 30.00s, 28.57MB read
Requests/sec:  13314.83
Transfer/sec:      0.95MB
```

### Python/BaseHTTPServer

```sh
ostera/httpkit λ wrk2 --threads=12 --connections=400 --duration=30s --rate 30K http://localhost:8080/bench-it-chewie!
Running 30s test @ http://localhost:8080/bench-it-chewie!
  12 threads and 400 connections
  Thread calibration: mean lat.: 4190.046ms, rate sampling interval: 14639ms
  Thread calibration: mean lat.: 4269.527ms, rate sampling interval: 14483ms
  Thread calibration: mean lat.: 4213.688ms, rate sampling interval: 15704ms
  Thread calibration: mean lat.: 3632.081ms, rate sampling interval: 14434ms
  Thread calibration: mean lat.: 3932.717ms, rate sampling interval: 13344ms
  Thread calibration: mean lat.: 4980.591ms, rate sampling interval: 15450ms
  Thread calibration: mean lat.: 3027.887ms, rate sampling interval: 12361ms
  Thread calibration: mean lat.: 4010.051ms, rate sampling interval: 12828ms
  Thread calibration: mean lat.: 4807.472ms, rate sampling interval: 18104ms
  Thread calibration: mean lat.: 4543.476ms, rate sampling interval: 14524ms
  Thread calibration: mean lat.: 4214.574ms, rate sampling interval: 14008ms
  Thread calibration: mean lat.: 4466.553ms, rate sampling interval: 16506ms
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    10.99s     2.92s   20.97s    63.93%
    Req/Sec     6.50      1.89    10.00     91.67%
  15704 requests in 30.20s, 1.93MB read
  Socket errors: connect 0, read 1533, write 57, timeout 4489
Requests/sec:    519.94
Transfer/sec:     65.50KB
```

### Lua/http
### Rust/hyper+tokio
