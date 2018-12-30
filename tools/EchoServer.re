open Httpkit;

type state = {req_count: int};

let log: Server.middleware(state, state) =
  ctx => {
    let {Unix.tm_hour, tm_min, tm_sec, _} = Unix.time() |> Unix.localtime;
    let time = Printf.sprintf("%d:%d:%d", tm_hour, tm_min, tm_sec);

    let meth = ctx.req.meth |> Httpaf.Method.to_string;
    let path = ctx.req.target;

    Logs.info(m => m("%s — %s %s", time, meth, path));

    ctx.state;
  };

type other = {
  req_count: int,
  name: string,
};

let inc: Server.middleware(state, other) =
  ctx => {req_count: ctx.state.req_count + 1, name: "what"};

Server.(
  make({req_count: 0})
  |> use(log)
  |> use(inc)
  |> listen(~port=2112, ~on_start=() =>
       Printf.printf("Running on localhost:2112")
     )
);
