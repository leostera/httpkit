open Lwt_result.Infix;

/** Handle sigpipe internally */
Sys.(set_signal(sigpipe, Signal_ignore));

/** Setup loggers */
Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.Debug));
Logs.set_reporter(Logs_fmt.reporter());

let https_url = "https://api.github.com/users/ostera";
Logs.app(m => m("Requesting: %s", https_url));
switch (
  Httpkit_lwt.Client.(
    https_url
    |> Uri.of_string
    |> Https.send(~headers=[("User-Agent", "Reason HttpKit")])
    >>= Response.body
    |> Lwt_main.run
  )
) {
| exception e => Logs.err(m => m("%s", Printexc.to_string(e)))
| Ok(body) => Logs.app(m => m("Response: %s", body))
| Error(_) => Logs.err(m => m("Something went wrong!!!"))
};

/* NOTE: the HelloWorld server in tools/HelloWorld.re can help you run this :) */
let http_url = "http://localhost:9999/awesome/posum";
Logs.app(m => m("Requesting: %s", https_url));
switch (
  Httpkit_lwt.Client.(
    http_url
    |> Uri.of_string
    |> Http.send(~headers=[("User-Agent", "Reason HttpKit")])
    >>= Response.body
    |> Lwt_main.run
  )
) {
| exception e => Logs.err(m => m("%s", Printexc.to_string(e)))
| Ok("") => Logs.app(m => m("Empty body!"))
| Ok(body) => Logs.app(m => m("Response: %s", body))
| Error(_) => Logs.err(m => m("Something went wrong!!!"))
};
