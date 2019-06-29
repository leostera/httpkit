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
