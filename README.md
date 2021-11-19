![build](https://github.com/busshi/webserv/actions/workflows/compil.yml/badge.svg)

[![aldubar's 42Project Score](https://badge42.herokuapp.com/api/project/aldubar/webserv)](https://github.com/JaeSeoKim/badge42)

# Webserv

A tiny HTTP server implementation built with C++, for learning purpose.

More details about the project can be found in the [wiki](https://github.com/busshi/webserv/wiki).

## Build

### Prerequisites

- A C++ compiler, preferably clang++ (g++ should work too though)
- A UNIX-based operating system: the web server has been mainly built to work on Linux but should also work on MacOS.
- `make`

### Compile the server

A simple `make` command should make the `webserv` executable. This executable is the server itself and can then
be started by providing it a configuration file like so:

```sh
./webserv ./asset/config/simple.conf
```
More information about the configuration file format and available configuration options can be found on the corresponding
[wiki page](https://github.com/busshi/webserv/wiki/Configuration-File) .

## Generate documentation (doxygen)

Our webserv's code is highly documented and HTML-based documentation can be generated using `doxygen`.
For that, special Makefile rules are provided: you just need to have the doxygen executable available on your system.

| Rule | Effect |
|------|--------|
| doc  |  generate documentation if it hasn't been generated already, and then open the index.html file in a new tab in the default browser.  |
| cleandoc | Basically removes the documentation directory, allowing any subsequent doc command to regenerate it.
| redoc | cleandoc + doc

**IMPORTANT NOTE**: documentation WON'T be regenerated on source file change, you need to remake it to update it as needed. 