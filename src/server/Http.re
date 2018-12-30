module type Handlers = {
  /** [request_handler(~closer, client, reqd)] will be called whenever a new
      client connection is initiated and a request is available for consumption.

      [~closer] is a function that can be used to close shut down the server.
      [client] is the socket address of the clients socket.
      [reqd] is a request descriptor that can be used to inspect the request.
      */
  let request_handler:
    (~closer: unit => unit, Unix.sockaddr, Httpaf.Reqd.t('b)) => unit;

  /** [error_handler(client, ~request, error, start_response)] will be called
      whenever a client request has an error that prevents it to be handled at
      all.

      [client] is the socket address of the clients socket.
      [~request] is an optional value describing the request.
      [error] will be the error that occurred.
      [start_response] is a function to access a writable body to return a final
      error response to the client.
      */
  let error_handler:
    (
      Unix.sockaddr,
      ~request: Httpaf.Request.t=?,
      Httpaf.Server_connection.error,
      Httpaf.Headers.t => Httpaf.Body.t([ | `write])
    ) =>
    unit;
};

module type S = {
  /** The type of a server module */

  /** [start(port, on_start)] will start a server on port [port] and execute
      [on_start] when it's finished starting. */
  let start: (~port: int, ~on_start: unit => unit) => Lwt.t(unit);
};

/**
  Functor to make server modules.
  */
let start = (~port, ~on_start, ~request_handler, ~error_handler) => {
  open Lwt.Infix;

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
  >>= (
    _ => {
      on_start();
      Lwt.return_unit;
    }
  )
  |> ignore;

  forever;
};
