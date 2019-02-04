let read_body: Httpaf.Reqd.t(Lwt_unix.file_descr) => Lwt.t(option(string)) =
  reqd => {
    let req = reqd |> Httpaf.Reqd.request;
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
