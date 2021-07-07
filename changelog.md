# Changelog

## 6.6.0

* Rename header filenames to singular instead of plural.
* Add `assert` checks to non-networking tests.
* Add `_zu` `std::size_t` literal.
* Default configuration on Windows VSCode changed to `integratedTerminal`.
* Added many clarifying comments for tests and non-networking headers.
* `LruCache` renamed from `LRUCache` to preserve style with acronyms.
* `cStrSearchKmp` renamed from `cStrSearchKMP` to preserve style with acronyms.
* LRU cache carries exceptions from superclass.
* Modify all non-networking libraries to use `Rain::ErrorException::Exception` for throwing errors.
* Add platform-dependent `std::size_t` parser to `Rain::String::WaterfallParser`.

## 6.5.4

* Order MX servers alphabetically, in the temporary absence of ordering them via MX priority.

## 6.5.3

* Continue migration to using `TYPE const` specification syntax.
* Change `Host` internals for `localhost` configuration.
* Add comments to tests.

## 6.5.2

* Document and update usage, build, and development procedures in `readme.md`.
* Remove `-march=native` build flag in `build/makefile`.
* Add `-DNDEBUG` build flag in `build/makefile` if the build is a debug build.
* Add comments atop each test file specifying the purpose of each test.
* Add `Rain::Platform::isDebug` to detect whether the current build is a debug build or otherwise.
* Split the configuration specification for `VsDevCmd.bat` into a default and local file for Windows systems needing to build using the `cl.exe` command line tool, and update VSCode tasks accordingly.
* Add recommended VSCode workspace extensions to `.vscode/extensions.json`.
* Move the Visual Studio test project tempalte to `build/rain.template/`.
* Move recommended VSCode settings to `.vscode/settings.default.json` and remove `.vscode/settings.json` from version control.

## 6.5.1

* Makefile now links libraries after objects as intended.
* Revert to `march=native` during release builds.

## 6.5.0

* Use `TYPE const` instead of `const TYPE` when declaring variables, now standard in this codebase.
* Refreshed Windows Visual Studio project template configurations to be more concise.
  * Added building readme.
* Add additional build/debug tasks for Visual Studio Code.
* Revamp build script to output LF-only newline.
* Modified some tests to flush and be a bit more concise.

## 6.4.7

* Hotfix for global functions not `inline`-d.
* Small change to `rain.props` to use `C++17` instead of `C++-latest`.

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
