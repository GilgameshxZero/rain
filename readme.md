# rain

A cross-platform library written in C++. Features:

* HTTP & SMTP webserver.
* Low-level socket & WSA utilities.
* Common utility functions for low-level WinAPI calls and GDI+.
* Logging libraries, condition variables, common algorithms, and time utilities.
* OS/platform detection.
* Filesystem utilities.

See [changelog.md](changelog.md) for versioning and udpate information.

## Usage

Use `rain` by incorporating its source code, or using `git submodule`. To initialize, run

```bash
git submodule add https://github.com/GilgameshxZero/rain
```

When pulling a repository with a `rain` dependency, run

```bash
git submodule init
git submodule update
```

Source files (`*.cpp`) in `/src/` should be included in the build of the `rain`-dependent application. `/include/` should be specified as an additional include location (`-Iinclude` on `g++`).

## Dependencies

`rain` is built on zero external dependencies.

The only requirement is a C++ compiler. On Windows, `rain`, expects a Visual Studio C++ compiler and IDE. On Unix-based platforms, including Cygwin, `rain` expects `g++` and GNU `make`.

## Building the tests

All build-related files for `rain` are located in `/build/`. Each test inhabits one file under `/test/`.

### For Windows

Builds are managed via Visual Studio on Windows. Each test is structured as a project under the solution `/build/rain.sln`. New tests can be incorporated by creating a new project under the `rain` solution via the template in `/build/template.zip`, and including the corresponding test file from `/test/`.

To build on Cygwin with GNU make, utilize the `make` command from `/build/` and refer to the makefile documentation for Unix systems.

### For Unix

The GNU makefile `/build/makefile` compiles and runs all tests under `/test/` by default. It features the following rules.

Rule or flag|Usage
-|-
`DEBUG=1`|By default, build for release with `-O3` optimization. When this flag is set, build for default with no optimization.
`testall`|Build and run all tests.
`test X`|Build and run test `X`.
`clean`|Remove generated files (`/bin/` and `/obj/`).
`increment-build`|Called automatically on each build, increments the build number by `1`.

## Linting and formatting

`rain`'s source and header code is formatted with Clang with the following options:

```json
{
	BasedOnStyle: Chromium,
	FixNamespaceComments: false,
	NamespaceIndentation: All,
	PointerAlignment: Right,
	UseTab: Always,
	TabWidth: 2,
	AlignAfterOpenBracket: DontAlign,
	IncludeBlocks: Preserve,
	ContinuationIndentWidth: 2,
	AccessModifierOffset: 0,
	ReflowComments: false,
	BinPackArguments: false,
	BinPackParameters: false,
	AlignTrailingComments: false,
	IndentCaseLabels: true,
	AlignConsecutiveDeclarations: false,
	ColumnLimit: 0,
}
```
