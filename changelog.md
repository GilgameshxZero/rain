# Changelog

## 7.1.43

1. `auto` type inference on `PrimeModulus`.

## 7.1.42

1. Added implementation for SCC/condensation graph algorithms by Tarjan and Kosarju.

## 7.1.41

1. Rename `PrimeModulusRing` to `PrimeModulusField`.

## 7.1.40

1. Add cast operators and fix cast precendence in `PrimeModulusRing`.

## 7.1.39

1. Various `Algorithm` fixes.
2. Update `ModRing` to `PrimeModulusRing` with support for non-compile-time moduli, and mod factorials and chooses.
3. Add test for Max-Flow.
4. Fix `Networking::Client` test.
5. `#include <fstream>` in `Networking::Body` to fix compilation error.
6. Split Fibonacci, bit manipulation, and GCD/LCM code and tests.

## 7.1.38

1. Allow construction of body directly from `std::filebuf`.

## 7.1.37

1. Remove Visual Studio build pipeline support.
2. Add `SharedLockGuard` to `Multithreading`.

## 7.1.36

1. Add option to test subpath with `absolute` instead of `canonical`, to allow for symlink subpaths.

## 7.1.35

1. Add Edmonds-Karp MFMC implementation.

## 7.1.34

1. Fix bug in KMP search to return the first, not last match.

## 7.1.33

1. Clean up some syntax.

## 7.1.32

1. Add support for HTTP Authorization header.

## 7.1.31

1. Added default rate limiting of 60 connections/second/IP to base socket server.
2. Changed HTTP worker `filters` to return a const reference to a vector instead, so that it is constant time with number of filters instead of linear.

## 7.1.30

1. `onRequest` for HTTP checks against the filter method first before the host or path string.
2. Prefer using `{}` braces initialization rather than `=` initialization for clarity. `=` can be confusing as it implies the use of a copy assignment operator, whereas in reality it doesn’t.

## 7.1.29

1. Add `tests` command for `make`.
2. Fix offset `__builtin_clzll` in `mostSignificant1BitIdx`.
3. Include headers to define `SIZE_MAX` in `algorithm/fenwick` and `UCHAR_MAX` in `algorithm/huffman`.
4. Define function/class prototypes in `data/serializer` to fix compilation errors.
5. Fix unused pragma/variable warnings in tests.

## 7.1.28

1. Create the windows `nmake` command `tests`, which compiles then runs all tests.
2. Added corresponding `tests-debug` task to VSCode.
3. Rework `networking-http-server` and `networking-smtp-server` tests to not wait for 60s at the end.
4. Rework `windows` test to automatically close the MessageBox after 3 seconds.
5. Fix compilation bug in `data/serializer`.

## 7.1.27

1. Explicit casts for some types relating to `std::size_t`.

## 7.1.26

* Add `HuffmanStreamBuf` to enable encoding/decoding with the Huffman compression algorithm.

## 7.1.25

* Add `ModRing` `cout` test.

## 7.1.24

* Add many `const` modifiers to `ModRing` parameters.

## 7.1.23

* Added $O(\log N)$ Fibonacci number implementation, with optional mod via `ModRing`.
* Added some `const` overloads for `ModRing`.

## 7.1.22

* Add implementation for integer modulus ring supporting basic arithmetic operations in `Algorithm::ModRing`.
* Enforced x64 linking in MSVC Make.

## 7.1.21

* Fix cookie type parsing.
* Added `Set-Cookie` header type.
* Allow `ResponseAction` to be moved.

## 7.1.20

* Added TODO comment.

## 7.1.19

* Added DSU/union-find data structure.
* Make Fenwick tree static size.
* Clarified some comments in segment tree.

## 7.1.18

* Resolve signed/unsigned type mismatches explicitly.
* Add HTTP query parameter resolution functionality.

## 7.1.17

* Added case for MacOS in the `makefile` for differences in need for `-lstdc++fs`.

## 7.1.16

* `Serializer` no longer remaps memory addreses nor does it track serialization references. Any memory addresses present in data to be serialized must be treated before serialization.

## 7.1.15

* `Serializer` now houses a simplified reference schema and does not print headers.
* `Serializer` does not automatically serialize underlying memory for pointers. However, memory addresses are converted to serialization references during serialization.
* Removed all VS solutions and projects from the repository. Tests are built using `nmake`; however, the rain template is left to enable creating projects for `rain` tests and derivatives.

## 7.1.14

* Refactored segtree to virtualize 5 basic operations instead of 3, enabling separate `Value` and `Result` types for tree storage vs. query storage.
* Added basic serialization/deserialization routines in `Rain::Data` and tests. These need more work to ensure stability, but work for now.

## 7.1.13

* Optimized segtree implementation to use a complete binary tree, such that boundary checks don’t need to be made. Trades around 50% more memory for a similar constant factor in performance.
* Allows segtree `Update` to be `bool` by correctly typing virtual functions signatures with vector references.
* Added performance test for segtree.

## 7.1.12

* Added segment tree implementation and test.
* Added some basic algorithm utilities and tests: msb/lsb, popcount, gcd/lcm.

## 7.1.11

* Revamped VSCode build routines with a unified `make`/`nmake` makefile.
* Added Fenwick tree implementation and test.

## 7.1.10

Rename Mailbox `domain` to `host` to better clarify its type.

## 7.1.9

Domain names are compared case-insensitively. In particular, SMTP now ignores case for domain names.

## 7.1.8

Formatting for VSCode build files.

## 7.1.7

Proper `MediaType` for `.webm`. No more unused variable warnings for `Windows::Exception` on non-Windows compile.

## 7.1.6

Allow specifying `MediaType` parameter as separate string in constructor.

## 7.1.5

Set default automation shell to CMD on Windows for new VSCode update.

## 7.1.4

Rename `RAIN_VERSION_BUILD` to `VERSION_BUILD` in makefile to be more consistent with derivative projects.

## 7.1.3

Update `readme` to reflect new `makefile`.

## 7.1.2

This release primarily focuses on improving compatibility with Windows derivative project builds.

* `build.bat` no longer defines `_CONSOLE` by default, to be compatible with both CONSOLE and WINDOWS rain tests.
* `makefile` uses updated naming for some variables, and only builds but doesn’t run by default. In addition, it now correctly appends a `\n` to `build.hpp`.
  * The proper ending of `build.hpp` is important if it is to be included in a resource `.rc` file, since the resource compiler is a limited-syntax C compiler and needs the trailing newline for all files to compile properly.
* `rain-debug.props` and `rain-release.props` is merged into `rain.props` to reduce project configuration overhead in `.vcxproj`.
  * `rain.props` does not include the rain includes by default; this is to be specified in each project properties. This allows for derivative projects to use `rain.props` directly without including unnecessary directories.
* Added test `windows` for general Windows-library tests.
* All existing Visual Studio projects updated to use the new property sheets specifications.
* `STRINGIFY` macro is separated out into `string/stringify.hpp` for possible inclusion in `.rc` files if usage is needed.
* Removed incorrect specification of `_WIN32_WINNT`, which is set by the compiler, not by us, for the current Windows version.
* Explicitly undefine `min` and `max` for all `algorithm` and `Windows.h` includes, specifically so that the resource compiler does not get confused.
* Add exceptions for `Rain::Windows` simliar to `Rain::Networking`, and code-share the `FormatMessage` block.

The change of usages of `strto*` to `sto*` was considered, but ultimately rejected, as the latter throw exceptions and ultimately lead to bulkier code.

## 7.1.1

Resolve Client constructor ambiguities in `server`-tests on POSIX.

## 7.1.0 “Rosario”

This “minor” release introduces sweeping inheritance and public interface changes to the `Networking` module as well as small changes to other components.

### Networking inheritance revamp

The primary goal of this revamp is to enable selective, compile-time verification of different types of sockets and their imbued properties, and satisfy LSP inheritance. To this end, the socket inheritance is separated into four broad types of classes:

* Socket options, postfixed with `SocketOption`.
* Resource-holding sockets, postfixed with `Socket`. There is only one class of this type.
* Family/Type/Protocol interfaces (F/T/P), postfixed with F/T/P-`Interface`.
* Socket specializations, postfixed with `SocketSpec`.

In addition, each class has an `Interface` class associated with, named with `Interface` postpended to the class name. Interfaces must have default constructors, must be virtually inherited, and define available functions on that class as well as enable any code-sharing amongst subclasses. Classes which are further templated have a second `InterfaceInterface` class, where the second interface is any commonalities between all template versions of the templated `Interface` class.

A Socket is built by first specifying the F/T/P interfaces, which define the `family()`, `type()`, and `protocol()` virtual functions, then subclassing with `Socket` to RAII-style manage the platform socket resource. One difference from previous `rain` versions is that `Socket` can no longer be moved at all in order to avoid unnecessary kernel calls and socket creations. Then, the socket options are specified; these have their constructors call `setsockopt` on the underlying socket with the appropriate parameters. Finally, additionally functionality is appended with the socket specialization classes. Both socket option and specialization classes all template inherit from a resource-holding Socket; thus, a final specified Socket is built as many layers of templated classes.

Each protocol layer defines six socket specializations: `SocketSpec`, `NamedSocketSpec`, `ConnectedSocketSpec`, `ClientSocketSpec`, `WorkerSocketSpec`, and `ServerSocketSpec`. Named sockets inherit the base socket; connected sockets inherit the named sockets, and client and worker sockets are connected while server sockets inherit from the named sockets only. A protocol layer built atop another one will have each of its socket specializations additionally inherit the lower layer’s corresponding socket specialization.

In this way, a socket instance can be stored as a pointer or reference in any of its socket specialization, socket option, socket, or F/T/P superclasses.

All socket specializations additionally follow RAII convensions: connected sockets must be connected at construction; server sockets must be bound and listening at construction. Additionally, original socket functionality from `socket.hpp` has now been split to its corresponding specialization file depending on where it is needed.

A similar inheritance was used for the M/R/R classes, which also follow a similar naming scheme. The pre/post-processing chains have been scrapped for a much simpler “call super” pattern. Now, pp-chains may be appended either in the M/R/R classes during send/recv via virtual dispatch, or in the client/worker classes during their send/recv.

Finally, request handling in HTTP is overhauled to enable simpler syntax for defining request filters and handlers. See the tests for examples.

### Networking bugfixes

* `send` no longer raises `SIGPIPE` occasionally on POSIX.
* `accept`ed sockets are now correctly marked non-blocking once accepted.
* Connecting to MX records now correctly tries them in order of priority instead of all at the same time, preventing waiting for the timeout of the longest-to-connect.
* `send`/`recv` do not fail on other flags set from `poll`. However, `connect` fails if additional flags were set in addition to `WRITE_NORMAL`.

### Smaller changes to Networking

* Removed timeouts on `Resolve` since it is reasonable to expect a DNS response in reasonable time on most implementations, and the MX record resolution was leaking memory in the form of circular `std::shared_ptr` references.
* WSA code is now stored as an automatic-duration global variable which performs necessary startup and cleanup on Windows.
* Now can move a `std::stringbuf` directly into `Body`.
* `RequestResponse` renamed to `ReqRes`.
* Add functions to observe workers and threads count in a server from its underlying threadpool.

### Smaller changes

* Revamp LRU cache to avoid any copying of keys or values, and remove the meaningless private inheritance.
* `consumeThrowable` now properly works as expected with moved callables as well as mutable lambdas.
* Added error messages for custom errors in `Networking::Exception`.
* Add hash function for `std::filesystem::path`.
* Do not redefine `STRINGIFY` and `STRINGIFY_INNER` if already defined.
* Macro `RAIN_FUNCTIONAL_RESOLVE_OVERLOAD` resolves and overloaded name for passing as a callable parameter.
* Add `MediaType` to Networking representing a MIME Type.
* `RAIN_DEBUG_NDEBUG` renamed to `RAIN_PLATFORM_NDEBUG`.
* `localtime_r` is now a function in `Rain::Time` on every platform.
* `Timeout` uses `steady_clock` by design instead of a templated clock.
* Time output to streams defaults to `ms`.
* Remove `_UNICODE` preprocessor.
* Revamped tests in accordance with the new changes.

### Build procedure

* Updated `readme.md` to reflect changes in build procedure and note incompatibility with `g++` and `apple-clang`.
* Ignore Warning 4250 on Visual Studio project build procedures, since inheritance by dominance is an intended trait of the Networking module.
* Clean up `build/makefile` and default to `clang++` and allow override by setting environment variable `CXX`.
* Remove `*.filters` from Visual Studio projects and template.

## 7.0.12

Parsing mailbox from RCPT and MAIL commands in SMTP server is improved, but still does not work with mailboxes containing spaces.

## 7.0.11

* Fix a critical bug in `Networking::Server::serve`, where the timeout for `accept` was not refreshed on every `accept` call, leading to servers running longer than the default timeout (1 minute) to busy-wait instead of block.
* Update the `networking-smtp-server` test to reflect the new `DATA` default behavior to return `TRANSACTION_FAILED`.

## 7.0.10

* Allow empty `Mailbox` construction.
* Allow stream operators on TCP `Socket` specializations to allow sending of `std::streambuf *`.
* Fix `AuthMethod` parsing.
* Make `Rain::Time::localtime_r` available of every platform.

## 7.0.9

Remove unused flags from `g++` command-line options in `build/makefile`.

## 7.0.8

* Correctly parse out “FROM:” and “TO:” in the relevant SMTP commands in SMTP `Worker`.

## 7.0.7

* Revert changes to `Algorithm::LruCache` inheritance.

## 7.0.6

* Change `std::unordered_map` inheritance in `Algorithm::LruCache` to public to allow accessing base interface functions, while `.at` and `.insert_or_assign` is still overwritten.
* Default exception handling in HTTP and SMTP `Worker`s now uses `Error::consumeException`.

## 7.0.5

* Inline additional stream operator free functions in `Networking`.

## 7.0.4

* Update `RAIN_VERSION_REVISION` to `4` from `0`.

## 7.0.3

* Replace `build/build.bat` with more generic format allowing for child projects to use a very similar `build.bat` while only replacing `%PROJECT_NAME%`.
* Enforce 80-character line limit on `build/makefile`.
* `inline` `Literal::_zu`, `Literal::_re`, and `strcasecmp` as they caused problems with multiple definitions in child projects with more than 1 source file.

## 7.0.2

Two of two updates continued from `7.0.1`.

## 7.0.1

One of two updates to add `.vscode/settings.json` to version-control, but also ignore any changes to it.
The next update will re-add `.vscode/settings.json` to `.gitignore`, and re-introduce `.vscode/settings.DEFAULT.json` after a rename from `.vscode/settings.default.json`.

## 7.0.0 “Emancipation”

*The `Networking::Slave`s have been freed!*
*They are now `Networking::Worker`s of their own free will.*

This major release introduces changes to almost every module in `rain`. These changes are described below, organized by the affected module, sorted in order of importance.

### `Networking` module

The entire `Networking` module has been re-imagined.

#### Inheritance and subtyping

Inerhitance follows the pattern known as *base-specialization across protocol layers*, which is described below.

| Type \ Protocol                       | None | TCP (from None) | Request-Response (from TCP) | HTTP (from Request-Response) | SMTP (from Request-Response) |
| ------------------------------------- | ---- | --------------- | --------------------------- | ---------------------------- | ---------------------------- |
| `Socket` (base)                       | X    | X               | X                           | X                            | X                            |
| `Client` (`Socket` specialization)    | X    | X               | X                           | X                            | X                            |
| `Worker` (`Socket` specialization)    | X    | X               | X                           | X                            | X                            |
| `Server` (`Socket` specialization)    | X    | X               | X                           | X                            | X                            |
| `Message` (base)                      |      |                 | X                           | X                            | X                            |
| `Request` (`Message` specialization)  |      |                 | X                           | X                            | X                            |
| `Response` (`Message` specialization) |      |                 | X                           | X                            | X                            |

Sockets are still divided into three specializations: `Client`, `Server`, and `Worker` (renamed from `Slave`), all of which derive (eventually) from the base `Socket`. Each specialization defines their own contracts and interfaces. Networking protocols are represented as layers, where each layer provides its own implementation of the protocol `Socket` and the protocol specializations, each of which derive from the layer below. The base layer specializations derive from the top-layer protocol `Sockets`, instead of `virtual`ly inheriting each layer’s protocol `Socket` multiple times.

Higher-level protocol layers in `RequestResponse`, `Http`, and `Smtp` also define their own representations of `Request` and `Response`, whose commonalities are shared in their common superclass `Message`, defined for each protocol. Again, `Request` and `Response` are specializations of `Message`, whose inheritance pattern follows that of `Socket` and its specializations across protocol layers. That is, `Http::Request` inherits `RequestResponse::Request` which inherits `Http::Message` which inherits `RequestResponse::Message`.

Each of these base-specialization inheritance patterns across protocol layers is actually implemented as inheritance between *interface* classes, which define the behavior for that type of object, but are not meant to be directly instantiated. Instead, the concrete types such as `Http::Client` is itself derived from `Http::ClientInterface`, the latter of which follows the base-specialization inheritance pattern. Most of the time, the concrete type is simply a typedef of the interface. `Message`, `Request`, and `Response` also follow this *interface-concrete* pattern.

The classes implemented as part of the *base-specialization across protocol layers* are listed above in the chart. An ‘X’ indicates that that specialization or base exists on that protocol layer. Each protocol also indicates the protocol layer it is derived from, which each type indicates whether it is a base, or the base it is specialized from.

All inheritance is public unless otherwise specialized. `Socket` specializations tend to hide functionality, but this could be improved by specialization shared functionality in an additional protocol layer prior to `TCP`.

Below, changes to each protocol layer is summarized.

#### No protocol

* `Socket`s are now dual-stack, non-blocking, and no-linger by default.
* `Socket`s can now be interrupted, by supplying a connected `Socket` pair, through which all blocking calls are also called on. By breaking either of this pair, all future blocking calls are interrupted.
* There are now three modes to closing a socket: `shutdown`, `close`, and `abort`. `shutdown` shuts down either the read or write or both. `close` attempts to gracefully close by issuing a `shutdown` on write, then reading until graceful close from the peer, then aborting. Aborts call the kernel `::close` on the file descriptor. `close` may block during reading, and thus is supplied with a timeout.
* `connect` now has the option to parallel-connect, which spawns multiple `Socket`s and `std::thread`s, each of which attempts to connect to the address independently. The first to succeed is swapped with the current socket file descriptor, while the others are closed regardless of outcome.
* `Socket` operations are now thread-safe, locked via `std::mutex`.
* Timeouts no longer throw exceptions, but instead return special values. Normal error exceptions are still thrown.
* All blocking calls are now equipped with timeouts in the form of `Time::Timeout`.
* All networking errors are located in the `Networking` namespace, as most networking errors are shared between modules.
* Blocking calls are now implemented through `poll` rather than `select`.

All address/DNS resolution libraries have been placed into `Networking::Resolve`. Similarly to the blocking calls on `Socket`, blocking resolution is now supplied with a timeout. Since resolvers are typically synchronous without timeout, the `Resolve` timeouts simply leave a dangling thread to finish the resolver call while control flow returns to the caller. `getAddressInfo` now returns a list of `AddressInfo`s, a modern interface for the `addrinfo` struct.

`Server`s now implement their interrupt pair with all spawned `Worker`s, such that interrupting the server will interrupt all its `Worker`s. Additionally, all dynamic memory is now managed via smart pointers. `Worker`s now store the connection `AddressInfo` during construction by the `Server`. This is enfored across all `Worker`s as it is enforced in the no protocol `Worker` interface from which all other `Worker`s derive from. In addition, server behavior is now specified in the `Worker` via polymorphic overriding of `virtual` functions on the `Worker`.

#### TCP protocol

The TCP protocol is a new protocol layer with enforced `TCP`, `STREAM`-style sockets. All TCP `Socket`s multiple-inherit `std::iostream`, and can be streamed to/from for `send`/`recv`. While normal `send`/`recv` throw, the stream operators on a TCP `Socket` only set the stream as not good.

Streamability is implemented with a `send`/`recv` buffer wrapped in a `std::streambuf` supplied to the superclass `std::iostream`.

#### Request-Response protocol

Request-Response (R/R) protocols not only establish the Message-Request-Response base-specialization pattern (M/R/R), but also allow for request/response *pre/post-processing*. `send`/`recv` now use the stream operators and do not throw. In addition, a `maxRecvIdleDuration` can be specified for the maximum time between receiving two valid requests, and a `sendOnceTimeoutDuration` can be specified for the maximum time for attempting to `send` any data.

The stream operators and `send`/`recv` on R/R `Sockets` support the relevant timeouts and pre/post-processing by overriding virtual stream operators from the TCP `Socket`. Pre-processing allows for an incoming request/response to be pre-processed before it is received internally, while post-processing allows for an outgoing request/response to be processed before it is sent externally. `Client` and `Worker` support pre/post-processing symmetrically, since `Client`s receive `Response`s and send `Request`s while `Worker`s do the opposite.

M/R/Rs are streamable via stream operators on `std::iostream`s. All `Socket` `send`/`recv` on M/R/R types go through `sendWith`/`recvWith` on the M/R/R type, where the pre/post-processing happens. Downstream M/R/R subtypes may extend pre/post-processing by extending the *template-tagged chain* via polymorphic overriding in `sendWithImpl`/`recvWithImpl`.

#### HTTP protocol

Common types such as `Headers`, `TransferEncoding`, `Method`, `StatusCode`, and `Version` are now strongly typed via `enum`s. In addition, R/R parsing is now stream-based as the HTTP `Socket`s inherit from the streamable TCP `Socket`s. `Worker`s now delegate `Request` processing via a *template-tagged chain*, where subclasses can prepend to the chain and optionally return a `PreResponse`. The `PreResponse` dictates whether the `Worker` should return a response, and whether it should close afterwards.

Most `PreResponse` chain matching is implemented with a combination of checking the `Request` `Method` and `std::regex` matching on the target path.

* `Transfer-Encoding: chunked` is now supported.
* Backwards-compatibility with HTTP/0.9 and HTTP/1.0 is now supported. By default, `Server`s operate in HTTP/1.1.
* Includes a full implementation for `Client` now and supports parsing of both R/Rs.

#### SMTP protocol

Similarly to the HTTP protocol, common types such as `AuthMethod`, `Command`, `Mailbox`, and `StatusCode` have now been strongly typed, and R/R parsing is also stream-based. `Request` handler delegation is not implemented via a *template-tagged chain*, but instead via regular overriding of `virtual` methods. A single `virtual` method is denoted for each `Command`.

* Includes a full implementation for `Client` now and supports parsing of both R/Rs.

### `Error` module (renamed from `Error-Exception`)

* Introduces `consumeThrowable`, which takes arbitrary *Callable* objects and wraps them in a try/catch block. Any caught exceptions are ignored, with the option to print an error message to `std::cerr`.
* `RAIN_ERROR_LOCATION` macro identifies caller site, useful when passed to `consumeThrowable` for an error message.
* `Exception` constructor now takes an `Error` directly, isntead of having to go through `std::error_condition`.
* `Incrementer` implements an automatic-duration incrementer, whose affects are negated at destruction.

### `Multithreading` module

* `ThreadPool` tasks now are parameter-less. Any parameters should be captured via lambdas passed as tasks instead.
* `ThreadPool` mutexes are now `mutable`, allowing for several methods to be declared `const.`
* `ThreadPool` now uses smart pointer internals and `consumeThrowable` so that any exceptions satisfy the basic guarantee, and ideally won’t crash.
* `ThreadPool` now utilizes new `Time::Timeout` type.
* `UnlockGuard` performs the opposite of `std::lock_guard`, unlocking a mutex on construction and locking on destruction. This pattern is similar to the `std::condition_variable` pattern of unlocking during waits.

### `String` module

* Utility functions have been standardized to take as parameters either a `std::string` or a `char *` with a `std::size_t`.
* `WaterfallParser` renamed to `KeyedParser`, and now has public interfaces which use references instead of pointers.
* Added Base 64 encode/decode utilities for `std::string`s.
* All functions now return a non-`const` `char *` if necessary, even if parameters are `const`.
* Functions now make the assumption of UTF-8 narrow encoding on parameters unless specified otherwise.
* Additional case-agnostic comparators, hashes, and equality functors have been added for usage in `std::unordered_map`, `std::unordered_set`, and case-agnostic variants.
* `String::strcpy_s` and friends compatibility implementations have been removed as they provide little additional safety beyond the non-standard, non-safe versions. In fact, similar careful usage is required for both. In addition, CRT warnings have been disabled to allow for usage of the non-safe versions.

### `Time` module

* `Timeout` standardizes the timeout event representation and can be constructed from either a duration from the current time, or a time point in the future for the event to occur.
* Added `std::ostream` overloads for common `std::chrono::duration` types.

### `Algorithm` module

* KMP functions now use standardized parameters for strings as in the `String` module. 
* `computeKmpPartialMatch` now returns the partial match table directly as a `std::vector<std::size_t>`.
* KMP searches now return a pair of `match, candidate` instead of updating the reference to a `candidate` parameter.
* LRU cache now avoids unnecessary copying of internal key/value pairs by moving around `std::unique_ptr`s instead.

### Header module organization and smaller modules

* Headers with the same name as a directory in the same level now only include all files from that directory.
* General utility functions in a certain module are now located in `module/module.hpp` instead of just `module.hpp`, since `module.hpp` is used for the above purpose.
* `Debug` module has now been moved in with the `Platform` module.
* `Filesystem::subpath` has been renamed to `Filesystem::isSubpath`, and now converts to canonical paths internally.
* Defined the `_zu` `std::size_t` literal and `_re` `std::regex` literal in the inline namespace `Literal`. `using namespace Literal` now also imports literals from `std::literals`.
* `memmem` compatibility implementation is removed as KMP search is strictly faster, and the interface is more modern.
* `Platform` now supports only three major platforms: Windows, MacOS, and Linux. These are now defined as `enum class`es and have `std::ostream` overloads.
* `Time::sleepMs` removed as it provided little additional utility now with the import of `std::literals`.
* Windows module now also defines `NOMINMAX`, `UNICODE`, `_UNICODE`, and `_WIN32_WINNT` prior to including `Windows.h`. `_WIN32_WINNT`, the minimum version `rain` can run on, is specified to be Windows 7.

### Tests

* Each test primarily tests one module within `rain`. Tests have been renamed to be the namespace path to that module.
* Many important modules now have tests.

### Visual Studio Code integration

* In-IDE debugging now debugs x64 builds on any platform.
* Added tasks for release x64 builds.
* Build tasks on Windows are no longer specified in `.vscode/tasks.json`. Instead, `.vscode/tasks.json` calls `build/build.bat`.
* `build/increment-build.bat` renamed to `build/increment-version-build.bat`.
* Removed `yzhang.markdown-all-in-one` from extension recommendations.

### `cl.exe` Windows command-line build

* `build/VsDevCmd.default.bat`, which used to call `VsDevCmd.bat` to initialize the Visual Studio command-line, has now been replaced with `build/vcvarsall.bat` and `build/vcvarsall.DEFAULT.bat`. The former is `.gitignore`d, and may override the latter in specifying the local path to `vcvarsall.bat`. Both are called by `build/build.bat` to initialize the Visual Studio command-line.
* Removed `build/make.bat` as a proxy for calling WSL `make`.
* `build/build.bat` calls `cl.exe` with identical options as in the Visual Studio projects.

### Visual Studio integration

* `rain` template projects now use a common property page `build/rain.props`, with additional property pages `build/rain-debug.props` and `build/rain-release.props` set based on build configuration.
* Since tests were all changed and renamed, all associated projects have been re-added to the Visual Studio solution.
* `rain` template project icon changed.

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

* Don’t sort `using` in `.clang-format`.
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
* Create reference `HttpServerSlave` and `HttpServer` implementations which don’t process requests just yet.

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
