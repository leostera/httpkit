let log: Middleware.t('a, 'a) =
  ctx => {
    let {Unix.tm_hour, tm_min, tm_sec, _} = Unix.time() |> Unix.localtime;
    let time = Printf.sprintf("%d:%d:%d", tm_hour, tm_min, tm_sec);

    let meth = ctx.req.meth |> Httpaf.Method.to_string;
    let path = ctx.req.target;

    Logs.info(m => m("%s — %s %s", time, meth, path));

    ctx.state;
  };

module BodyParser = {
  /** TODO: properly parse the body string as JSON
   * - as Yojson
   * - accepting a decoder function
   */
  let json: Middleware.t('a, option(string)) =
    ctx => {
      ctx.body_string;
    };
};

type headers = list((string, string));
type path = list(string);

type route_handler('a) =
  (Middleware.ctx('a), path) =>
  [
    | `OK(string)
    | `With_headers(Httpaf.Status.t, headers, string)
    | `With_status(Httpaf.Status.t, string)
    | `Unmatched
  ];

let router:
  route_handler('a) =>
  Middleware.t('a, [ | `Replied(Httpaf.Status.t, headers, string)]) =
  (handler, ctx) => {
    let path = ctx.req.target |> String.split_on_char('/') |> List.tl;
    let (status, headers, response) =
      switch (handler(ctx, path)) {
      | `OK(response) => (`OK, [], response)
      | `With_status(status, response) => (status, [], response)
      | `With_headers(status, headers, response) => (
          status,
          headers,
          response,
        )
      | `Unmatched => (`Not_found, [], "")
      };
    ctx.respond(~status, ~headers, response);
    `Replied((status, headers, response));
  };
