open Lwt_result.Infix;

/** Handle sigpipe internally */
Sys.(set_signal(sigpipe, Signal_ignore));

/** Setup loggers */
Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.Debug));
Logs.set_reporter(Logs_fmt.reporter());

let api_response =
  "https://api.github.com/"
  |> Uri.of_string
  |> Httpkit.Client.Https.send
  >>= Httpkit.Client.Response.body
  |> Lwt_main.run;

switch (api_response) {
| Ok(body) => Printf.printf("%s", body)
| Error(_) => Printf.printf("Something went wrong!")
};
