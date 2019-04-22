module Make = (M: BASE) : MIDDLEWARE => {
  type ctx('a) = {
    respond:
      (~status: M.status, ~headers: list((string, string))=?, string) => unit,
    req: M.request,
    body: unit => option(string),
    closer: unit => unit,
    state: 'a,
  };

  type middleware('a, 'b) = ctx('a) => 'b;

  type chain('i, 'o) =
    | Init('a): chain('a, 'a)
    | Next(middleware('b, 'c), chain('a, 'b)): chain('a, 'c);

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
        chain(i, o)
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
};
