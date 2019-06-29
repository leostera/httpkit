/** Handle sigpipe internally */
Sys.(set_signal(sigpipe, Signal_ignore));

/** Setup loggers */
Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.Debug));
Logs.set_reporter(Logs_fmt.reporter());

module Httpkit = Httpkit_lwt_unix_httpaf;

let port = 8080;

let on_start = (~hoststring) =>
  Logs.app(m => m("Running on %s", hoststring));

let handler: Httpkit.Server.handler =
  (req, reply, close) => {
    let method = req |> Httpkit.Request.meth |> H2.Method.to_string;
    let path = req |> Httpkit.Request.path;
    Logs.app(m => m("%s %s", method, path));
    reply(200, "hi");
    close();
  };

Httpkit.Server.Http.listen(~port, ~address=`Any, ~handler, ~on_start)
|> Lwt_main.run;
