/** Handle sigpipe internally */
Sys.(set_signal(sigpipe, Signal_ignore));

/** Setup loggers */
Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.Debug));
Logs.set_reporter(Logs_fmt.reporter());

open Httpkit;

module Common = {
  let log: Server.Middleware.ctx('a) => 'a =
    ctx => {
      let {Unix.tm_hour, tm_min, tm_sec, _} = Unix.time() |> Unix.localtime;
      let time = Printf.sprintf("%d:%d:%d", tm_hour, tm_min, tm_sec);

      let meth = ctx.req.meth |> Httpaf.Method.to_string;
      let path = ctx.req.target;

      Logs.info(m => m("%s — %s %s", time, meth, path));

      ctx.state;
    };
};

module App = {
  type state = {req_count: int};
  let initial_state = {req_count: 0};
  type other = {
    req_count: int,
    name: string,
  };
  let inc: Server.Middleware.t(state, other) =
    ctx => {req_count: ctx.state.req_count + 1, name: "what"};
  let json: Server.Middleware.t(other, unit) =
    ctx => {
      let str = ctx.state.req_count |> string_of_int;
      ctx.respond(~status=`OK, str);
    };
};

/* TODO(@ostera):  why aren't we running this? */
let on_start = () => Printf.printf("Running on localhost:9999");

Server.(
  make(App.initial_state)
  |> use(Common.log)
  |> use(App.inc)
  |> reply(App.json)
  |> listen(~port=9999, ~on_start)
  |> Lwt_main.run
);
