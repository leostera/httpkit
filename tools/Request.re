open Lwt_result.Infix;

/** Handle sigpipe internally */
Sys.(set_signal(sigpipe, Signal_ignore));

/** Setup loggers */
Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.Debug));
Logs.set_reporter(Logs_fmt.reporter());

/**

  Sample HTTPS Request using No Authentication :tm:

*/

let https_url = "https://api.github.com/users/ostera";
Logs.app(m => m("Requesting: %s", https_url));
switch (
  Httpkit_lwt.Client.(
    Httpkit.Client.Request.create(
      ~headers=[("User-Agent", "Reason HttpKit")],
      `GET,
      https_url |> Uri.of_string,
    )
    |> Https.send
    >>= Response.body
    |> Lwt_main.run
  )
) {
| exception e => Logs.err(m => m("%s", Printexc.to_string(e)))
| Ok(body) => Logs.app(m => m("Response: %s", body))
| Error(_) => Logs.err(m => m("Something went wrong!!!"))
};

/**

  Sample HTTPS Request using an Authentication config from
  files.

*/

let https_url = "https://api.github.com/users/ostera";
let tls_config =
  Httpkit_lwt.Client.Https.Config.from_pems(
    ~cert="./cert.pem" |> Fpath.v,
    ~priv_key="./priv_key" |> Fpath.v,
    ~ca="./cert.pem" |> Fpath.v,
    (),
  );

Logs.app(m => m("Requesting: %s", https_url));
switch (
  Httpkit_lwt.Client.(
    Httpkit.Client.Request.create(
      ~headers=[("User-Agent", "Reason HttpKit")],
      `GET,
      https_url |> Uri.of_string,
    )
    |> Https.(send(~config=tls_config))
    >>= Response.body
    |> Lwt_main.run
  )
) {
| exception e => Logs.err(m => m("%s", Printexc.to_string(e)))
| Ok(body) => Logs.app(m => m("Response: %s", body))
| Error(_) => Logs.err(m => m("Something went wrong!!!"))
};

/**

  Sample HTTP Request.

*/
/* NOTE: the HelloWorld server in tools/HelloWorld.re can help you run this :) */
let http_url = "http://localhost:9999/awesome/posum";
Logs.app(m => m("Requesting: %s", http_url));
switch (
  Httpkit_lwt.Client.(
    Httpkit.Client.Request.create(
      ~headers=[("User-Agent", "Reason HttpKit")],
      `GET,
      http_url |> Uri.of_string,
    )
    |> Http.send
    >>= Response.body
    |> Lwt_main.run
  )
) {
| exception e => Logs.err(m => m("%s", Printexc.to_string(e)))
| Ok("") => Logs.app(m => m("Empty body!"))
| Ok(body) => Logs.app(m => m("Response: %s", body))
| Error(_) => Logs.err(m => m("Something went wrong!!!"))
};
