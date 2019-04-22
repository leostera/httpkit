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

let https_url = "https://api.npms.io/v2/package/bsdoc";
Logs.app(m => m("Requesting: %s", https_url));
switch (
  Httpkit.Client.Request.create(
    ~headers=[("User-Agent", "Reason HttpKit")],
    `GET,
    https_url |> Uri.of_string,
  )
  |> Httpkit_lwt_unix_h2.Client.Https.send(~client=`No_authentication)
  >>= Httpkit_lwt_unix_h2.Client.Response.body
  |> Lwt_main.run
) {
| exception e => Logs.err(m => m("%s", Printexc.to_string(e)))
| Ok(body) => Logs.app(m => m("Response: %s", body))
| Error(_) => Logs.err(m => m("Something went wrong!!!"))
};
