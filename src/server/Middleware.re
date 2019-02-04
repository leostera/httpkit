type ctx('a) = {
  respond:
    (~status: Httpaf.Status.t, ~headers: list((string, string))=?, string) =>
    unit,
  req: Httpaf.Request.t,
  body: unit => option(string),
  closer: unit => unit,
  state: 'a,
};

type t('a, 'b) = ctx('a) => 'b;

type stack('i, 'o) =
  | Init('a): stack('a, 'a)
  | Next(t('b, 'c), stack('a, 'b)): stack('a, 'c);

let rec run:
  type i o. (_, _, Httpaf.Request.t, unit => option(string), stack(i, o)) => o =
  (closer, respond, req, body, stack) =>
    switch (stack) {
    | Init(last) => last
    | Next(f, cont) =>
      f({
        closer,
        respond,
        req,
        body,
        state: run(closer, respond, req, body, cont),
      })
    };
