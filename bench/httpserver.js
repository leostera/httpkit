const httpServer = require('http');

httpServer.createServer((req, res) => {
  console.log(new Date(), req.method, req.url);
  res.end(req.url);
})
.listen(8080, () => console.log("Server started..."));;
