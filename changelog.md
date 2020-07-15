# Changelog

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
