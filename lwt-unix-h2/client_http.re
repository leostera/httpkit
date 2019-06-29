open Lwt.Infix;

let send:
  (~config: H2.Config.t=?, Httpkit.Request.t) =>
  Lwt_result.t(
    (H2.Response.t, H2.Body.t([ | `read])),
    [> | `Connection_error(H2.Client_connection.error)],
  ) =
  (~config=H2.Config.default, req) => {
    let uri = Httpkit.Request.uri(req);

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
      | None => "443"
      | Some(number) => string_of_int(number)
      };

    Lwt_unix.getaddrinfo(host, port, [Unix.(AI_FAMILY(PF_INET))])
    >>= (
      addresses => {
        Logs.debug(m => m("Got address..."));
        let socket_addr = List.hd(addresses).Unix.ai_addr;
        let socket = Lwt_unix.socket(Unix.PF_INET, Unix.SOCK_STREAM, 0);

        Lwt_unix.connect(socket, socket_addr)
        >>= (
          () => {
            Logs.debug(m => m("Opened socket..."));
            let (response_received, notify_response_received) = Lwt.wait();
            let response_handler = response_handler(notify_response_received);
            let error_handler = error_handler(notify_response_received);

            let write_body = request_body => {
              Logs.debug(m => m("Writing body..."));
              switch (Httpkit.Request.body(req)) {
              | None => ()
              | Some(str) => H2.Body.write_string(request_body, str)
              };
              H2.Body.close_writer(request_body);
              response_received >>= (x => x);
            };

            let request = Client_request.of_httpkit_request(req);

            H2_lwt_unix.Client.create_connection(
              ~config,
              ~error_handler,
              socket,
            )
            >>= (
              connection => {
                H2_lwt_unix.Client.request(
                  connection,
                  request,
                  ~error_handler,
                  ~response_handler,
                )
                |> write_body;
              }
            );
          }
        );
      }
    );
  };
