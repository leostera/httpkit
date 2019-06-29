/** Handle sigpipe internally */
Sys.(set_signal(sigpipe, Signal_ignore));

/** Setup loggers */
Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.Debug));
Logs.set_reporter(Logs_fmt.reporter());

module Httpkit = Httpkit_lwt_unix_h2;

let on_start = () => Logs.app(m => m("Running on localhost:8080"));

let handler = (req, res) => {
  Logs.app(m =>
    m(
      "%s %s",
      req |> Httpkit.Client.Request.meth,
      req |> Httpkit.Client.Request.path,
    )
  );
};

Httpkit.Server.listen(~port=8080, ~host="0.0.0.0", ~handler, ~on_start)
|> Lwt_main.run;
