.PHONY: deps
deps:
	opam install --deps-only .

.PHONY: install
install:
	dune install

.PHONY: build
build:
	dune build

.PHONY: watch
watch:
	dune build --watch

.PHONY: fmt
fmt:
	dune build @fmt --auto-promote

.PHONY : clean
clean :
	dune clean
