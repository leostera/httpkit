type ctx('a) = {
  respond:
    (~status: Httpaf.Status.t, ~headers: list((string, string))=?, string) =>
    unit,
  req: Httpaf.Request.t,
  body: unit => option(string),
  closer: unit => unit,
  state: 'a,
};

type middleware('a, 'b) = ctx('a) => 'b;
