# Contributing

Hopefully you'll find that `httpkit` is well organized enough for you to jump on
without much handholding.

## Project Structure

The project is structured in 4 different packages:

1. `Httpkit`, the main public API of Httpkit
2. `Client`, containing code related to making requests
3. `Server`, with the middleware stack for building servers
4. `Transports`, the HTTP/HTTPS implementations used by Client and Server

### Httpkit.Client

In this module you will find submodules for handling responses, and functions
for making requests to servers.

* `Response`, with utility functions for handling responses (such as extracting
  its body).

### Httpkit.Server

This module includes the core datatypes used to build up safe servers.

It also has the following submodules:

* `Infix`, infix operators for stacking middleware.
* `Middleware`, the underlying middleware stack.
* `Common`, common middleware such as `log` or `router`.

### Httpkit.Transports

This module includes bindings to Lwt that can be used to run a server over HTTP.

It is clear that this module should be part of a different library,
`httpkit-lwt`, and include bindings for both the `Httpkit.Server` and
`Httpkit.Client` modules.
