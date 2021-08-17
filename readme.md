# rain

Cross-platform, self-contained, header-only C++17 libraries with utilities for

* Sockets, TCP, HTTP, SMTP, general request/response-based protocols, DNS resolution (A, AAAA, MX),
* Threadpool-based thread management,
* Command-line, string, and base 64 parsing and encode/decode,
* Basic exception management, error management, and memory leak detection,
* Knuth-Morris-Pratt (KMP) string matching and template Least-Recently-Used (LRU) cache.

This document contains summary-level information on `rain`. The changelog at `changelog.md` contains updates in each version, as well as high-level information about the codebase. Finally, low-level documentation can be found with in-code comments as well as the tests in `test/`.

## Usage

The `rain` library utilizes the C++17 standard (or newer) and any projects built with `rain` must as well.

All library code is under `include/`. Since `rain` is header-only, no specialized build options are necessary. Either copy the contents of `include/` into the directory of the source file containing `main`, or add `include/` to the include paths for the compiler.

Unfortunately, `g++ v9.3.0` won’t build `rain` and later versions of `g++` up to `v11.2` all contain a constructor virtual dispatch bug which causes incorrect behavior. To build on POSIX-like platforms, use `clang` instead. On MacOS, `apple-clang` contains a bug with `std::function`; thus, build with an independent `clang` source, such as that from `homebrew`.

## Build

To run the tests under `test/`, one needs to invoke one of several build procedures under `build/`. By default, the target is `x64`. Depending on the platform/development environment, one or multiple of these may be available.

### `make`

`build/makefile` contains several functions for building:

* `runall`:  Builds and runs all tests in `test/` consecutively.
* `run TEST=$TEST_NAME`: Builds and runs a single test with the name `$TEST_NAME`.
* `all`: Build all the tests into `bin/`.
* `$TEST_NAME`: Build a single test with the name `$TEST_NAME`.
* `clean`: Deletes the `bin/` and `obj/` intermediates directories.

Builds default to debug. Specify `RELEASE=1` to build with release optimization. Specify `INSTRUMENT=1` to build with instrumentation options.

### Visual Studio

Each test is given its own project under the `build/rain.sln` solution for Visual Studio 2019.

All tests and new tests follow the project template specified in `build/rain.template/`.

### `cl.exe`

Command-line builds on Windows are supported via `cl.exe`, or with the pre-specified options in `build/build.bat`.

### Visual Studio Code

Visual Studio Code delegates to either the `make`-based or `cl.exe`-based command-line procedures with two launch options: `windows-debug-x64` and `posix-debug-x64`.

Visual Studio Code utilizes `build/build.bat` or `build/makefile` with two launch options: `windows-debug-x64` and `posix-debug-x64`.

An additional task `clean` is configured to remove the `bin/` and `obj/` directories.

## Development

The style guide is implicitly specified in `.clang-format`. Additionally, please place `const` specifiers after type specifiers, i.e. write `char const *tmp` instead of `const char *tmp`.
