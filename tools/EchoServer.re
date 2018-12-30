/** Handle sigpipe internally */
Sys.(set_signal(sigpipe, Signal_ignore));

/** Setup loggers */
Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.Debug));
Logs.set_reporter(Logs_fmt.reporter());

open Httpkit;

type state = {req_count: int};

let log: Server.Middleware.t(state, state) =
  ctx => {
    let {Unix.tm_hour, tm_min, tm_sec, _} = Unix.time() |> Unix.localtime;
    let time = Printf.sprintf("%d:%d:%d", tm_hour, tm_min, tm_sec);

    let meth = ctx.req.meth |> Httpaf.Method.to_string;
    let path = ctx.req.target;

    Logs.info(m => m("%s — %s %s", time, meth, path));

    {req_count: ctx.state.req_count};
  };

type other = {
  req_count: int,
  name: string,
};

let inc: Server.Middleware.t(state, other) =
  ctx => {req_count: ctx.state.req_count + 1, name: "what"};

let json: Server.Middleware.t(other, Server.Middleware.replied) =
  ctx => {
    let str = ctx.state.req_count |> string_of_int;
    ctx.respond(~status=`OK, str);
  };

let on_start = () => Printf.printf("Running on localhost:2112");

Server.(
  make({req_count: 0})
  |> use(log)
  |> use(inc)
  |> reply(json)
  |> listen(~port=9999, ~on_start)
  |> Lwt_main.run
);
