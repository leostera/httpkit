module Http: {
  let start:
    (
      ~port: int,
      ~on_start: unit => unit,
      ~request_handler: (~closer: unit => unit, Unix.sockaddr) =>
                        Httpaf_lwt.Server.request_handler,
      ~error_handler: Unix.sockaddr => Httpaf.Server_connection.error_handler
    ) =>
    Lwt.t(unit);
};

module Middleware: {
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

type status = [ | `Clean | `Listening | `With_middleware];
type has_response = [ | `No_response | `Responded];
type t('status, 'has_response, 'state_in, 'state_out);

let make: 'state => t([ | `Clean], [ | `No_response], 'state, 'state);

let use:
  (Middleware.t('b, 'c), t([< | `Clean | `With_middleware], 'r, 'a, 'b)) =>
  t([ | `With_middleware], 'r, 'a, 'c);

let reply:
  (
    Middleware.t('b, 'c),
    t([< | `Clean | `With_middleware], [ | `No_response], 'a, 'b)
  ) =>
  t([ | `With_middleware], [ | `Responded], 'a, 'c);

let listen:
  (
    ~port: int,
    ~on_start: unit => unit,
    t([ | `With_middleware], [ | `Responded], 'a, 'b)
  ) =>
  Lwt.t(unit);

module Infix: {
  let ( *> ):
    (t([< | `Clean | `With_middleware], 'r, 'a, 'b), Middleware.t('b, 'c)) =>
    t([ | `With_middleware], 'r, 'a, 'c);

  let (<<):
    (
      t([< | `Clean | `With_middleware], [ | `No_response], 'a, 'b),
      Middleware.t('b, 'c)
    ) =>
    t([ | `With_middleware], [ | `Responded], 'a, 'c);
};
