// Copyright (c) ZeroC, Inc.

#pragma once

module Test
{
    struct MyStruct
    {
        int v;
    }

    dictionary<string, MyStruct> MyStructMap;
    sequence<MyStruct> MyStructSeq;

    class MyClass
    {
        MyStruct s;
    }

    dictionary<string, MyClass> MyClassMap;
    sequence<MyClass> MyClassSeq;

    enum E1
    {
        v1,
        v2,
        v3
    }

    struct S1
    {
        string s;
    }

    class C1
    {
        string s;
    }

    class C2
    {
        E1 E1;
        S1 S1;
        C1 C1;
    }

    interface I
    {
        MyClass opMyClass(MyClass s1, out MyClass s2);
        MyClassSeq opMyClassSeq(MyClassSeq s1, out MyClassSeq s2);
        MyClassMap opMyClassMap(MyClassMap s1, out MyClassMap s2);

        MyClass opMyClass(MyClass c1, out MyClass c2);
        MyClassSeq opMyClassSeq(MyClassSeq s1, out MyClassSeq s2);
        MyClassMap opMyClassMap(MyClassMap c1, out MyClassMap c2);

        E1 opE1(E1 E1);
        S1 opS1(S1 S1);
        C1 opC1(C1 C1);

        void shutdown();
    }

    dictionary<string, I*> IMap;
    sequence<I*> ISeq;

    module Inner
    {
        struct MyClass
        {
            int v;
        }

        module Inner2
        {
            struct MyStruct
            {
                int v;
            }

            dictionary<string, MyStruct> MyStructMap;
            sequence<MyStruct> MyStructSeq;

            class MyClass
            {
                MyStruct s;
            }

            dictionary<string, MyClass> MyClassMap;
            sequence<MyClass> MyClassSeq;

            interface I
            {
                MyStruct opMyStruct(MyStruct s1, out MyStruct s2);
                MyStructSeq opMyStructSeq(MyStructSeq s1, out MyStructSeq s2);
                MyStructMap opMyStructMap(MyStructMap s1, out MyStructMap s2);

                MyClass opMyClass(MyClass c1, out MyClass c2);
                MyClassSeq opMyClassSeq(MyClassSeq c1, out MyClassSeq c2);
                MyClassMap opMyClassMap(MyClassMap c1, out MyClassMap c2);

                void shutdown();
            }

            dictionary<string, I*> IMap;
            sequence<I*> ISeq;
        }

        class MyClass
        {
            MyStruct s;
        }

        sequence<Inner2::MyStruct> MyStructSeq;
        dictionary<string, Inner2::MyStruct> MyStructMap;

        dictionary<string, Inner2::MyClass> MyClassMap;
        sequence<Inner2::MyClass> MyClassSeq;

        interface I
        {
            Inner2::MyStruct opMyStruct(Inner2::MyStruct s1, out Inner2::MyStruct s2);
            Inner2::MyStructSeq opMyStructSeq(Inner2::MyStructSeq s1, out Inner2::MyStructSeq s2);
            Inner2::MyStructMap opMyStructMap(Inner2::MyStructMap s1, out Inner2::MyStructMap s2);

            Inner2::MyClass opMyClass(Inner2::MyClass c1, out Inner2::MyClass c2);
            Inner2::MyClassSeq opMyClassSeq(Inner2::MyClassSeq c1, out Inner2::MyClassSeq c2);
            Inner2::MyClassMap opMyClassMap(Inner2::MyClassMap c1, out Inner2::MyClassMap c2);

            void shutdown();
        }

        dictionary<string, I*> IMap;
        sequence<I*> ISeq;
    }
}

module Inner::Test::Inner2
{
    interface I
    {
        Test::MyStruct opMyStruct(Test::MyStruct s1, out Test::MyStruct s2);
        Test::MyStructSeq opMyStructSeq(Test::MyStructSeq s1, out Test::MyStructSeq s2);
        Test::MyStructMap opMyStructMap(Test::MyStructMap s1, out Test::MyStructMap s2);

        Test::MyClass opMyClass(Test::MyClass c1, out Test::MyClass c2);
        Test::MyClassSeq opMyClassSeq(Test::MyClassSeq c1, out Test::MyClassSeq c2);
        Test::MyClassMap opMyClassMap(Test::MyClassMap c1, out Test::MyClassMap c2);

        void shutdown();
    }
}
