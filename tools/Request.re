open Lwt_result.Infix;

Sys.(set_signal(sigpipe, Signal_ignore));

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
