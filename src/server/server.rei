type status = [ | `Clean | `Listening | `With_middleware];

type has_response = [ | `No_response | `Responded];

type middleware('state_in, 'state_out) =
  Httpkit_middleware.Middleware.Model.middleware('state_in, 'state_out);

type t('status, 'has_response, 'state_in, 'state_out);

let make: 'state => t([ | `Clean], [ | `No_response], 'state, 'state);

let use:
  (middleware('b, 'c), t([< | `Clean | `With_middleware], 'r, 'a, 'b)) =>
  t([ | `With_middleware], 'r, 'a, 'c);

let reply:
  (
    middleware('b, 'c),
    t([< | `Clean | `With_middleware], [ | `No_response], 'a, 'b)
  ) =>
  t([ | `With_middleware], [ | `Responded], 'a, 'c);

let run:
  (
    t('a, 'b, 'c, 'd),
    ~closer: unit => unit,
    ~respond: (
                ~status: Httpaf.Status.t,
                ~headers: list((string, string))=?,
                string
              ) =>
              unit,
    ~request: Httpaf.Request.t,
    ~body: unit => option(string)
  ) =>
  'd;

module Infix: {
  let ( *> ):
    (t([< | `Clean | `With_middleware], 'r, 'a, 'b), middleware('b, 'c)) =>
    t([ | `With_middleware], 'r, 'a, 'c);

  let (<<):
    (
      t([< | `Clean | `With_middleware], [ | `No_response], 'a, 'b),
      middleware('b, 'c)
    ) =>
    t([ | `With_middleware], [ | `Responded], 'a, 'c);
};
