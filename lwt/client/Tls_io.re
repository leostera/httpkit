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
            Logs.err(m => m("Https.read: Failing..."));
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
