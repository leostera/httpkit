module Model: {
  type ctx('a) = {
    respond:
      (
        ~status: Httpaf.Status.t,
        ~headers: list((string, string))=?,
        string
      ) =>
      unit,
    req: Httpaf.Request.t,
    body: unit => option(string),
    closer: unit => unit,
    state: 'a,
  };

  type middleware('a, 'b) = ctx('a) => 'b;
};

module Chain: {
  type t('i, 'o) =
    | Init('a): t('a, 'a)
    | Next(Model.middleware('b, 'c), t('a, 'b)): t('a, 'c);

  let run:
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
      t('i, 'o)
    ) =>
    'o;
};

module Common: {
  let log: Model.middleware('a, 'a);

  type headers = list((string, string));
  type path = list(string);

  type route_handler('a) =
    (Model.ctx('a), path) =>
    [
      | `OK(string)
      | `With_headers(Httpaf.Status.t, headers, string)
      | `With_status(Httpaf.Status.t, string)
      | `Unmatched
    ];

  let router:
    route_handler('a) =>
    Model.middleware('a, [ | `Replied(Httpaf.Status.t, headers, string)]);
};
