open Lwt.Infix;

/**
  This module was originally written during one of the [ReasonableCoding]
  live-streams for the Twitchboard project.

  Special thanks to @anmonteiro for guiding me through the whole process.

  Loosely based on examples from httpaf-lwt, ocaml-tls, and this gist from
  @anmonteiro: https://gist.github.com/anmonteiro/794d2713e787690ef16c684360a4d39f
  */

type tls_config = {
  tracer: option(Tls_lwt.tracer),
  tls_client:
    (Httpkit.Client.Request.t, Lwt_unix.file_descr) => Lwt.t(Tls_lwt.Unix.t),
};

type tls_auth = [
  | `Ca_dir(string)
  | `Ca_file(string)
  | `Hex_key_fingerprints(Nocrypto.Hash.hash, list((string, string)))
  | `Key_fingerprints(Nocrypto.Hash.hash, list((string, Cstruct.t)))
  | `No_authentication_I'M_STUPID
];

module Config = {
  let from_pems = (~tracer=?, ~cert, ~priv_key, ()) => {
    {
      tracer,
      tls_client: (req, socket) => {
        let host = Httpkit.Client.Request.uri(req) |> Uri.host_with_default;
        X509_lwt.authenticator(`Ca_file(cert |> Fpath.to_string))
        >>= (
          authenticator =>
            X509_lwt.private_of_pems(
              ~cert=cert |> Fpath.to_string,
              ~priv_key=priv_key |> Fpath.to_string,
            )
            >>= (
              certificate => {
                let client =
                  Tls.Config.client(
                    ~authenticator,
                    ~certificates=`Single(certificate),
                    (),
                  );
                switch (tracer) {
                | Some(tracer) =>
                  Tls_lwt.Unix.client_of_fd(
                    ~trace=tracer,
                    client,
                    ~host,
                    socket,
                  )
                | _ => Tls_lwt.Unix.client_of_fd(client, ~host, socket)
                };
              }
            )
        );
      },
    };
  };
  let no_auth = (~tracer=?, ()) => {
    {
      tracer,
      tls_client: (req, socket) => {
        let host = Httpkit.Client.Request.uri(req) |> Uri.host_with_default;
        X509_lwt.authenticator(`No_authentication_I'M_STUPID)
        >>= (
          authenticator => {
            let client = Tls.Config.client(~authenticator, ());
            switch (tracer) {
            | Some(tracer) =>
              Tls_lwt.Unix.client_of_fd(~trace=tracer, client, ~host, socket)
            | _ => Tls_lwt.Unix.client_of_fd(client, ~host, socket)
            };
          }
        );
      },
    };
  };
};

module M:
  Httpkit.Client.Request.S with
    type io('a) = Lwt.t('a) and type config = tls_config =
  Httpkit.Client.Request.Make({
    type io('a) = Lwt.t('a);

    type config = tls_config;

    let send = (~config=?, req) => {
      open Httpkit.Client;

      let body = Request.body(req);
      let uri = Request.uri(req);

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
      Lwt_unix.getaddrinfo(host, "443", [Unix.(AI_FAMILY(PF_INET))])
      >>= (
        addresses => {
          let socket_addr = List.hd(addresses).Unix.ai_addr;
          let socket = Lwt_unix.socket(Unix.PF_INET, Unix.SOCK_STREAM, 0);

          Lwt_unix.connect(socket, socket_addr)
          >>= (
            _ => {
              switch (config) {
              | Some({tls_client, _}) => tls_client(req, socket)
              | None => Config.no_auth().tls_client(req, socket)
              };
            }
          )
          >>= (
            tls_client => {
              let request = Request.as_httpaf(req);

              let (response_received, notify_response_received) = Lwt.wait();
              let response_handler =
                response_handler(notify_response_received);
              let error_handler = error_handler(notify_response_received);

              let request_body =
                Httpaf_lwt.Client.request(
                  ~writev=Tls_io.writev(tls_client),
                  ~read=Tls_io.read(tls_client),
                  socket,
                  request,
                  ~error_handler,
                  ~response_handler,
                );

              switch (body) {
              | Some(body) => Httpaf.Body.write_string(request_body, body)
              | None => ()
              };
              Httpaf.Body.flush(request_body, () =>
                Httpaf.Body.close_writer(request_body)
              );

              /* TODO(@ostera): Better idiom for this? */
              response_received >>= (result => result);
            }
          );
        }
      );
    };
  });

include M;
