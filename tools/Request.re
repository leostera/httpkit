open Lwt_result.Infix;

/** Handle sigpipe internally */
Sys.(set_signal(sigpipe, Signal_ignore));

/** Setup loggers */
Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.Debug));
Logs.set_reporter(Logs_fmt.reporter());

/** Setup async reporting */
switch (
  "https://api.github.com/"
  |> Uri.of_string
  |> Httpkit.Client.Https.send(~headers=[("User-Agent", "Reason HttpKit")])
  >>= Httpkit.Client.Response.body
  |> Lwt_main.run
) {
| exception e => Printf.printf("%s", Printexc.to_string(e))
| Ok(body) => Printf.printf("%s", body)
| Error(_) => Printf.printf("Something went wrong!")
};
