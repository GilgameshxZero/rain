# Changelog

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
