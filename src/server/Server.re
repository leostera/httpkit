module Http = Http;

module type M = {
  type ctx('a) = {
    respond:
      (
        ~status: Httpaf.Status.t,
        ~headers: list((string, string))=?,
        string
      ) =>
      unit,
    req: Httpaf.Request.t,
    state: 'a,
  };
  type t('a, 'b) = ctx('a) => 'b;
};

module Middleware: M = {
  type ctx('a) = {
    respond:
      (
        ~status: Httpaf.Status.t,
        ~headers: list((string, string))=?,
        string
      ) =>
      unit,
    req: Httpaf.Request.t,
    state: 'a,
  };

  type t('a, 'b) = ctx('a) => 'b;
};

type stack('i, 'o) =
  | Init('a): stack('a, 'a)
  | Next(Middleware.t('b, 'c), stack('a, 'b)): stack('a, 'c);

let rec run: type i o. (_, Httpaf.Request.t, stack(i, o)) => o =
  (respond, req, stack) =>
    switch (stack) {
    | Init(last) => last
    | Next(f, cont) => f({respond, req, state: run(respond, req, cont)})
    };

type status = [ | `Clean | `Listening | `With_middleware];
type has_response = [ | `No_response | `Responded];
type t('status, 'replied, 'state_in, 'state_out) = {
  state: 'state_in,
  middleware: stack('state_in, 'state_out),
};

let make: 'state => t([ | `Clean], [ | `No_response], 'state, 'state) =
  state => {state, middleware: Init(state)};

let use:
  (Middleware.t('b, 'c), t([< | `Clean | `With_middleware], 'r, 'a, 'b)) =>
  t([ | `With_middleware], 'r, 'a, 'c) =
  (middleware, server) => {
    state: server.state,
    middleware: Next(middleware, server.middleware),
  };

let reply:
  (
    Middleware.t('b, 'c),
    t([< | `Clean | `With_middleware], [ | `No_response], 'a, 'b)
  ) =>
  t([ | `With_middleware], [ | `Responded], 'a, 'c) =
  (middleware, server) => {
    state: server.state,
    middleware: Next(middleware, server.middleware),
  };

let listen:
  (
    ~port: int,
    ~on_start: unit => unit,
    t([ | `With_middleware], [ | `Responded], 'a, 'b)
  ) =>
  Lwt.t(unit) =
  (~port, ~on_start, server) => {
    let request_handler:
      (~closer: unit => unit, Unix.sockaddr) =>
      Httpaf_lwt.Server.request_handler =
      (~closer as _, _client, reqd) => {
        let req = Httpaf.Reqd.request(reqd);
        let respond = (~status, ~headers=?, content) => {
          let headers =
            (
              switch (headers) {
              | None => []
              | Some(hs) => hs
              }
            )
            @ [
              ("Content-Length", content |> String.length |> string_of_int),
            ]
            |> Httpaf.Headers.of_list;
          let res = Httpaf.Response.create(status, ~headers);
          Httpaf.Reqd.respond_with_string(reqd, res, content);
        };
        server.middleware |> run(respond, req) |> ignore;
      };

    let error_handler:
      (
        Unix.sockaddr,
        ~request: Httpaf.Request.t=?,
        Httpaf.Server_connection.error,
        Httpaf.Headers.t => Httpaf.Body.t([ | `write])
      ) =>
      unit =
      (_client, ~request as _=?, _err, _get) => ();

    Http.start(~port, ~on_start, ~request_handler, ~error_handler);
  };

module Infix = {
  let ( *> ) = (s, m) => use(m, s);
  let (<<) = (s, m) => reply(m, s);
};
