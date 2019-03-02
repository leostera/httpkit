/** Handle sigpipe internally */
Sys.(set_signal(sigpipe, Signal_ignore));

/** Setup loggers */
Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.Debug));
Logs.set_reporter(Logs_fmt.reporter());

module App = {
  let on_start = () => Logs.app(m => m("Running on localhost:9999"));
  type state = {random: string};
  let initial_state = {random: Random.float(0.1) |> string_of_float};
  let route_handler: Httpkit.Middleware.Common.route_handler(state) =
    (ctx, path) =>
      switch (path) {
      | [""] => `OK("hello world #" ++ ctx.state.random)
      | ["err"] => `With_status((`Unauthorized, "Yikes! Login first."))
      | ["with", "code", code] =>
        `With_status((code |> Httpaf.Status.of_string, ""))
      | _ => `Unmatched
      };
};

module Common = Httpkit.Middleware.Common;
Httpkit.(
  Server.(
    make(App.initial_state)
    |> use(Common.log)
    |> reply(Common.router(App.route_handler))
    |> Httpkit_lwt.Server.Http.listen(~port=9999, ~on_start=App.on_start)
    |> Lwt_main.run
  )
);
