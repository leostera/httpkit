let writev = (tls_client, _fd, io_vecs) =>
  Lwt.(
    catch(
      () => {
        let {Faraday.len, buffer, off} = io_vecs |> List.hd;
        Cstruct.of_bigarray(~len, ~off, buffer)
        |> Tls_lwt.Unix.write(tls_client)
        >|= (_ => `Ok(len));
      },
      exn =>
        switch (exn) {
        | Unix.Unix_error(Unix.EBADF, "check_descriptor", _) =>
          Lwt.return(`Closed)
        | exn =>
          Logs_lwt.err(m => m("failed: %s", Printexc.to_string(exn)))
          >>= (() => Lwt.fail(exn))
        },
    )
  );

let read = (tls_client, fd, buffer) =>
  Lwt.(
    catch(
      () =>
        Httpaf_lwt.Buffer.put(buffer, ~f=(bigstring, ~off, ~len) =>
          Tls_lwt.Unix.read_bytes(tls_client, bigstring, off, len)
        ),
      exn => {
        let err = Printexc.to_string(exn);
        (
          switch (Lwt_unix.state(fd)) {
          | Lwt_unix.Closed =>
            Logs_lwt.err(m => m("Https.read: Socket closed"))
          | Aborted(exn) =>
            Logs_lwt.err(m =>
              m("Https.read: Socket aborted: %s", Printexc.to_string(exn))
            )
          | Opened =>
            Logs_lwt.err(m => m("Https.read: Socket opened! %s", err))
          }
        )
        >>= (
          _ => {
            Lwt.async(() => Tls_lwt.Unix.close(tls_client));
            Lwt.fail(exn);
          }
        );
      },
    )
    >|= (
      bytes_read =>
        if (bytes_read == 0) {
          `Eof;
        } else {
          `Ok(bytes_read);
        }
    )
  );

let send = (~meth=`GET, ~headers=[], ~body=?, uri) => {
  open Httpaf;
  open Lwt.Infix;
  let response_handler = (notify_response_received, response, response_body) =>
    Lwt.wakeup_later(
      notify_response_received,
      Ok((response, response_body)),
    );

  let error_handler = (notify_response_received, error) =>
    Lwt.wakeup_later(notify_response_received, Error(error));

  let host = Uri.host_with_default(uri);
  Lwt_unix.getaddrinfo(host, "443", [Unix.(AI_FAMILY(PF_INET))])
  >>= (
    addresses => {
      let socket_addr = List.hd(addresses).Unix.ai_addr;
      let socket = Lwt_unix.socket(Unix.PF_INET, Unix.SOCK_STREAM, 0);

      Lwt_unix.connect(socket, socket_addr)
      >>= (_ => X509_lwt.authenticator(`No_authentication_I'M_STUPID))
      >>= (
        authenticator => {
          let client =
            Tls.Config.client(
              ~authenticator,
              ~alpn_protocols=["http/1.1", "h2"],
              (),
            );
          Tls_lwt.Unix.client_of_fd(client, ~host, socket);
        }
      )
      >>= (
        tls_client => {
          let content_length =
            switch (body) {
            | None => "0"
            | Some(body) => string_of_int(String.length(body))
            };

          let request_headers =
            Request.create(
              meth,
              Uri.path_and_query(uri),
              ~headers=
                Headers.of_list(
                  [("Host", host), ("Content-Length", content_length)]
                  @ headers,
                ),
            );

          let (response_received, notify_response_received) = Lwt.wait();
          let response_handler = response_handler(notify_response_received);
          let error_handler = error_handler(notify_response_received);

          let request_body =
            Httpaf_lwt.Client.request(
              ~writev=writev(tls_client),
              ~read=read(tls_client),
              socket,
              request_headers,
              ~error_handler,
              ~response_handler,
            );

          switch (body) {
          | Some(body) => Body.write_string(request_body, body)
          | None => ()
          };
          Body.flush(request_body, () => Body.close_writer(request_body));
          response_received;
        }
      );
    }
  );
};
