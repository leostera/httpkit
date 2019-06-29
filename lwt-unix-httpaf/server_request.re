let read_body = reqd => {
  let (next, awake) = Lwt.wait();

  Lwt.async(() => {
    let body = reqd |> Httpaf.Reqd.request_body;
    let body_str = ref("");
    let on_eof = () => Lwt.wakeup_later(awake, Some(body_str^));
    let rec on_read = (request_data, ~off, ~len) => {
      let read = Bigstringaf.substring(~off, ~len, request_data);
      body_str := body_str^ ++ read;
      Httpaf.Body.schedule_read(body, ~on_read, ~on_eof);
    };
    Httpaf.Body.schedule_read(body, ~on_read, ~on_eof);
    Lwt.return_unit;
  });

  next;
};
