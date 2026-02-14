/*!
    @file src/Deps/String/StringTest.cpp
    @brief String类测试代码
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-13
*/

#include <cassert>
#include <iostream>
#include "String.hpp"

using Fig::Deps::String;

static void test_ascii_sso()
{
    String s("hello");
    assert(s.size() == 5);
    assert(s[0] == U'h');
    assert(s.toStdString() == "hello");

    s.push_back(U'!');
    assert(s.toStdString() == "hello!");

    s.pop_back();
    assert(s.toStdString() == "hello");

    assert(s.starts_with("he"));
    assert(s.ends_with("lo"));
    assert(s.contains(U'e'));
}

static void test_ascii_heap()
{
    String a("abcdefghijklmnopqrstuvwxyz"); // > SSO
    assert(a.size() == 26);

    String b("123");
    a += b;

    assert(a.ends_with("123"));
    assert(a.find(U'1') == 26);
}

static void test_utf8_decode()
{
    String s("你好");
    assert(s.size() == 2);
    assert(s.toStdString() == "你好");

    s.push_back(U'!');
    assert(s.toStdString() == "你好!");
}

static void test_concat_modes()
{
    String a("abc");
    String b("你好");

    String c = a + b;
    assert(c.size() == 5);
    assert(c.toStdString() == "abc你好");

    String d = b + a;
    assert(d.toStdString() == "你好abc");
}

static void test_substr_erase_insert()
{
    String s("abcdef");

    String sub = s.substr(2, 3);
    assert(sub.toStdString() == "cde");

    s.erase(2, 2);
    assert(s.toStdString() == "abef");

    s.insert(2, String("CD"));
    assert(s.toStdString() == "abCDef");
}

static void test_replace()
{
    String s("hello world");
    s.replace(6, 5, String("Fig"));
    assert(s.toStdString() == "hello Fig");
}

static void test_find_rfind()
{
    String s("abcabcabc");

    assert(s.find(String("abc")) == 0);
    assert(s.find(String("abc"), 1) == 3);
    assert(s.rfind(String("abc")) == 6);
}

static void test_compare()
{
    String a("abc");
    String b("abd");
    String c("abc");

    assert(a.compare(b) < 0);
    assert(b.compare(a) > 0);
    assert(a.compare(c) == 0);
    assert(a == c);
    assert(a != b);
}

static void test_resize_append()
{
    String s("abc");
    s.resize(5, U'x');
    assert(s.toStdString() == "abcxx");

    s.append(3, U'y');
    assert(s.toStdString() == "abcxxyyy");
}

static void test_std_interop()
{
    std::string stds = "hello";
    String s(stds);
    assert(s.toStdString() == "hello");

    s += " world";
    assert(s.toStdString() == "hello world");
}

int main()
{
    test_ascii_sso();
    test_ascii_heap();
    test_utf8_decode();
    test_concat_modes();
    test_substr_erase_insert();
    test_replace();
    test_find_rfind();
    test_compare();
    test_resize_append();
    test_std_interop();

    std::cout << "All String tests passed.\n";
}
