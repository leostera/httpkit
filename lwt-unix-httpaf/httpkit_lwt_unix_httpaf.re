module Method = Httpkit.Method;

module Status = Httpkit.Status;

module Request = Httpkit.Request;

module Server = {
  include Httpkit.Server;
  module Http = Server_http;
};

module Client = {
  module Https = Client_https;
  module Http = Client_http;
  module Response = Client_response;
};
