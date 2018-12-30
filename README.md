# ⚡️HttpKit — High-level, High-performance HTTP(S) Clients/Servers in Reason/OCaml

Hello! `httpkit` is not much more than an experiment in API ergonomics backed
by a type-system, that in this case is aimed both at being delightful to use,
impossible to misuse, and blazingly fast.

It was born out of necessity really, while working on the `twitchboard` cli
tool during one of the [ReasonableCoding](https://twitch.tv/ostera) streams,
because working directly with
[`http/af`](https://github.com/inhabitedtype/httaf) is quite low-level.

1. [Getting Started](#getting-started)
2. [Principles: Server-side](#principles-server-side)
2. [Principles: Client-side](#principles-client-side)

## Getting Started

#### Usage

`httpkit` can be used both to build servers and to make requests as a client.

Documentation is still a work-in-progress, but there's examples in the `tools`
section that can give you a better idea of how to use the libraries. In short:

For making a request:

```reason
switch (
  "https://api.github.com/users/ostera"
  |> Uri.of_string
  |> Httpkit.Client.Https.send(~headers=[("User-Agent", "Reason HttpKit")])
  >>= Httpkit.Client.Response.body
  |> Lwt_main.run
) {
| exception e => Printf.printf("%s", Printexc.to_string(e))
| Ok(body) => Printf.printf("%s", body)
| Error(_) => Printf.printf("Something went wrong!")
};

```

For making a server:

```reason
Server.(
  make(App.initial_state)
  |> use(Common.log)
  |> use(App.inc)
  |> reply(App.json)
  |> listen(~port=9999, ~on_start)
  |> Lwt_main.run
);
```

I encourage you to read on through the Principles and let me know what you
think! 🙌🏼

#### Installing

You can install by pinning with opam:

```sh
$ opam pin add httpkit git+https://github.com/ostera/httpkit
```

## Principles: Server-side

The following principles guide the server-side library:

1. [Servers should be safe to build](#servers-should-be-safe-to-build)
2. [Servers should be buildable using composable parts](#servers-should-be-buildable-using-composable-parts)
3. [Servers should be responsive to all requests](#servers-should-be-responsive-to-all-requests)
4. [Servers should be fast to run](#servers-should-be-fast-to-run)

In other words, `httpkit` should make it impossibly hard to do the wrong thing
(i.e, [_Pit of
Success_](https://blog.codinghorror.com/falling-into-the-pit-of-success/)),
allow maximum code-reuse, with the guarantee that all requests will be handled,
and should be lightning fast.

### Servers should be safe to build

Have you ever forgotten to call a function that did some side-effect, a
necessary one, before calling the next one? A few come to mind:

* Read a file after you close it
* Write to a read-only buffer before aquiring a lock
* Forget to respond to a request deep down within your API
* ...and many more.

We all do this, we realize the problem fairly quickly, we fix it. But if we have
tools that can help us with this, why aren't we using them more extensively?

By making heavy use of type-state, `httpkit` aims to only let you build servers
that do _The Right Thing_:

* Servers can not be started if they don't always reply to a request
* All middleware must compose type-safely 

I believe that these 2 invariants will make building web servers much safer. 

If you manage to build a server that is not safe, **this is a bug**.

### Servers should be buildable using composable parts

The first thing that came to mind when thinking of composable servers was
`connect`, of Node.js fame. The library didn't invent, but popularized the
concept of _middleware_ in the Web community at large.

_middleware_ is simply a function from a context value to the next state value
to be handled by the next piece of middleware. Chaining them means a request is
handled across multiple functions until one of them responds and the chain ends.

```reason
type middleware('a, 'b) = ctx('a) => 'b;
```

The context itself is less relevant, but the important bit is that it's really
just a function. And if you build them carefully, functions can compose very
well.

Let's see an example. Here's a logging middleware that by contract promises us
not touch the state:

```reason
/**
  Because the state is a type variable, we can never know what it actually is,
  and because of this we can't really change it!

  This middleware ensures us that whatever state comes in, will come out.
  */
let log : middleware('a, 'a) = (ctx) => {

  /* Put together a timestamp */
  let {Unix.tm_hour, tm_min, tm_sec, _} = Unix.time() |> Unix.localtime;
  let time = Printf.sprintf("%d:%d:%d", tm_hour, tm_min, tm_sec);

  /* Get the string representation of the METHOD */
  let meth = ctx.req.meth |> Httpaf.Method.to_string;

  /* Get the path of the request, such as /users/1/edit */
  let path = ctx.req.target;
   
  /* Log things! */
  Logs.info(m => m("%s — %s %s", time, meth, path));

  /* Return the next state */
  ctx.state;

};
```

It's easy to see that given any `ctx` value, we can call `log` on it repeatedly:

```reason
ctx |> log |> log |> log |> log
```

Assuming that our composition operator (`|>`) takes care of putting things back
into a `ctx('a)`. We will go back to this in a second.

Of course this is ludicrous 😅 but if we instead of repeating a log function had
multiple bits and pieces of middleware that performed different things? That
sounds more useful. As long as the `output` type of the previous middleware
matches the `input` type of the next, we can compose them in long, type-safe
chains.

```reason
ctx                    /* <-- initial context value! */
|> log                 /* No change here, we just log stuff */
|> Analytics.trackReq  /* No change here, register analytics */
|> Auth.track          /* Auth.track returns `Valid(user) | `Auth_error(msg) */
|> Router.handle       /* Routerhandle returns `Ok(content) | `Not_found(msg) */
|> Responder.respond;  /* Turns `Ok(content) into HTTP 200, `Not_found to 404 */
```

And in this way we can compose request paths with smaller functions, most of
which can be built once and reused everywhere, and the rest of which will be
safely composed.

The last missing piece here is this composition operator. Clearly `|>` will not
cut it. For this purpose, `httpkit` provides a combinator called `use`.

`use` takes a middleware, and a server, and returns another server that has that
middleware "stacked". If we change the snippet of code above to use it, it will
look like this:

```reason
ctx
|> use(log)
|> use(Analytics.trackReq)
|> use(Auth.track)
|> use(Router.handle)
|> use(Responder.respond);
```

Or if you like operators (like I do) you can use `*>` and `<<`:

```reason
ctx
*> log
*> Analytics.trackReq
*> Auth.track
*> Router.handle
<< Responder.respond;
```

> Note: there's an additional combinator called `reply`, which uses the operator
> `<<` and signals _a response_ to a client.

If you manage to build middleware that should but does not compose safely,
**this is a bug**.

### Servers should be responsive to all requests

TBD. I'm working on it 👨‍🏫 

### Servers should be fast to run

The main idea here is that `httpkit` should have a negligible impact on the
performance of your service. So if your API is slow, and it's because `httpkit`
is taking too long, then *that's a bug*.

Of course it'll take time to define what the maximum overhead should be (and how
it should scale given you throw more middleware at it), but as an experiment it's doing quite well.

Let's see an echo server (which you can see in the `tools` directory of this
repo). First the raw version that does not use anything but some types from
`httpkit`:

```sh
ostera/httpkit λ wrk2 --threads=12 --connections=400 --duration=30s --rate 30K http://localhost:9999/what
Running 30s test @ http://localhost:9999/what
  12 threads and 400 connections
  Thread calibration: mean lat.: 3103.933ms, rate sampling interval: 11304ms
  Thread calibration: mean lat.: 3074.874ms, rate sampling interval: 11141ms
  Thread calibration: mean lat.: 3190.094ms, rate sampling interval: 11370ms
  Thread calibration: mean lat.: 2974.283ms, rate sampling interval: 10543ms
  Thread calibration: mean lat.: 2987.974ms, rate sampling interval: 11091ms
  Thread calibration: mean lat.: 2895.381ms, rate sampling interval: 10919ms
  Thread calibration: mean lat.: 2908.596ms, rate sampling interval: 10870ms
  Thread calibration: mean lat.: 3164.646ms, rate sampling interval: 11526ms
  Thread calibration: mean lat.: 3000.368ms, rate sampling interval: 11182ms
  Thread calibration: mean lat.: 3051.481ms, rate sampling interval: 11386ms
  Thread calibration: mean lat.: 3005.150ms, rate sampling interval: 11165ms
  Thread calibration: mean lat.: 2956.432ms, rate sampling interval: 10960ms
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    12.55s     3.75s   21.20s    59.01%
    Req/Sec     0.91k    15.95     0.95k    75.00%
  326792 requests in 30.01s, 12.15MB read
  Socket errors: connect 0, read 97, write 4, timeout 11
Requests/sec:  10890.16
Transfer/sec:    414.76KB
```

That is a whopping 10890 Requests per Second, courtesy of `http/af` and `lwt`.
Seriously, have a look at those libraries.

The echo server above just bootstraps the minimum infrastructure needed to run
a logging middleware, and integer incrementing middleware, and the json replier.

Below is the exact same servers, with the exact same handlers, built with
`httpkit`:

```sh
ostera/httpkit λ wrk2 --threads=12 --connections=400 --duration=30s --rate 30K http://localhost:9999/what
Running 30s test @ http://localhost:9999/what
  12 threads and 400 connections
  Thread calibration: mean lat.: 3092.528ms, rate sampling interval: 11714ms
  Thread calibration: mean lat.: 3241.606ms, rate sampling interval: 12107ms
  Thread calibration: mean lat.: 3183.128ms, rate sampling interval: 11763ms
  Thread calibration: mean lat.: 3235.211ms, rate sampling interval: 12009ms
  Thread calibration: mean lat.: 3109.703ms, rate sampling interval: 11780ms
  Thread calibration: mean lat.: 3214.701ms, rate sampling interval: 12017ms
  Thread calibration: mean lat.: 3174.243ms, rate sampling interval: 11862ms
  Thread calibration: mean lat.: 3067.300ms, rate sampling interval: 11575ms
  Thread calibration: mean lat.: 3066.558ms, rate sampling interval: 11755ms
  Thread calibration: mean lat.: 3101.464ms, rate sampling interval: 11722ms
  Thread calibration: mean lat.: 3084.410ms, rate sampling interval: 11829ms
  Thread calibration: mean lat.: 3075.844ms, rate sampling interval: 11444ms
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    12.87s     3.62s   20.76s    58.95%
    Req/Sec     0.92k    18.58     0.95k    58.33%
  326230 requests in 30.00s, 12.13MB read
  Socket errors: connect 0, read 40, write 3, timeout 4
Requests/sec:  10874.67
Transfer/sec:    414.17KB
```

And it clocked 10874 Requests per Second. That is just about 16 RPS less than
barebones `http/af`.

I'm running this benchmark essentially on every change to make sure that the
abstractions and safety that `httpkit` give you have close to zero impact on the
performance of your service.

## Principles: Client-side

TBD! I'm working on it 👨‍🏫 
