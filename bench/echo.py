from BaseHTTPServer import BaseHTTPRequestHandler,HTTPServer

PORT_NUMBER = 8080

class handler(BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.send_header('Content-Length',len(self.path))
        self.end_headers()
        self.wfile.write(self.path)
        return

server = HTTPServer(('', PORT_NUMBER), handler)
print 'Listening on port ' , PORT_NUMBER
server.serve_forever()
