open Lwt.Infix;

let make_request_handler:
  (
    ~uri: Uri.t,
    ~handler: Httpkit.Server.handler,
    ~closer: unit => unit,
    Unix.sockaddr
  ) =>
  Httpaf.Server_connection.request_handler =
  (~uri, ~handler, ~closer, _client, reqd) => {
    let req = reqd |> Httpaf.Reqd.request;
    Logs.debug(m => m("Handling request..."));
    let respond = (~headers=?, status, content) => {
      let headers =
        (
          switch (headers) {
          | None => []
          | Some(hs) => hs
          }
        )
        @ [("Content-Length", content |> String.length |> string_of_int)]
        |> Httpaf.Headers.of_list;
      let res =
        Httpaf.Response.create(status |> Httpaf.Status.of_code, ~headers);
      Httpaf.Reqd.respond_with_string(reqd, res, content);
    };
    Server_request.read_body(reqd)
    >|= (
      body => {
        let uri = Uri.with_path(uri, req.target);
        let req = Client_request.to_httpkit_request(~uri, ~body, req);
        let () = handler(req, respond, closer);
        ();
      }
    )
    |> ignore;
  };

let error_handler = (_client, ~request as _=?, err, _get) => {
  Logs.err(m =>
    m(
      "Something went wrong! %s",
      switch (err) {
      | `Bad_gateway => "Bad gateway"
      | `Bad_request => "Bad request"
      | `Internal_server_error => "Internal_server_error"
      | `Exn(exn) => Printexc.to_string(exn)
      },
    )
  );
  ();
};

let listen:
  (
    ~address: [ | `Loopback | `Any | `Of_string(string)]=?,
    ~port: int,
    ~on_start: (~hoststring: string) => unit,
    ~handler: Httpkit.Server.handler
  ) =>
  Lwt.t(unit) =
  (~address=`Any, ~port, ~on_start, ~handler) => {
    let host =
      switch (address) {
      | `Loopback => "127.0.0.1"
      | `Any => "0.0.0.0"
      | `Of_string(str) => str
      };
    let uri = Uri.make(~scheme="http", ~host, ~port, ());

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
      Httpaf_lwt_unix.Server.create_connection_handler(
        ~config=Httpaf.Config.default,
        ~request_handler=make_request_handler(~uri, ~handler, ~closer),
        ~error_handler,
      );

    Lwt_io.establish_server_with_client_socket(
      listening_address,
      connection_handler,
    )
    >|= (_ => on_start(~hoststring=Uri.to_string(uri)))
    |> ignore;

    forever;
  };
