# Contributing

`httpkit` builds with `esy`. If you don't have it, have a read
[here](https://esy.sh).

Hopefully you'll find that `httpkit` is well organized enough for you to jump on
without much handholding.

## Project Structure

The project is structured in 2 different packages:

1. `httpkit`, the main library
2. `httpkit-lwt`, the Lwt bindings


### `httpkit`

HttpKit keeps the core datatypes for working with the DSLs, including functors
and interfaces for creating new backend bindings.

It's structured in the following libraries:

#### Httpkit.Client

In this module you will find submodules for handling responses, and functions
for making requests to servers.

* `Response`, with utility functions for handling responses (such as extracting
  its body).

#### Httpkit.Server

This module includes the core datatypes used to build up safe servers.

It also has the following submodules:

* `Infix`, infix operators for stacking middleware.
* `Middleware`, the underlying middleware stack.
* `Common`, common middleware such as `log` or `router`.

## `httpkit-lwt`

This module includes bindings to an Lwt backend that can be used to run a
server over HTTP, and make requests over HTTPS.

It includes two libraries:

* `Httpkit_lwt.Client`
* `Httpkit_lwt.Server`
