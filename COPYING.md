Redistribution terms for the Automated Testing Framework (ATF) project.

## Licensing

The ATF project, at is core, is a) BSD 2-clause licensed and b) the
copyright is owned by The NetBSD Foundation and The FreeBSD Foundation,
with some exceptions as noted below and elsewhere.

Some components are contain a BSD 3-clause license and were sponsored by
Google.

Finally, some sources were obtained from third-party projects and may have
different licenses from the core ATF project.

### Main Project License

Copyright (c) 2007-2019 The NetBSD Foundation, Inc.
Copyright (c) 2020-2026 The FreeBSD Foundation.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

### Google Licensed Code

Copyright 2010, 2011, 2014 Google Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

* Neither the name of Google Inc. nor the names of its contributors
  may be used to endorse or promote products derived from this software
  without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

### Relicensed code

The following code snippets have been taken from other projects.  Even
though they were not originally licensed under the terms above, the
original authors have agreed to relicense their work so that this project
can be distributed under a single license.  This section is put here just to
clarify this fact.

* configure.ac, Makefile.am: The original versions were derived from the
  ones in the XML Catalog Manager project, version 2.2.

  Author: Julio Merino <jmmv@users.sourceforge.net>

* admin/check-style.sh, admin/check-style-common.awk,
  admin/check-style-cpp.awk, admin/check-style-shell.awk: These files,
  except the first one, were first implemented in the Buildtool project.
  They were later adapted to be part of Boost.Process and, during that
  process, the shell script was created.

  Author: Julio Merino <jmmv84@gmail.com>

* m4/ac_cxx_compile_stdcxx.m4: This file comes from the
  [autoconf-archive][] project and is GPLv3 licensed, with exceptions as
  noted in the project.

  Please see the commit history for the file for authorship details.

  Author: Enji Cooper <ngie@FreeBSD.org>

[autoconf-archive]: https://github.com/autoconf-archive/autoconf-archive

===========================================================================
vim: filetype=markdown:textwidth=75:expandtab:shiftwidth=2:softtabstop=2
