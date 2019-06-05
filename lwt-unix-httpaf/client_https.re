open Lwt.Infix;
open Httpkit.Client;

type client_security = [
  | `No_authentication
  | `Tls_default
  | `Tls_custom(Tls_lwt.Unix.t)
];

let send:
  (
    ~config: Httpaf.Config.t=?,
    ~client: client_security=?,
    Httpkit.Client.Request.t
  ) =>
  Lwt_result.t(
    (Httpaf.Response.t, Httpaf.Body.t([ | `read])),
    [> | `Connection_error(Httpaf.Client_connection.error)],
  ) =
  (~config=Httpaf.Config.default, ~client as _=`No_authentication, req) => {
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
              switch (Request.body(req)) {
              | None => ()
              | Some(str) => Httpaf.Body.write_string(request_body, str)
              };
              Httpaf.Body.close_writer(request_body);
              response_received >>= (x => x);
            };

            let request = Client_request.of_httpkit_request(req);

            Httpaf_lwt_unix.Client.TLS.request(
              ~config,
              ~error_handler,
              ~response_handler,
              socket,
              request
            )
            |> write_body
          }
        );
      }
    );
  };
