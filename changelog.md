# Changelog

## 6.4.6

* Additional small fixes to include path and versioning.

## 6.4.5

* Fix two `size_t` conversion warnings.

## 6.4.4

* Revamp `makefile` process to only link when necessary, and clean up Release vs. Debug flags.
* Setup and test Visual Studio Code pipeline for debugging tests.

## 6.4.3

* Fix includes in `networking/http/client.hpp`.
* Start migrating over to `RequestResponse::Socket::send` functions introduced in `6.4.2`.

## 6.4.2

* Redo accessibility and ambiguity resolution in `Socket` inheritance.
* Add `send` timeouts to `RequestResponse::Server`, partially implemented.
* `RECV_TIMEOUT_MS` default tuned down to `5000`.
* Break down files in `Networking` to encompass one class each.

## 6.4.1

* Increase default `RECV_TIMEOUT_MS` to `60000` from `1000`.

## 6.4.0

* Don't sort `using` in `.clang-format`.
* Fix `make noinc`.
* Define `NOMINMAX` where it matters.
* Restructure `Networking` inheritance with fewer templates, around `RequestResponse` `Server` and `Socket`.

## 6.3.0

* Make `Socket` RAII by implementing move constructors.
* Removed Unicode specification in VS test projects.
* Added custom error/exception handling to `Rain`.
* Fix builds on `MacOS`.
* Rework inheritance model of `Socket` subclasses.
* Make `socket-server` test much more comprehensive.

## 6.2.1

* Switch to WSL from Cygwin.

## 6.2.0

* Make `Rain::Socket` weakly RAII.

## 6.1.5

* Fix the `http-server` test with new `DataType` server templates.

## 6.1.4

* Bugfix response parsing in `Smtp::Client`.
* Shorten default `acceptTimeoutMs` to `5` seconds.

## 6.1.3

* Fix bug with DNS parsing in `Smtp::Client` with regards to using `len`.

## 6.1.2

* Fix small bug with `Smtp::Client` DNS parsing.

## 6.1.1

* Complete very basic `Smtp::Client` and `Smtp::Server` implementations.
* Update `SocketSlave` implementation to include custom templated `DataType`.

## 6.1.0

* Add boilerplate for `Smtp` libraries.

## 6.0.23

* Add `-lstdc++fs` flag to compilation options.
* Fix `http-server` test.

## 6.0.22

* Fix some bugs in `Networking` and `Http`.
* Add `Filesystem` utility function `subpath`.

## 6.0.21

* Add `LRUCache` implementation as part of `Rain::Algorithm`.
* Add timeout options for `CustomServer`.

## 6.0.20

`Server` now has a timeout on `recv` since `select` is not guaranteed to break when the socket is closed.

## 6.0.19

Revert `ThreadPool` to throw error when threads cannot be created.

Make `Rain::strcpy_s` use `strncpy` as a backend, with `_TRUNCATE` on Windows.

## 6.0.18

`Socket::accept` now blocks on `select` with an optimal timeout. `CustomServer` uses a 1-minute `accept` blocking timeout to avoid being blocked indefinitely on `Socket::accept`, and exiting eventually when the server is closed.

## 6.0.17

Modify `rain` networking libraries to report errors via exceptions instead of return `int`s. `send` and `recv` now block on `select` to terminate correctly on exceptional conditions (closing the socket from another thread).

## 6.0.16

Fix `std::map` problem by fixing the lowercase comparator. Begin work on revamping `Socket` to crash less often and throw errors correctly.

## 6.0.15

Abstract the `Body` and `Header` interface in `Http::Server`.

Todo: There is some bug in iterating through `std::map` when sending a `Response`.

## 6.0.14

Working inheritance for `Http::Server`. Nice!

Next: refactor `Http` headers.

## 6.0.13

Refactor `Networking` namespace except for `Http`. Create standalone inheritable `CustomServer` and `CustomServerSlave` with non-templated `Server` and `ServerSlave`.

Todo: `Http` namespace library.

## 6.0.12

Add `std::error_code` handling to `ThreadPool::queueTask`, which speeds up code significantly if surpassing system thread limits. Move `ThreadPool` to `Rain` namespace.

Restructure `CommandLineParser` and `WaterfallParser`, and add `std::vector` parsing.

## 6.0.11

Refactor organization in `Algorithm`, `String`, and `Thread`. Remove erroneous `ConditionVariable` implementation. Improve `ThreadPool` implementation to handle common exception upon creating thread in a resource-limited environment. Use `std::condition_variable` correctly in `ThreadPool`. Improve `String` namespace with `anyToAny` and efficient whitespace methods.

## 6.0.10

Remove erroneous comments from makefile.

## 6.0.9

Modify makefile to increment the build only when rain headers change.

## 6.0.8

Refactor files into folders based on namespace.

## 6.0.7

Implemented sending of `HttpResponse` and `HttpRequest`.

## 6.0.6

`CustomHttpServer` now parses headers and calls `onRequest` as designed.

## 6.0.5

* Have `ThreadPool` use `std::function` to allow lambdas with captures to be passed as `Task`s.
* Create skeleton implementation for `HttpPayload`, `HttpRequest`, `HttpResponse`, `HttpSocket`, `HttpClient`.
* Implement `CustomHttpServerSlave` and `CustomHttpServer` to allow for subclassing.
* Create reference `HttpServerSlave` and `HttpServer` implementations which don't process requests just yet.

## 6.0.4

* Create `BufferPool` implementation to decrease impact of freeing and allocating memory on the heap.
* Begin `ThreadPool` implementation.
* Implemented KMP string matching.
* Created platform-specific types for `Socket`.

## 6.0.3

Convert `rain` to header-only. Use `inline static` member variables and require at least `C++17` compilation.

## 6.0.2

Add basic cross-platform `Socket` implementation.

## 6.0.1

Add `CommandLineParser` and `WaterfallParser` and some `RAIN_WINDOWS`-based compatibility for `rsize_t` and `strcpy_s`.

## 6.0.0

Setup for cross-platform builds.
