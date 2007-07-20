//
// Automated Testing Framework (atf)
//
// Copyright (c) 2007 The NetBSD Foundation, Inc.
// All rights reserved.
//
// This code is derived from software contributed to The NetBSD Foundation
// by Julio M. Merino Vidal, developed as part of Google's Summer of Code
// 2007 program.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. All advertising materials mentioning features or use of this
//    software must display the following acknowledgement:
//        This product includes software developed by the NetBSD
//        Foundation, Inc. and its contributors.
// 4. Neither the name of The NetBSD Foundation nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND
// CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
// IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
// IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
}

#include <fstream>

#include <atf.hpp>

#include "atfprivate/exceptions.hpp"
#include "atfprivate/fs.hpp"

static
void
create_files(void)
{
    ::mkdir("files", 0755);
    ::mkdir("files/dir", 0755);

    std::ofstream os("files/reg");
    os.close();

    // TODO: Should create all other file types (blk, chr, fifo, lnk, sock)
    // and test for them... but the underlying file system may not support
    // most of these.  Specially as we are working on /tmp, which can be
    // mounted with flags such as "nodev".  See how to deal with this
    // situation.
}

// ------------------------------------------------------------------------
// Test cases for the "path" class.
// ------------------------------------------------------------------------

ATF_TEST_CASE(path_normalize);
ATF_TEST_CASE_HEAD(path_normalize)
{
    set("descr", "Tests the path's normalization");
}
ATF_TEST_CASE_BODY(path_normalize)
{
    using atf::fs::path;

    try {
        path("");
        ATF_FAIL("Construction of an empty path did not throw an error");
    } catch (const atf::fs::path_error& es) {
    }

    ATF_CHECK_EQUAL(path(".").str(), ".");
    ATF_CHECK_EQUAL(path("..").str(), "..");

    ATF_CHECK_EQUAL(path("foo").str(), "foo");
    ATF_CHECK_EQUAL(path("foo/bar").str(), "foo/bar");
    ATF_CHECK_EQUAL(path("foo/bar/").str(), "foo/bar");

    ATF_CHECK_EQUAL(path("/foo").str(), "/foo");
    ATF_CHECK_EQUAL(path("/foo/bar").str(), "/foo/bar");
    ATF_CHECK_EQUAL(path("/foo/bar/").str(), "/foo/bar");

    ATF_CHECK_EQUAL(path("///foo").str(), "/foo");
    ATF_CHECK_EQUAL(path("///foo///bar").str(), "/foo/bar");
    ATF_CHECK_EQUAL(path("///foo///bar///").str(), "/foo/bar");
}

ATF_TEST_CASE(path_is_absolute);
ATF_TEST_CASE_HEAD(path_is_absolute)
{
    set("descr", "Tests the path::is_absolute function");
}
ATF_TEST_CASE_BODY(path_is_absolute)
{
    using atf::fs::path;

    ATF_CHECK( path("/").is_absolute());
    ATF_CHECK( path("////").is_absolute());
    ATF_CHECK( path("////a").is_absolute());
    ATF_CHECK( path("//a//").is_absolute());
    ATF_CHECK(!path("a////").is_absolute());
    ATF_CHECK(!path("../foo").is_absolute());
}

ATF_TEST_CASE(path_is_root);
ATF_TEST_CASE_HEAD(path_is_root)
{
    set("descr", "Tests the path::is_root function");
}
ATF_TEST_CASE_BODY(path_is_root)
{
    using atf::fs::path;

    ATF_CHECK( path("/").is_root());
    ATF_CHECK( path("////").is_root());
    ATF_CHECK(!path("////a").is_root());
    ATF_CHECK(!path("//a//").is_root());
    ATF_CHECK(!path("a////").is_root());
    ATF_CHECK(!path("../foo").is_root());
}

ATF_TEST_CASE(path_branch_path);
ATF_TEST_CASE_HEAD(path_branch_path)
{
    set("descr", "Tests the path::branch_path function");
}
ATF_TEST_CASE_BODY(path_branch_path)
{
    using atf::fs::path;

    ATF_CHECK_EQUAL(path(".").branch_path().str(), ".");
    ATF_CHECK_EQUAL(path("foo").branch_path().str(), ".");
    ATF_CHECK_EQUAL(path("foo/bar").branch_path().str(), "foo");
    ATF_CHECK_EQUAL(path("/foo").branch_path().str(), "/");
    ATF_CHECK_EQUAL(path("/foo/bar").branch_path().str(), "/foo");
}

ATF_TEST_CASE(path_leaf_name);
ATF_TEST_CASE_HEAD(path_leaf_name)
{
    set("descr", "Tests the path::leaf_name function");
}
ATF_TEST_CASE_BODY(path_leaf_name)
{
    using atf::fs::path;

    ATF_CHECK_EQUAL(path(".").leaf_name(), ".");
    ATF_CHECK_EQUAL(path("foo").leaf_name(), "foo");
    ATF_CHECK_EQUAL(path("foo/bar").leaf_name(), "bar");
    ATF_CHECK_EQUAL(path("/foo").leaf_name(), "foo");
    ATF_CHECK_EQUAL(path("/foo/bar").leaf_name(), "bar");
}

ATF_TEST_CASE(path_compare_equal);
ATF_TEST_CASE_HEAD(path_compare_equal)
{
    set("descr", "Tests the comparison for equality between paths");
}
ATF_TEST_CASE_BODY(path_compare_equal)
{
    using atf::fs::path;

    ATF_CHECK(path("/") == path("///"));
    ATF_CHECK(path("/a") == path("///a"));
    ATF_CHECK(path("/a") == path("///a///"));

    ATF_CHECK(path("a/b/c") == path("a//b//c"));
    ATF_CHECK(path("a/b/c") == path("a//b//c///"));
}

ATF_TEST_CASE(path_compare_different);
ATF_TEST_CASE_HEAD(path_compare_different)
{
    set("descr", "Tests the comparison for difference between paths");
}
ATF_TEST_CASE_BODY(path_compare_different)
{
    using atf::fs::path;

    ATF_CHECK(path("/") != path("//a/"));
    ATF_CHECK(path("/a") != path("a///"));

    ATF_CHECK(path("a/b/c") != path("a/b"));
    ATF_CHECK(path("a/b/c") != path("a//b"));
    ATF_CHECK(path("a/b/c") != path("/a/b/c"));
    ATF_CHECK(path("a/b/c") != path("/a//b//c"));
}

ATF_TEST_CASE(path_concat);
ATF_TEST_CASE_HEAD(path_concat)
{
    set("descr", "Tests the concatenation of multiple paths");
}
ATF_TEST_CASE_BODY(path_concat)
{
    using atf::fs::path;

    ATF_CHECK_EQUAL((path("foo") / "bar").str(), "foo/bar");
    ATF_CHECK_EQUAL((path("foo/") / "/bar").str(), "foo/bar");
    ATF_CHECK_EQUAL((path("foo/") / "/bar/baz").str(), "foo/bar/baz");
    ATF_CHECK_EQUAL((path("foo/") / "///bar///baz").str(), "foo/bar/baz");
}

// ------------------------------------------------------------------------
// Test cases for the "directory" class.
// ------------------------------------------------------------------------

ATF_TEST_CASE(directory_read);
ATF_TEST_CASE_HEAD(directory_read)
{
    set("descr", "Tests the directory class creation, which reads the "
                 "contents of a directory");
}
ATF_TEST_CASE_BODY(directory_read)
{
    using atf::fs::directory;
    using atf::fs::path;

    create_files();

    directory d(path("files"));
    ATF_CHECK_EQUAL(d.size(), 4);
    ATF_CHECK(d.find(".") != d.end());
    ATF_CHECK(d.find("..") != d.end());
    ATF_CHECK(d.find("dir") != d.end());
    ATF_CHECK(d.find("reg") != d.end());
}

ATF_TEST_CASE(directory_names);
ATF_TEST_CASE_HEAD(directory_names)
{
    set("descr", "Tests the directory's names method");
}
ATF_TEST_CASE_BODY(directory_names)
{
    using atf::fs::directory;
    using atf::fs::path;

    create_files();

    directory d(path("files"));
    std::set< std::string > ns = d.names();
    ATF_CHECK_EQUAL(ns.size(), 4);
    ATF_CHECK(ns.find(".") != ns.end());
    ATF_CHECK(ns.find("..") != ns.end());
    ATF_CHECK(ns.find("dir") != ns.end());
    ATF_CHECK(ns.find("reg") != ns.end());
}

// ------------------------------------------------------------------------
// Test cases for the "file_info" class.
// ------------------------------------------------------------------------

ATF_TEST_CASE(file_info_stat);
ATF_TEST_CASE_HEAD(file_info_stat)
{
    set("descr", "Tests the file_info creation through ::stat and its "
                 "contents");
}
ATF_TEST_CASE_BODY(file_info_stat)
{
    using atf::fs::file_info;
    using atf::fs::path;

    create_files();

    {
        path p("files/dir");
        file_info fi(p);
        ATF_CHECK(fi.get_path() == p);
        ATF_CHECK(fi.get_type() == file_info::dir_type);
    }

    {
        path p("files/reg");
        file_info fi(p);
        ATF_CHECK(fi.get_path() == p);
        ATF_CHECK(fi.get_type() == file_info::reg_type);
    }
}

ATF_TEST_CASE(file_info_directory);
ATF_TEST_CASE_HEAD(file_info_directory)
{
    set("descr", "Tests the file_info creation through directory and its "
                 "contents");
}
ATF_TEST_CASE_BODY(file_info_directory)
{
    using atf::fs::directory;
    using atf::fs::file_info;
    using atf::fs::path;

    create_files();

    directory d(path("files"));

    {
        directory::const_iterator iter = d.find("dir");
        ATF_CHECK(iter != d.end());
        const file_info& fi = (*iter).second;
        ATF_CHECK(fi.get_path() == path("files/dir"));
        ATF_CHECK(fi.get_type() == file_info::dir_type);
    }

    {
        directory::const_iterator iter = d.find("reg");
        ATF_CHECK(iter != d.end());
        const file_info& fi = (*iter).second;
        ATF_CHECK(fi.get_path() == path("files/reg"));
        ATF_CHECK(fi.get_type() == file_info::reg_type);
    }
}

// ------------------------------------------------------------------------
// Test cases for the free functions.
// ------------------------------------------------------------------------

ATF_TEST_CASE(change_directory);
ATF_TEST_CASE_HEAD(change_directory)
{
    set("descr", "Tests the change_directory function");
}
ATF_TEST_CASE_BODY(change_directory)
{
    using atf::fs::change_directory;
    using atf::fs::get_current_dir;
    using atf::fs::path;

    create_files();
    const path old = get_current_dir();

    try {
        change_directory(path("files/reg"));
        ATF_FAIL("Changing directory to a file succeeded");
    } catch (const atf::system_error& e) {
    }
    ATF_CHECK(get_current_dir() == old);

    path old2 = change_directory(path("files"));
    ATF_CHECK(old2 == old);
    path old3 = change_directory(path("dir"));
    ATF_CHECK(old3 == old2 / "files");
    path old4 = change_directory(path("../.."));
    ATF_CHECK(old4 == old3 / "dir");
    ATF_CHECK(get_current_dir() == old);
}

ATF_TEST_CASE(create_temp_dir);
ATF_TEST_CASE_HEAD(create_temp_dir)
{
    set("descr", "Tests the create_temp_dir function");
}
ATF_TEST_CASE_BODY(create_temp_dir)
{
    using atf::fs::create_temp_dir;
    using atf::fs::exists;
    using atf::fs::path;

    path tmpl("testdir.XXXXXX");
    path t1 = create_temp_dir(tmpl);
    path t2 = create_temp_dir(tmpl);
    ATF_CHECK(t1.str().find("XXXXXX") == std::string::npos);
    ATF_CHECK(t2.str().find("XXXXXX") == std::string::npos);
    ATF_CHECK(t1 != t2);
    ATF_CHECK(!exists(tmpl));
    ATF_CHECK( exists(t1));
    ATF_CHECK( exists(t2));

    // TODO: Once there is a way to check for the file modes in file_info,
    // ensure that the directories are secure.  This will be done when
    // implementing executable-files filtering in atf-run.
}

ATF_TEST_CASE(exists);
ATF_TEST_CASE_HEAD(exists)
{
    set("descr", "Tests the exists function");
}
ATF_TEST_CASE_BODY(exists)
{
    using atf::fs::exists;
    using atf::fs::path;

    create_files();

    ATF_CHECK( exists(path("files")));
    ATF_CHECK(!exists(path("file")));
    ATF_CHECK(!exists(path("files2")));

    ATF_CHECK( exists(path("files/.")));
    ATF_CHECK( exists(path("files/..")));
    ATF_CHECK( exists(path("files/dir")));
    ATF_CHECK( exists(path("files/reg")));
    ATF_CHECK(!exists(path("files/foo")));
}

ATF_TEST_CASE(get_current_dir);
ATF_TEST_CASE_HEAD(get_current_dir)
{
    set("descr", "Tests the get_current_dir function");
}
ATF_TEST_CASE_BODY(get_current_dir)
{
    using atf::fs::change_directory;
    using atf::fs::get_current_dir;
    using atf::fs::path;

    create_files();

    path curdir = get_current_dir();
    change_directory(path("."));
    ATF_CHECK(get_current_dir() == curdir);
    change_directory(path("files"));
    ATF_CHECK(get_current_dir() == curdir / "files");
    change_directory(path("dir"));
    ATF_CHECK(get_current_dir() == curdir / "files/dir");
    change_directory(path(".."));
    ATF_CHECK(get_current_dir() == curdir / "files");
    change_directory(path(".."));
    ATF_CHECK(get_current_dir() == curdir);
}

ATF_TEST_CASE(rm_rf);
ATF_TEST_CASE_HEAD(rm_rf)
{
    set("descr", "Tests the rm_rf function");
}
ATF_TEST_CASE_BODY(rm_rf)
{
    using atf::fs::get_current_dir;
    using atf::fs::exists;
    using atf::fs::path;
    using atf::fs::rm_rf;

    create_files();

    // rm_rf is currently restricted to use absolute paths.
    path p = get_current_dir() / "files";

    ATF_CHECK( exists(p));
    ATF_CHECK( exists(p / "dir"));
    ATF_CHECK( exists(p / "reg"));
    rm_rf(p);
    ATF_CHECK(!exists(p));
}

// ------------------------------------------------------------------------
// Main.
// ------------------------------------------------------------------------

ATF_INIT_TEST_CASES(tcs)
{
    // Add the tests for the "path" class.
    tcs.push_back(&path_normalize);
    tcs.push_back(&path_is_absolute);
    tcs.push_back(&path_is_root);
    tcs.push_back(&path_branch_path);
    tcs.push_back(&path_leaf_name);
    tcs.push_back(&path_compare_equal);
    tcs.push_back(&path_compare_different);
    tcs.push_back(&path_concat);

    // Add the tests for the "directory" class.
    tcs.push_back(&directory_read);
    tcs.push_back(&directory_names);

    // Add the tests for the "file_info" class.
    tcs.push_back(&file_info_stat);
    tcs.push_back(&file_info_directory);

    // Add the tests for the free functions.
    tcs.push_back(&get_current_dir);
    tcs.push_back(&exists);
    tcs.push_back(&change_directory);
    tcs.push_back(&create_temp_dir);
    tcs.push_back(&rm_rf);
}
