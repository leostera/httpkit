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
    body_string: option(string),
    closer: unit => unit,
    state: 'a,
  };

  type t('a, 'b) = ctx('a) => 'b;

  type stack('i, 'o) =
    | Init('a): stack('a, 'a)
    | Next(t('b, 'c), stack('a, 'b)): stack('a, 'c);

  let run:
    (
      unit => unit,
      (
        ~status: Httpaf.Status.t,
        ~headers: list((string, string))=?,
        string
      ) =>
      unit,
      Httpaf.Request.t,
      option(string),
      stack('i, 'o)
    ) =>
    'o;
};

module Common: {
  let log: Middleware.t('a, 'a);

  type headers = list((string, string));
  type path = list(string);

  type route_handler('a) =
    (Middleware.ctx('a), path) =>
    [
      | `OK(string)
      | `With_headers(Httpaf.Status.t, headers, string)
      | `With_status(Httpaf.Status.t, string)
      | `Unmatched
    ];

  let router:
    route_handler('a) =>
    Middleware.t('a, [ | `Replied(Httpaf.Status.t, headers, string)]);

  module BodyParser: {let json: Middleware.t('a, option(string));};
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

let middleware: t('s, 'r, 'a, 'b) => Middleware.stack('a, 'b);

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
