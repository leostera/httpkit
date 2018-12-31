type status = [ | `Clean | `Listening | `With_middleware];
type has_response = [ | `No_response | `Responded];
type t('status, 'replied, 'state_in, 'state_out) = {
  state: 'state_in,
  middleware: Middleware.stack('state_in, 'state_out),
};

let make = state => {state, middleware: Middleware.Init(state)};

let middleware = server => server.middleware;

let use = (middleware, server) => {
  state: server.state,
  middleware: Middleware.Next(middleware, server.middleware),
};

let reply = (middleware, server) => {
  state: server.state,
  middleware: Middleware.Next(middleware, server.middleware),
};

module Infix = {
  let ( *> ) = (s, m) => use(m, s);
  let (<<) = (s, m) => reply(m, s);
};

module Middleware = Middleware;
