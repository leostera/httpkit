type ctx('a) = {
  respond:
    (~status: Httpaf.Status.t, ~headers: list((string, string))=?, string) =>
    unit,
  req: Httpaf.Request.t,
  state: 'a,
};

type t('a, 'b) = ctx('a) => 'b;

type stack('i, 'o) =
  | Init('a): stack('a, 'a)
  | Next(t('b, 'c), stack('a, 'b)): stack('a, 'c);

let rec run: type i o. (_, Httpaf.Request.t, stack(i, o)) => o =
  (respond, req, stack) =>
    switch (stack) {
    | Init(last) => last
    | Next(f, cont) => f({respond, req, state: run(respond, req, cont)})
    };
