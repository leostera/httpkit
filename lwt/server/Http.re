open Lwt.Infix;
open Httpkit_server;

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
    Request.read_body(reqd)
    >|= (
      body_string => {
        server
        |> Server.middleware
        |> Server.Middleware.run(closer, respond, req, () => body_string)
        |> ignore;
      }
    )
    |> ignore;
  };

let error_handler: error_handler =
  (_client, ~request as _=?, _err, _get) => ();

let start:
  (
    ~address: [ | `Loopback | `Any | `Of_string(string)],
    ~port: int,
    ~on_start: unit => unit,
    ~request_handler: request_handler,
    ~error_handler: error_handler
  ) =>
  Lwt.t(unit) =
  (~address, ~port, ~on_start, ~request_handler, ~error_handler) => {
    let (forever, awaker) = Lwt.wait();
    let closer = () => Lwt.wakeup_later(awaker, ());

    let address =
      switch (address) {
      | `Loopback => Unix.inet_addr_loopback
      | `Any => Unix.inet_addr_any
      | `Of_string(str) => Unix.inet_addr_of_string(str)
      };
    let listening_address = Unix.(ADDR_INET(address, port));

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
    ~address: [ | `Loopback | `Any | `Of_string(string)]=?,
    ~port: int,
    ~on_start: unit => unit,
    Httpkit_server.Server.t([ | `With_middleware], [ | `Responded], 'a, 'b)
  ) =>
  Lwt.t(unit) =
  (~address=`Any, ~port, ~on_start, server) =>
    start(
      ~address,
      ~port,
      ~on_start,
      ~request_handler=make_request_handler(server),
      ~error_handler,
    );
