type ctx('a) = {
  req: Httpaf.Request.t,
  state: 'a,
};

type middleware('a, 'b) = ctx('a) => 'b;

type mid('i, 'o) =
  | Noop('a): mid('a, 'a)
  | Mid(middleware('b, 'c), mid('a, 'b)): mid('a, 'c);

type status = [ | `Clean | `Listening | `With_middleware];
type t('status, 'state_in, 'state_out) = {
  state: 'state_in,
  middleware: mid('state_in, 'state_out),
};

let make: 'state => t([ | `Clean], 'state, 'state) =
  state => {state, middleware: Noop(state)};

let use:
  (middleware('b, 'c), t([< | `Clean | `With_middleware], 'a, 'b)) =>
  t([ | `With_middleware], 'a, 'c) =
  (middleware, server) => {
    state: server.state,
    middleware: Mid(middleware, server.middleware),
  };

let listen:
  (~port: int, ~on_start: unit => unit, t([ | `With_middleware], 'a, 'b)) =>
  t([ | `Listening], 'a, 'b) =
  (~port as _, ~on_start as _, server) => {
    state: server.state,
    middleware: server.middleware,
  };
