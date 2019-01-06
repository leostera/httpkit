module type S = {
  type io('a);

  let send:
    (
      ~trace: Tls_lwt.tracer=?,
      ~meth: Httpaf.Method.t=?,
      ~headers: list((string, string))=?,
      ~body: string=?,
      Uri.t
    ) =>
    Lwt_result.t(
      (Httpaf.Response.t, Httpaf.Body.t([ | `read])),
      [> | `Connection_error(Httpaf.Client_connection.error)],
    );
};

module Make = (M: S) : (S with type io('a) = M.io('a)) => {
  type io('a) = M.io('a);

  let send = M.send;
};
