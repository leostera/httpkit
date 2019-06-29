let of_httpkit_request = req => {
  Httpkit.(
    Httpaf.Request.create(
      ~headers=req |> Request.headers |> Httpaf.Headers.of_list,
      req |> Request.meth,
      req |> Request.path,
    )
  );
};

let to_httpkit_request = (~body, ~uri, req) => {
  Httpkit.Request.create(
    ~headers=req.Httpaf.Request.headers |> Httpaf.Headers.to_list,
    ~body=
      switch (body) {
      | None => ""
      | Some(b) => b
      },
    req.meth,
    uri,
  );
};
