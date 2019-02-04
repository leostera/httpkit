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

let on_start = () => Printf.printf("Running on localhost:2112");

let request_handler:
  (~closer: unit => unit, Unix.sockaddr) => Httpaf_lwt.Server.request_handler =
  (~closer, _client, reqd) => {
    let req = Httpaf.Reqd.request(reqd);
    let respond = (~status, ~headers=?, content) => {
      let headers =
        (
          switch (headers) {
          | None => []
          | Some(hs) => hs
          }
        )
        @ [("Content-Length", content |> String.length |> string_of_int)]
        |> Httpaf.Headers.of_list;
      let res = Httpaf.Response.create(status, ~headers);
      Httpaf.Reqd.respond_with_string(reqd, res, content);
    };
    Httpkit.Server.Middleware.(
      Lwt.Infix.(
        Httpkit_lwt.Server.Request.read_body(reqd)
        >|= (
          body_string => {
            let body = () => body_string;
            let ctx = {closer, req, respond, body, state: App.initial_state};
            /* manually run middlewares */
            Common.log(ctx)
            |> (state => App.inc({closer, req, respond, body, state}))
            |> (state => App.json({closer, req, respond, body, state}));
          }
        )
        |> ignore
      )
    );
  };

let error_handler:
  (
    Unix.sockaddr,
    ~request: Httpaf.Request.t=?,
    Httpaf.Server_connection.error,
    Httpaf.Headers.t => Httpaf.Body.t([ | `write])
  ) =>
  unit =
  (_client, ~request as _=?, _err, _get) => ();

Httpkit_lwt.Server.(
  Http.start(~port=9999, ~on_start, ~request_handler, ~error_handler)
)
|> Lwt_main.run;
