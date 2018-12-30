require 'rack'
 
app = Proc.new do |env|
  req = Rack::Request.new(env)
  meth = req.request_method
  path = req.path
  now = Time.now
  puts "#{now} - #{meth} #{path}"
  ['200', {'Content-Type' => 'text/html'}, [path]]
end
 
Rack::Handler::WEBrick.run app
