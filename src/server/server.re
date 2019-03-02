module Middleware = Httpkit_middleware.Middleware;

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
