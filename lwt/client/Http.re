module M: Httpkit.Client.Request.S with type io('a) = Lwt.t('a) =
  Httpkit.Client.Request.Make({
    type io('a) = Lwt.t('a);
    let send = (~trace as _=?, ~meth=`GET, ~headers=[], ~body=?, uri) => {
      open Httpaf;
      open Lwt.Infix;
      let response_handler =
          (notify_response_received, response, response_body) =>
        Lwt.wakeup_later(
          notify_response_received,
          (response, response_body) |> Lwt_result.return,
        );

      let error_handler = (notify_response_received, error) =>
        Lwt.wakeup_later(
          notify_response_received,
          `Connection_error(error) |> Lwt_result.fail,
        );

      let host = Uri.host_with_default(uri);
      let port =
        switch (Uri.port(uri)) {
        | Some(port) => port |> string_of_int
        | None => "80"
        };
      Lwt_unix.getaddrinfo(host, port, [Unix.(AI_FAMILY(PF_INET))])
      >>= (
        addresses => {
          let socket_addr = List.hd(addresses).Unix.ai_addr;
          let socket = Lwt_unix.socket(Unix.PF_INET, Unix.SOCK_STREAM, 0);

          Lwt_unix.connect(socket, socket_addr)
          >>= (
            () => {
              let content_length =
                switch (body) {
                | None => "0"
                | Some(body) => body |> String.length |> string_of_int
                };

              let headers =
                Headers.of_list(
                  [("Host", host), ("Content-Length", content_length)]
                  @ headers,
                );
              let path = uri |> Uri.path_and_query;
              let request = Request.create(meth, path, ~headers);

              let (response_received, notify_response_received) = Lwt.wait();
              let response_handler =
                response_handler(notify_response_received);
              let error_handler = error_handler(notify_response_received);

              let request_body =
                Httpaf_lwt.Client.request(
                  socket,
                  request,
                  ~error_handler,
                  ~response_handler,
                );

              switch (body) {
              | Some(body) => Body.write_string(request_body, body)
              | None => ()
              };
              Body.flush(request_body, () => Body.close_writer(request_body));

              /* TODO(@ostera): Better idiom for this? */
              response_received >>= (result => result);
            }
          );
        }
      );
    };
  });

include M;
