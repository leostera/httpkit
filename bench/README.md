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

### Python
