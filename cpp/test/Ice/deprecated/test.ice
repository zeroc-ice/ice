//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

module Test
{
    ["deprecated:don't use 'MyConst'"]
    const int MyConst = 79;

    ["deprecated:don't use 'MyEnum'"]
    enum MyEnum
    {
        Foo,
        Bar,
        Baz
    }

    ["deprecated:don't use 'MyStruct'"]
    struct MyStruct
    {
        int a;
        ["deprecated"] int b;
    }

    ["deprecated:don't use 'MyException'"]
    exception MyException
    {
        bool a;
        ["deprecated"] bool b;
    }

    ["deprecated:don't use 'MyClass'"]
    class MyClass
    {
        float a;
        ["deprecated"] float b;
    }

    class MyDerivedClass : MyClass
    {
        MyStruct ms;
    }

    ["deprecated:don't use MyService"]
    interface MyService
    {
        ["deprecated:don't call 'op1'"] void op1();

        MyStruct op2(MyClass mc);

        MyEnum op3(optional(MyConst) int i) throws MyException;
    }
}
