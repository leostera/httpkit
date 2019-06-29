open Lwt.Infix;
open Httpkit.Client;

let send:
  (~config: Httpaf.Config.t=?, Httpkit.Client.Request.t) =>
  Lwt_result.t(
    (Httpaf.Response.t, Httpaf.Body.t([ | `read])),
    [> | `Connection_error(Httpaf.Client_connection.error)],
  ) =
  (~config=Httpaf.Config.default, req) => {
    let uri = Request.uri(req);

    let response_handler = (notify_response_received, response, response_body) => {
      Logs.debug(m => m("Handling response..."));
      Lwt.wakeup_later(
        notify_response_received,
        (response, response_body) |> Lwt_result.return,
      );
    };

    let error_handler = (notify_response_received, error) => {
      Logs.debug(m => m("Handling errors..."));
      Lwt.wakeup_later(
        notify_response_received,
        `Connection_error(error) |> Lwt_result.fail,
      );
    };

    let host = Uri.host_with_default(uri);
    let port =
      switch (Uri.port(uri)) {
      | None => "80"
      | Some(number) => string_of_int(number)
      };

    Logs.debug(m => m("Getting address for %s:%s", host, port));
    Lwt_unix.getaddrinfo(host, port, [Unix.(AI_FAMILY(PF_INET))])
    >>= (
      addresses => {
        let socket_addr = List.hd(addresses).Unix.ai_addr;
        let socket = Lwt_unix.socket(Unix.PF_INET, Unix.SOCK_STREAM, 0);
        Logs.debug(m => m("Opening socket to %s:%s", host, port));
        Lwt_unix.connect(socket, socket_addr)
        >>= (
          () => {
            let (response_received, notify_response_received) = Lwt.wait();
            let response_handler = response_handler(notify_response_received);
            let error_handler = error_handler(notify_response_received);

            let write_body = request_body => {
              switch (Request.body(req)) {
              | None => ()
              | Some(str) => Httpaf.Body.write_string(request_body, str)
              };
              Httpaf.Body.close_writer(request_body);
              Logs.debug(m => m("Request sent. Awaiting for response..."));
              response_received >>= (x => x);
            };

            let request = Client_request.of_httpkit_request(req);

            Logs.debug(m =>
              m("Sending request: \n\n%s", req |> Request.to_string)
            );
            Httpaf_lwt_unix.Client.request(
              ~config,
              ~error_handler,
              ~response_handler,
              socket,
              request,
            )
            |> write_body;
          }
        );
      }
    );
  };
