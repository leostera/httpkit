type t('i, 'o) =
  | Init('a): t('a, 'a)
  | Next(Model.middleware('b, 'c), t('a, 'b)): t('a, 'c);

let rec run:
  type i o.
    (
      ~closer: unit => unit,
      ~respond: (
                  ~status: Httpaf.Status.t,
                  ~headers: list((string, string))=?,
                  string
                ) =>
                unit,
      ~request: Httpaf.Request.t,
      ~body: unit => option(string),
      t(i, o)
    ) =>
    o =
  (~closer, ~respond, ~request, ~body, chain) =>
    switch (chain) {
    | Init(last) => last
    | Next(f, cont) =>
      f({
        closer,
        respond,
        req: request,
        body,
        state: run(~closer, ~respond, ~request, ~body, cont),
      })
    };
