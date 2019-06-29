# ⚡️HttpKit — high-level, high-performance HTTP1.1/2 clients/servers in Reason

> NOTE: under heavy reconstruction. Latest stable version was [`660d1c8`](https://github.com/ostera/httpkit/tree/660d1c8b7438d207be2717495d8590a529bf5a1f)

HttpKit is a high-level library for building and consuming web servers over
HTTP, HTTPS, and HTTP2.

It serves as a thin layer over `h2` and `http/af`, and when it can it allows you
to seamlessly transition from one to the other.

0. [Roadmap](#roadmap)
1. [Getting Started](#getting-started)
1. [Running the Examples](#running-the-examples)

## Roadmap

| Feature          | HTTP/1.1 | HTTPS/1.1 | HTTP/2 | HTTPS/2 |
|------------------|----------|-----------|--------|---------|
| Listen as Server | Yes      | No        | Yes    | No      |
| Send Request     | Yes      | Yes       | No     | No      |
| Server Push      | -        | -         | No     | No      |
|                  |          |           |        |         |

## Getting Started

#### Usage

`httpkit` can be used both to build servers and to make requests as a client.

Documentation is still a work-in-progress, but there's examples in the
`examples` section that can give you a better idea of how to use the libraries.
In short:

For making a request:

```reason
open Lwt_result.Infix;

module Httpkit = Httpkit_lwt_unix_httpaf;

let req =
  Httpkit.Request.create(
    ~headers=[("User-Agent", "Reason HttpKit")],
    `GET,
    Uri.of_string("http://api.github.com/repos/ostera/httpkit"),
  );

/* Send over HTTP */
req
|> Httpkit.Client.Http.send
>>= Httpkit.Client.Response.body
|> Lwt_main.run

/* Send over HTTPS */
req
|> Httpkit.Client.Https.send
>>= Httpkit.Client.Response.body
|> Lwt_main.run
```

For making a server:

```reason
module Httpkit = Httpkit_lwt_unix_httpaf;

let port = 8080;

let on_start = (~hoststring) =>
  Logs.app(m => m("Running on %s", hoststring));

let handler: Httpkit.Server.handler =
  (req, reply, kill_server) => {
    let method = req |> Httpkit.Request.meth |> H2.Method.to_string;
    let path = req |> Httpkit.Request.path;
    Logs.app(m => m("%s %s", method, path));
    reply(200, "hi");
    kill_server();
  };

/* Start server over HTTP */
Httpkit.Server.Http.listen(~port, ~address=`Any, ~handler, ~on_start)
|> Lwt_main.run;

/* Start server over HTTPS */
Httpkit.Server.Http.listen(~port, ~address=`Any, ~handler, ~on_start)
|> Lwt_main.run;
```

#### Installing with esy

You can install by dropping the following dependencies in your `package.json`:

```json
{
  "dependencies": {
    "@opam/httpkit": "*",
    "@opam/httpkit-lwt-unix-httpaf": "*",
    "@opam/logs": "*",
    "@opam/fmt": "*",
    // ...
  },
  "resolutions": {
    "@opam/httpkit": "ostera/httpkit:httpkit.opam#f738417",
    "@opam/httpkit-lwt-unix-httpaf": "ostera/httpkit:httpkit-lwt-unix-httpaf.opam#f738417",
  }
}
```

> NOTE: For `httpkit` make sure you're using the latest commit hash!

## Running the Examples

All of the examples are runnable as binaries after compilation, so you can
either run `esy build` and find them within
`./_esy/default/build/default/examples/*.exe` or you can ask dune to run them
for you:

```sh
ostera/httpkit λ esy dune exec ./examples/Request.exe
```
