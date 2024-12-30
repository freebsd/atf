# Welcome to the ATF project!

The Automated Testing Framework (ATF) is a collection of libraries to
implement test programs in a variety of languages. 
At the moment, ATF offers **C, C++ and POSIX** shell bindings with which to implement tests.
These bindings all offer a similar set of functionality and any test
program written with them exposes a consistent user interface.

**ATF-based test programs rely on a separate runtime engine to execute them.**
The runtime engine is in charge of isolating the test programs from the
rest of the system to ensure that their results are deterministic and that
they cannot affect the running system.  The runtime engine is also
responsible for gathering the results of all tests and composing reports.
The current runtime of choice is Kyua.

The ATF libraries offer a **simple API**.  The API is expressed with the aforementioned
interfaces, allowing developers to quickly write test programs in C, C++, and POSIX shell.

ATF-based test programs offer a **consistent end-user command-line
interface** to allow both humans and automation to run the tests.

ATF-based test programs **rely on an execution engine** to be run and
this execution engine is *not* shipped with ATF.
**[Kyua](https://github.com/jmmv/kyua/) is the engine of choice.**

## Download

Formal releases for source files are available for download from GitHub:

* [atf 0.22](https://github.com/freebsd/atf/releases/tag/atf-0.22), released on November 25th, 2024.

## Installation

You are encouraged to install binary packages for your operating system
wherever available:

* Fedora 20 and above: install the `atf` package with `yum install atf`.

* FreeBSD 10.0 and above: install the `atf` package with `pkg install atf`.

* NetBSD with pkgsrc: install the `pkgsrc/devel/atf` package.

* OpenBSD: install the `devel/atf` package with `pkg_add atf`.

* Ubuntu: install the `libatf-dev` and `atf-sh` packages with
  `apt install libatf-dev atf-sh`.

Should you want to build and install ATF from the source tree provided
here, follow the instructions in the [INSTALL file](INSTALL).

## Other documents

* `AUTHORS`: List of authors and contributors for this project.

* `COPYING`: License information.

* `INSTALL.md`: Compilation and installation instructions.  These is not the
  standard document shipped with many packages, so be sure to read it for
  things that are specific to ATF's build.

* `NEWS`: List of major changes between formal, published releases.

## Support

Please use the
[Github Issses page for ATF](https://github.com/freebsd/atf/issues) or
[freebsd-testing mailing list](freebsd-testing@freebsd.org)
for any support inquiries related to `atf-c`, `atf-c++` or `atf-sh`.

Please direct any questions about Kyua to the Kyua project's [Discussions page](https://github.com/freebsd/kyua/discussions) or the [freebsd-testing](freebsd-testing@freebsd.org) mailing list.