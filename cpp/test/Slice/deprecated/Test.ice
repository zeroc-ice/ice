// Copyright (c) ZeroC, Inc.

[["suppress-warning:deprecated"]]

module DeprecatedTest
{
    // Applies the deprecated metadata on various Slice definitions.

    ["deprecated"] const int MyIntConst = 42;

    ["deprecated:Use NewStringSeq instead."] sequence<string> StringSeq;
    ["deprecated"] dictionary<string, string> StringDict;

    ["deprecated"] class MyClass
    {
        int i;
    }

    class MyOtherClass
    {
        ["deprecated"] int i;
    }

    ["deprecated"] class MyForwardClass;

    ["deprecated"] struct MyStruct
    {
        string name;
    }

    ["deprecated"] exception MyException
    {
        string reason;
    }

    exception MyOtherException
    {
        ["deprecated"] string reason;
    }

    ["deprecated"] interface MyInterface
    {
        void opInt(int i);
    }

    ["deprecated"] enum MyEnum { A, B, C }

    enum MyOtherEnum
    {
        A,
        ["deprecated:Use A instead."] B,
        C
    }

    interface MyOtherInterface
    {
        void opClass(MyClass c);

        void opInt(int x);

        ["deprecated:Use opInt() instead."] void opLong(long x);

        void opProxy(MyInterface* proxy) throws MyException, MyOtherException;

        MyStruct opStruct(MyStruct s);
    }

    ["deprecated"] interface MyForwardInterface;
}
