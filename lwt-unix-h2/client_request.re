let of_httpkit_request = req => {
  open Httpkit;
  let host = req |> Request.host;
  let headers = [(":authority", host)] @ (req |> Request.headers);
  H2.Request.create(
    ~headers=headers |> H2.Headers.of_list,
    ~scheme=req |> Request.scheme,
    req |> Request.meth,
    req |> Request.path,
  );
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
