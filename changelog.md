# Changelog

## 3.0.0

`rain` is now updated with a new Visual Studio-based toolchain for tests. In addition, the process of adapting the libraries to Unix platforms is now in progress, but much functionality is still disabled on non-Windows platforms.

This update also moves header files to `./include/`, but leaves the source files in `./src/`.

## 2.0.0

* Remove `css` library files. `rain` is now a pure C++ library.

## 1.2.5

* Add a wide printing option for `creative.css`.

## 1.2.4

* Replace `hwnd` with `hWnd`.

## 1.2.3

* Added option to interpret value as `double` in `Rain::Configuration`.

## 1.2.2

* In Markdown CSS, the first `h1` only has `0` top margin during printing.

## 1.2.1

* Created Makefile for `.cpp` tests. Tests don't work yet, since the library is not compatible with non-Windows systems.

## 1.2.0

* Verified builds working for `Emilia`.
* Reinstantiated `.cpp` files for a better build process.
* Removed legacy `rain-libraries.hpp` in favor of `rain.hpp`.

## 1.1.1

* Removed VS test suites for CPP.
* Update `.gitignore` to include `.out` files.

## 1.1.0

* Moved design HTML out of repository. Created tests directory.
* Started upgrading CPP library to be cross-platform.
* Moved all CPP code into HPP files to avoid having to link.

## 1.0.3

* Fixed `border-color` bug in `grouped-h2+p` CSS pages.

## 1.0.2

* Standardized use of `base.css` CSS pages.
* Created Garamond font pages.

## 1.0.1

* Fixed mislinked print CSS page in `notes.css`.

## 1.0.0

Initial release.
