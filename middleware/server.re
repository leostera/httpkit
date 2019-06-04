module Middleware = Httpkit_middleware.Middleware;

module type BASE = {
  type response_status;
  type request;
};

module type SERVER = {
  type response_status;

  type request;

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
                  ~status: response_status,
                  ~headers: list((string, string))=?,
                  string
                ) =>
                unit,
      ~request: request,
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
};

module Make =
       (M: BASE)

         : (
           SERVER with
             type request = M.request and
             type response_status = M.response_status
       ) => {
  type request = M.request;

  type response_status = M.response_status;

  type middleware('state_in, 'state_out) =
    Middleware.Model.middleware('state_in, 'state_out);

  type status = [ | `Clean | `Listening | `With_middleware];
  type has_response = [ | `No_response | `Responded];
  type t('status, 'replied, 'state_in, 'state_out) = {
    state: 'state_in,
    middleware: Middleware.Chain.t('state_in, 'state_out),
  };

  let make = state => {state, middleware: Middleware.Chain.Init(state)};

  let run = server => Middleware.Chain.run(server.middleware);

  let use = (middleware, server) => {
    state: server.state,
    middleware: Middleware.Chain.Next(middleware, server.middleware),
  };

  let reply = (middleware, server) => {
    state: server.state,
    middleware: Middleware.Chain.Next(middleware, server.middleware),
  };

  module Infix = {
    let ( *> ) = (s, m) => use(m, s);
    let (<<) = (s, m) => reply(m, s);
  };
};
