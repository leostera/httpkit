let send:
  (
    ~meth: Httpaf.Method.t=?,
    ~headers: list((string, string))=?,
    ~body: string=?,
    Uri.t
  ) =>
  Lwt_result.t(
    (Httpaf.Response.t, Httpaf.Body.t([ | `read])),
    [> | `Connection_error(Httpaf.Client_connection.error)],
  );
