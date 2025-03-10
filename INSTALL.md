# Installation instructions

ATF uses the GNU Automake, GNU Autoconf and GNU Libtool utilities as its
build system. These are used only when compiling the application from the
source code package. If you want to install ATF from a binary package, you
do not need to read this document.

For the impatient:

```sh
$ ./configure
$ make
$ sudo make install # or gain root privileges through su or doas
$ make installcheck
```

Or alternatively, install as a regular user into your home directory:

```sh
$ ./configure --prefix ~/local
$ make
$ make install
$ make installcheck
```

## Dependencies

To build and use ATF successfully you need:

-   A C++14 standards-compliant C/C++ toolchain
    -   Clang 3.4 or above ([Clang C++ status](https://clang.llvm.org/cxx_status.html))
    -   GCC 6.1 or above ([GCC C++ status](https://gcc.gnu.org/projects/cxx-status.html))
-   A POSIX compliant shell
    -   e.g. zsh, bash, tcsh
-   make

If you are building ATF from the code on the repository, you will also need
to have:

-   autoconf 2.68 or above
-   automake 1.9 or above
-   libtool
-   m4

## Regenerating the build system

If you are building ATF from code extracted from the repository, you must
first regenerate the files used by the build system. You will also need to
do this if you modify `configure.ac`, `Makefile.am` or any of the other build
system files. To do this, simply run:

```sh
$ autoreconf -i -s
```

For release builds, no extra steps are needed.

## General build procedure

To build and install the source package, you must follow these steps:

1. Configure the sources to adapt to your operating system. This is done
   using the `configure` script located on the sources' top directory,
   and it is usually invoked without arguments unless you want to change
   the installation prefix. More details on this procedure are given on a
   later section.

2. Build the sources to generate the binaries and scripts. Simply run
   `make` on the sources' top directory after configuring them. No
   problems should arise.

3. Install the program by running `make install`. You may need to become
   root to issue this step.

4. Issue any manual installation steps that may be required. These are
   described later in their own section.

5. Check that the installed programs work by running `make installcheck`.
   You do not need to be root to do this, even though some checks will not
   be run otherwise.

## Configuration flags

The most common, standard flags given to `configure` are:

-   `--prefix=directory`:

    **Possible values:** Any path

    **Default:** `/usr/local`

    Specifies where the program (binaries and all associated files) will
    be installed.

-   `--help`:

    Shows information about all available flags and exits immediately,
    without running any configuration tasks.

The following environment variables are specific to ATF's `configure`
script:

-   `ATF_BUILD_CC`:

    **Possible values:** empty, an absolute or relative path to a C compiler.

    **Default:** the value of CC as detected by the configure script.

    Specifies the C compiler that ATF will use at run time whenever the
    build-time-specific checks are used.

-   `ATF_BUILD_CFLAGS`:

    **Possible values:** empty, a list of valid C compiler flags.

    **Default:** the value of CFLAGS as detected by the configure script.

    Specifies the C compiler flags that ATF will use at run time whenever the
    build-time-specific checks are used.

-   `ATF_BUILD_CPP`:

    **Possible values:** empty, an absolute or relative path to a C/C++
    preprocessor.

    **Default:** the value of CPP as detected by `configure` script.

    Specifies the C/C++ preprocessor that ATF will use at run time whenever
    the build-time-specific checks are used.

-   `ATF_BUILD_CPPFLAGS`:

    **Possible values:** empty, a list of valid C/C++ preprocessor flags.

    **Default:** the value of `CPPFLAGS` as detected by the configure script.

    Specifies the C/C++ preprocessor flags that ATF will use at run time
    whenever the build-time-specific checks are used.

-   `ATF_BUILD_CXX`:

    **Possible values:** empty, an absolute or relative path to a C++ compiler.

    **Default:** the value of `CXX` as detected by the configure script.

    Specifies the C++ compiler that ATF will use at run time whenever the
    build-time-specific checks are used.

-   `ATF_BUILD_CXXFLAGS`:

    **Possible values:** empty, a list of valid C++ compiler flags.

    **Default:** the value of `CXXFLAGS` as detected by `configure` script.

    Specifies the C++ compiler flags that ATF will use at run time whenever
    the build-time-specific checks are used.

-   `ATF_SHELL`:

    **Possible values:** empty, an absolute path to a POSIX shell interpreter.

    **Default:** empty.

    Specifies the POSIX shell interpreter that ATF will use at run time to
    execute its scripts and the test programs written using the atf-sh
    library. If empty, the configure script will try to find a suitable
    interpreter for you.

The following flags are specific to ATF's `configure` script:

-   `--enable-developer`:

    **Possible values:** `yes`, `no`

    **Default:** `yes` in HEAD builds; `no` in release builds.

    Enables several features useful for development, such as the inclusion
    of debugging symbols in all objects or the enforcement of compilation
    warnings.

    The compiler will be executed with an exhaustive collection of warning
    detection features regardless of the value of this flag. However, such
    warnings are only fatal when `--enable-developer` is `yes`.
