type t = {
  req: Httpaf.Request.t,
  uri: Uri.t,
  body: option(string),
  headers: list((string, string)),
};

let create:
  (
    ~headers: list((string, string))=?,
    ~body: string=?,
    Httpaf.Method.t,
    Uri.t
  ) =>
  t =
  (~headers=[], ~body="", meth, uri) => {
    let host = Uri.host_with_default(uri);
    let content_length = body |> String.length |> string_of_int;
    let headers =
      [("Host", host), ("Content-Length", content_length)] @ headers;

    {
      req:
        Httpaf.Request.create(
          ~headers=Httpaf.Headers.of_list(headers),
          meth,
          uri |> Uri.to_string,
        ),
      uri,
      headers,
      body:
        switch (body) {
        | "" => None
        | _ => Some(body)
        },
    };
  };

let as_httpaf = req => req.req;
let body = req => req.body;
let headers = req => req.headers;
let uri = req => req.uri;

module type S = {
  type io('a);

  type config;

  let send:
    (~config: config=?, t) =>
    Lwt_result.t(
      (Httpaf.Response.t, Httpaf.Body.t([ | `read])),
      [> | `Connection_error(Httpaf.Client_connection.error)],
    );
};

module Make =
       (M: S)
       : (S with type io('a) = M.io('a) and type config = M.config) => {
  type io('a) = M.io('a);

  type config = M.config;

  let send = M.send;
};
