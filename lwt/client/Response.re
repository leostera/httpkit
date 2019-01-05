open Lwt.Infix;

module M: Httpkit.Client.Response.S with type io('a) = Lwt.t('a) =
  Httpkit.Client.Response.Make({
    type io('a) = Lwt.t('a);
    let body = ((response, body)) => {
      let buffer = Buffer.create(1024);
      let (next, wakeup) = Lwt.wait();
      switch (Httpaf.Response.(response.status)) {
      | `OK =>
        let rec read_response = () =>
          Httpaf.Body.schedule_read(
            body,
            ~on_eof=
              () => Lwt.wakeup_later(wakeup, Ok(Buffer.contents(buffer))),
            ~on_read=
              (response_fragment, ~off, ~len) => {
                let response_fragment_string = Bytes.create(len);
                Lwt_bytes.blit_to_bytes(
                  response_fragment,
                  off,
                  response_fragment_string,
                  0,
                  len,
                );
                Buffer.add_bytes(buffer, response_fragment_string);
                read_response();
              },
          );
        read_response();
      | _ =>
        Logs.err(m =>
          m("Https.Response.read_body: %s", "Something went wrong")
        );
        Lwt.wakeup_later(wakeup, Error(`Reading_error));
      };
      next >>= Lwt_result.lift;
    };
  });

include M;
