# rain

A cross-platform library primarily for use in web servers and related applications written in C++. The intended usage is to include `rain` as a submodule, and utilize its source files, but not its build files or test files.

See [changelog.md](changelog.md) for information on versioning.

## Dependencies

On Unix-based platforms, `rain`'s will expect a `g++` compiler, as well as GNU make. On Windows platforms, `rain` will expect a compatible Visual Studio C++ compiler or IDE, or a similar Cygwin environment. No additional libraries are required.

## Tests

Tests are located in `./test/`, and can be run from `./build/`. On a Unix-based platform, use GMU make. On Windows platforms, utilize the Visual Studio C++ solution, which contains a project per test.

To create new tests, use the `./build/template/` project to initialize directories and build pipelines.

## Linting & formatting

We use Clang with the options:

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
