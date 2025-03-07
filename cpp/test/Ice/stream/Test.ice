// Copyright (c) ZeroC, Inc.
    
#pragma once
    
#include "Ice/BuiltinSequences.ice"
    
module Test
{
    enum MyEnum
    {
        enum1,
        enum2,
        enum3
    }
<<<<<<< Updated upstream

    interface MyInterface;
    class MyClass;

=======
        
    interface MyInterface;
    class MyClass;
        
>>>>>>> Stashed changes
    struct LargeStruct
    {
        bool bo;
        byte by;
        short sh;
        int i;
        long l;
        float f;
        double d;
        string str;
        MyEnum e;
        MyInterface* p;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    struct ClassStruct
    {
        int i;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    class OptionalClass
    {
        bool bo;
        byte by;
        optional(1) short sh;
        optional(2) int i;
    }
<<<<<<< Updated upstream

    sequence<MyEnum> MyEnumS;
    sequence<LargeStruct> LargeStructS;
    sequence<MyClass> MyClassS;

=======
        
    sequence<MyEnum> MyEnumS;
    sequence<LargeStruct> LargeStructS;
    sequence<MyClass> MyClassS;
        
>>>>>>> Stashed changes
    sequence<Ice::BoolSeq> BoolSS;
    sequence<Ice::ByteSeq> ByteSS;
    sequence<Ice::ShortSeq> ShortSS;
    sequence<Ice::IntSeq> IntSS;
    sequence<Ice::LongSeq> LongSS;
    sequence<Ice::FloatSeq> FloatSS;
    sequence<Ice::DoubleSeq> DoubleSS;
    sequence<Ice::StringSeq> StringSS;
    sequence<MyEnumS> MyEnumSS;
    sequence<LargeStructS> LargeStructSS;
    sequence<MyClassS> MyClassSS;
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    dictionary<byte, bool> ByteBoolD;
    dictionary<short, int> ShortIntD;
    dictionary<long, float> LongFloatD;
    dictionary<string, string> StringStringD;
    dictionary<string, MyClass> StringMyClassD;
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    class MyClass
    {
        MyClass c;
        Object o;
        LargeStruct s;
        Ice::BoolSeq seq1;
        Ice::ByteSeq seq2;
        Ice::ShortSeq seq3;
        Ice::IntSeq seq4;
        Ice::LongSeq seq5;
        Ice::FloatSeq seq6;
        Ice::DoubleSeq seq7;
        Ice::StringSeq seq8;
        MyEnumS seq9;
        MyClassS seq10;
        StringMyClassD d;
    }
<<<<<<< Updated upstream

    interface MyInterface
    {
    }

=======
        
    interface MyInterface
    {
    }
        
>>>>>>> Stashed changes
    exception MyException
    {
        MyClass c;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    module Sub
    {
        enum NestedEnum
        {
            nestedEnum1,
            nestedEnum2,
            nestedEnum3
        }
<<<<<<< Updated upstream

=======
            
>>>>>>> Stashed changes
        struct NestedStruct
        {
            bool bo;
            byte by;
            short sh;
            int i;
            long l;
            float f;
            double d;
            string str;
            NestedEnum e;
        }
<<<<<<< Updated upstream

=======
            
>>>>>>> Stashed changes
        exception NestedException
        {
            string str;
        }
    }
}
<<<<<<< Updated upstream

=======
    
>>>>>>> Stashed changes
module Test2::Sub2
{
    enum NestedEnum2
    {
        nestedEnum4,
        nestedEnum5,
        nestedEnum6
    }
        
    struct NestedStruct2
    {
        bool bo;
        byte by;
        short sh;
        int i;
        long l;
        float f;
        double d;
        string str;
        NestedEnum2 e;
    }
        
    exception NestedException2
    {
        string str;
    }
}
