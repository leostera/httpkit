open Lwt_result.Infix;

/** Handle sigpipe internally */
Sys.(set_signal(sigpipe, Signal_ignore));

/** Setup loggers */
Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.Debug));
Logs.set_reporter(Logs_fmt.reporter());

switch (
  Httpkit_lwt.Client.(
    "https://api.github.com/users/ostera"
    |> Uri.of_string
    |> Https.send(~headers=[("User-Agent", "Reason HttpKit")])
    >>= Response.body
    |> Lwt_main.run
  )
) {
| exception e => Logs.err(m => m("%s", Printexc.to_string(e)))
| Ok(body) => Printf.printf("%s", body)
| Error(_) => Logs.err(m => m("Something went wrong!!!"))
};

switch (
  Httpkit_lwt.Client.(
    "http://api.github.com/repo/ostera/httpkit"
    |> Uri.of_string
    |> Http.send(~headers=[("User-Agent", "Reason HttpKit")])
    >>= Response.body
    |> Lwt_main.run
  )
) {
| exception e => Logs.err(m => m("%s", Printexc.to_string(e)))
| Ok(body) => Printf.printf("%s", body)
| Error(_) => Logs.err(m => m("Something went wrong!!!"))
};
