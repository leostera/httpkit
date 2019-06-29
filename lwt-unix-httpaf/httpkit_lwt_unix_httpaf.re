module Method = Httpkit.Method;

module Status = Httpkit.Status;

module Client = {
  include Httpkit.Client;
  module Https = Client_https;
  module Http = Client_http;
  module Response = Client_response;
};
