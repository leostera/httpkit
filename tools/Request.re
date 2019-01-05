open Lwt_result.Infix;

/** Handle sigpipe internally */
Sys.(set_signal(sigpipe, Signal_ignore));

/** Setup loggers */
Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.Debug));
Logs.set_reporter(Logs_fmt.reporter());

/** Setup async reporting */
switch (
  "https://api.github.com/users/ostera"
  |> Uri.of_string
  |> Httpkit_lwt.Client.Https.send(
       ~headers=[("User-Agent", "Reason HttpKit")],
     )
  >>= Httpkit_lwt.Client.Response.body
  |> Lwt_main.run
) {
| exception e => Logs.err(m => m("%s", Printexc.to_string(e)))
| Ok(body) => Printf.printf("%s", body)
| Error(_) => Logs.err(m => m("Something went wrong!!!"))
};
