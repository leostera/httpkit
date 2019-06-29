let of_httpkit_request = req => {
  Httpkit.Client.(
    Httpaf.Request.create(
      ~headers=req |> Request.headers |> Httpaf.Headers.of_list,
      req |> Request.meth,
      req |> Request.path,
    )
  );
};
