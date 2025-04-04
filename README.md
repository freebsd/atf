# Welcome to the ATF project!

ATF, or Automated Testing Framework, is a **collection of libraries** to
write test programs in **C, C++ and POSIX shell**.

The ATF libraries offer a simple API.  The API is orthogonal through the
various bindings, allowing developers to quickly learn how to write test
programs in different languages.

ATF-based test programs offer a **consistent end-user command-line
interface** to allow both humans and automation to run the tests.

ATF-based test programs **rely on an execution engine** to be run and
this execution engine is *not* shipped with ATF.
**[Kyua](https://github.com/freebsd/kyua/) is the engine of choice.**

## Download

Formal releases for source files are available for download from GitHub:

* [atf 0.23](../../releases/tag/atf-0.23), released on March 29th, 2025.

## Installation

You are encouraged to install binary packages for your operating system
wherever available:

* FreeBSD 10.0 and above: install the `atf` package with `pkg install atf`.

* NetBSD with pkgsrc: install the `pkgsrc/devel/atf` package.

* OpenBSD: install the `devel/atf` package with `pkg_add atf`.

* Ubuntu: install the `libatf-dev` and `atf-sh` packages with
  `apt install libatf-dev atf-sh`.

Should you want to build and install ATF from the source tree provided
here, follow the instructions in the [INSTALL file](INSTALL.md).

## More Reading

* AUTHORS: List of authors and contributors for this project.

* COPYING: License information.

* INSTALL.md: Compilation and installation instructions.  These is not the
  standard document shipped with many packages, so be sure to read it for
  things that are specific to ATF's build.

* NEWS.md: List of major changes between formal, published releases.

## Other documents

* `AUTHORS`: List of authors and contributors for this project.

* `COPYING`: License information.

* `INSTALL.md`: Compilation and installation instructions.  These is not the
  standard document shipped with many packages, so be sure to read it for
  things that are specific to ATF's build.

* `NEWS.md`: List of major changes between formal, published releases.

## Support

Please use the
[FreeBSD-testing@FreeBSD.org](mailto:FreeBSD-testing@FreeBSD.org) mailing list
for any support inquiries related to `atf-c`, `atf-c++`, `atf-sh`, or `kyua`.
