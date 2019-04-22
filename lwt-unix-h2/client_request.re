let of_httpkit_request = req => {
  Httpkit.Client.(
    H2.Request.create(
      ~headers=req |> Request.headers |> H2.Headers.of_list,
      ~scheme=req |> Request.scheme,
      req |> Request.meth,
      switch (req |> Request.body) {
      | None => ""
      | Some(body) => body
      },
    )
  );
};
