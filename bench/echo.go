package main

import (
	"log"
	"net/http"
  "bytes"
)

const DefaultPort = "8080"

func EchoHandler(writer http.ResponseWriter, request *http.Request) {
  stamp := request.Method + " " + request.URL.Path
	log.Println(stamp)
  buf := bytes.NewBufferString(request.URL.Path)
	request.Write(buf)
}

func main() {
	log.Println("Listening on port " + DefaultPort)
	http.HandleFunc("/", EchoHandler)
	http.ListenAndServe(":"+DefaultPort, nil)
}
