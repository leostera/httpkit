open Lwt.Infix;
open Httpkit;

type request_handler =
  (~closer: unit => unit, Unix.sockaddr) => Httpaf_lwt.Server.request_handler;

type error_handler =
  (
    Unix.sockaddr,
    ~request: Httpaf.Request.t=?,
    Httpaf.Server_connection.error,
    Httpaf.Headers.t => Httpaf.Body.t([ | `write])
  ) =>
  unit;

let make_request_handler: Server.t('s, 'r, 'a, 'b) => request_handler =
  (server, ~closer, _client, reqd) => {
    let req = reqd |> Httpaf.Reqd.request;
    let read_body = () => {
      switch (req.meth) {
      | `POST
      | `PUT =>
        let (next, awake) = Lwt.wait();

        Lwt.async(() => {
          let body = reqd |> Httpaf.Reqd.request_body;
          let body_str = ref("");
          let on_eof = () => Lwt.wakeup_later(awake, Some(body_str^));
          let rec on_read = (request_data, ~off, ~len) => {
            let read = Httpaf.Bigstring.to_string(~off, ~len, request_data);
            body_str := body_str^ ++ read;
            Httpaf.Body.schedule_read(body, ~on_read, ~on_eof);
          };
          Httpaf.Body.schedule_read(body, ~on_read, ~on_eof);
          Lwt.return_unit;
        });

        next;
      | _ => Lwt.return_none
      };
    };
    let respond = (~status, ~headers=?, content) => {
      let headers =
        (
          switch (headers) {
          | None => []
          | Some(hs) => hs
          }
        )
        @ [("Content-Length", content |> String.length |> string_of_int)]
        |> Httpaf.Headers.of_list;
      let res = Httpaf.Response.create(status, ~headers);
      Httpaf.Reqd.respond_with_string(reqd, res, content);
    };
    read_body()
    >|= (
      body_string => {
        server
        |> Server.middleware
        |> Server.Middleware.run(closer, respond, req, body_string)
        |> ignore;
      }
    )
    |> ignore;
  };

let error_handler: error_handler =
  (_client, ~request as _=?, _err, _get) => ();

let start:
  (
    ~port: int,
    ~on_start: unit => unit,
    ~request_handler: request_handler,
    ~error_handler: error_handler
  ) =>
  Lwt.t(unit) =
  (~port, ~on_start, ~request_handler, ~error_handler) => {
    let (forever, awaker) = Lwt.wait();
    let closer = () => Lwt.wakeup_later(awaker, ());

    let listening_address = Unix.(ADDR_INET(inet_addr_loopback, port));

    let connection_handler =
      Httpaf_lwt.Server.create_connection_handler(
        ~request_handler=request_handler(~closer),
        ~error_handler,
      );

    Lwt_io.establish_server_with_client_socket(
      listening_address,
      connection_handler,
    )
    >|= (_ => on_start())
    |> ignore;

    forever;
  };

let listen:
  (
    ~port: int,
    ~on_start: unit => unit,
    ~key: string,
    ~cert: string,
    Httpkit_server.Server.t([ | `With_middleware], [ | `Responded], 'a, 'b)
  ) =>
  Lwt.t(unit) =
  (~port, ~on_start, ~key as _, ~cert as _, server) =>
    start(
      ~port,
      ~on_start,
      ~request_handler=make_request_handler(server),
      ~error_handler,
    );