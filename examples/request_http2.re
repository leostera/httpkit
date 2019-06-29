open Lwt_result.Infix;

/** Handle sigpipe internally */
Sys.(set_signal(sigpipe, Signal_ignore));

/** Setup loggers */
Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.Debug));
Logs.set_reporter(Logs_fmt.reporter());

module Httpkit = Httpkit_lwt_unix_h2;

/**
  Sample HTTPS Request using No Authentication :tm:
*/

let https_url = Sys.argv[1];
Logs.app(m => m("Requesting: %s", https_url));
switch (
  Httpkit.Client.Request.create(
    ~headers=[
      ("User-Agent", "Reason HttpKit"),
      ("Accept", "*/*"),
    ],
    `GET,
    https_url |> Uri.of_string,
  )
  |> Httpkit.Client.Https.send
  >>= Httpkit.Client.Response.body
  |> Lwt_main.run
) {
| exception e => Logs.err(m => m("%s", Printexc.to_string(e)))
| Ok(body) => Logs.app(m => m("Response: %s", body))
| Error(`Connection_error(`Invalid_response_body_length(req))) =>
  let str = Buffer.create(1024);
  let fmt = Format.formatter_of_buffer(str);
  H2.Response.pp_hum(fmt, req);
  Logs.err(m => m("Connection Error (Invalid response body length): %s", str |> Buffer.to_bytes |> Bytes.to_string));
| Error(`Connection_error(`Malformed_response(str))) =>
  Logs.err(m => m("Connection Error (Malformed response): %s", str))
| Error(`Connection_error(`Exn(ex))) =>
  Logs.err(m => m("Connection Error (Exception): %s", ex |> Printexc.to_string))
| Error(`Connection_error(`Protocol_error)) =>
  Logs.err(m => m("Connection Error (Protocol Error)"));
};
