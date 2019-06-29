let of_httpkit_request = req => {
  open Httpkit;
  let host = req |> Request.host;
  let scheme = req |> Request.scheme;
  let meth = req |> Request.meth;
  let path = req |> Request.path;

  let headers =
    [
      (":authority", host),
      (":method", meth |> Method.to_string),
      (":path", path),
      (":scheme", scheme),
    ]
    @ (req |> Request.headers)
    |> H2.Headers.of_list;

  H2.Request.create(~headers, ~scheme, meth, path);
};

let to_httpkit_request = (~body, ~uri, req) => {
  Httpkit.Request.create(
    ~headers=req.H2.Request.headers |> H2.Headers.to_list,
    ~body=
      switch (body) {
      | None => ""
      | Some(b) => b
      },
    req.meth,
    uri,
  );
};
