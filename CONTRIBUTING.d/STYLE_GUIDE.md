# Style Guide and Conventions

## General Style Conventions

These general style conventions apply to all code contributions.

* Avoid adding trailing newlines and whitespace.
    * These slow down the integration process and are a distraction.
    * `git diff` will highlight them in red, as will the Files Changed tab in
      the pull request.

__This is enforced by clang-format/pre-commit__

## Coding Conventions

* Use 4-space indentation instead of hard-tabs.
    * This deviates from other FreeBSD style conventions.

__This is enforced by clang-format and scripts under admin/__

### C/C++ Coding Conventions

ATF, as a FreeBSD-adopted project, currently employs a hybrid style with its
C and C++ code.

Moreover, ATF has certain coding conventions which FreeBSD does not have.

#### In General

The FreeBSD coding style is documented in [style(9)][style-9-man-url].

The legacy coding style used by the project deviates from style(9) as follows:
* 4-space indents.
* Align indentation with opening ellipses with function
  declarations/definitions, e.g.,
```c
int
func(void *arg1, void *arg2, ...
     void *arg3, ...)
{
```
* All new files must be BSD-2-Clause licensed.
    * The `SPDX License Identifier` must be used, e.g.,
      `/** SPDX-License-Identifier: BSD-2-Clause **/`.

__This is enforced by [clang-format][clang-format-url]__

The following coding conventions are also enforced:

* Assertions must use macros provided by `sanity.hpp` instead of `assert(..)`
    * Uses of the `assert.h` and `cassert` headers are also flagged.
* `#ifdef` and `#ifndef` must use `#if defined(..)` and `#if !defined(..)`,
  respectively.

__This is enforced by scripts under admin/__

#### C-specific Coding Conventions

* C-style comments (`/* ... */`) must be used in code.
    * `// ...` are not permitted.

__This is enforced by scripts under admin/__

#### C++ style Coding Conventions

* C++-style comments (`// ... `) should be used in code.
    * This is not a hard requirement.
* `\n` must be used instead of `std::endl`.

__This is enforced by scripts under admin/__

### Shell script Conventions

#### Style conventions

* 4-space indents.
    * This is required.
* All scripts must have vim mode strings at the bottom of files, e.g.,
  `# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4`
* Braces are required around variable names, e.g., `${foo}`, not `$foo`.
* Variable assignment does not require quotes.
* `[`(1) should be used instead of `test`(1).
* Conditionals must be multiline, e.g.,
  ```sh
  if foo; then
    bar
  fi
  ```
  not:
  ```sh
  if foo; then bar; fi
  ```
__This is enforced by scripts under admin/__

#### Portability

For shell scripts, use [portable (POSIX) shell syntax/statements][posix-sh-url].
The system shell (`/bin/sh`) should be added to any scripts not directly
invoked by atf-sh.

### Suppressing Linter Rules

Linters can sometimes be too pedantic/rigid with the rules they enforce. If you are stuck in one of these kinds of scenarios, you can bypass the linter rules using one of the documented techniques in this section.

**Important**: Linter suppressing should be done as a last resort.

#### admin/ scripts and `clang-tidy`



#### clang-format

`clang-format` linting/autofixing can be suppressed by adding a comment with
`clang-format: off`. This directive tells `clang-format` to stop processing the
rest of the file.

In order to turn `clang-format` back on, you can use `clang-format: on`.

##### C Example
```c
#include <stdio.h>

/* clang-format: off */
#include <atf-c.h>
/* clang-format: on */
```

##### C++ Example
```c
#include <iostream>

// clang-format: off
#include <atf-c++.h>
// clang-format: on
```

## Documentation Conventions

All new documentation in the project is written in
[mandoc](https://man.freebsd.org/cgi/man.cgi?query=mdoc&sektion=7)-compatible
manual pages and [Markdown](https://www.markdownguide.org/).

__This is enforced by scripts under admin/__

[clang-format-url]: https://clang.llvm.org/docs/ClangFormat.html
[posix-sh-url]: https://pubs.opengroup.org/onlinepubs/9799919799/
[style-9-man-url]: https://man.freebsd.org/cgi/man.cgi?query=style&sektion=9
