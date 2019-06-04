module Method = H2.Method;

module Status = H2.Status;

module Client = {
  module Request = {
    type t = {
      meth: Method.t,
      uri: Uri.t,
      body: option(string),
      headers: list((string, string)),
    };

    let body = t => t.body;
    let meth = t => t.meth;
    let uri = t => t.uri;
    let headers = t => t.headers;
    let scheme = t =>
      switch (t |> uri |> Uri.scheme) {
      | None => ""
      | Some(s) => s
      };

    let create = (~headers=[], ~body="", meth, uri) => {
      let host = Uri.host_with_default(uri);
      let content_length = body |> String.length |> string_of_int;
      let headers =
        [("Host", host), ("Content-Length", content_length)] @ headers;

      {
        meth,
        uri,
        headers,
        body:
          switch (body) {
          | "" => None
          | _ => Some(body)
          },
      };
    };
  };
};
