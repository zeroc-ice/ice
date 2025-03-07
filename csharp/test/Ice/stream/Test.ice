// Copyright (c) ZeroC, Inc.
    
#pragma once
    
// Suppress invalid metadata warnings which we expect this test to generate.
[["suppress-warning"]]
    
#include "Ice/BuiltinSequences.ice"
    
["cs:namespace:Ice.stream"]
module Test
{
    enum MyEnum
    {
        enum1,
        enum2,
        enum3
    }
<<<<<<< Updated upstream

    class MyClass;
    interface MyInterface;

=======
        
    class MyClass;
    interface MyInterface;
        
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
    class OptionalClass
    {
        bool bo;
        byte by;
        optional(1) short sh;
        optional(2) int i;
    }
<<<<<<< Updated upstream

    sequence<MyEnum> MyEnumS;
    sequence<MyClass> MyClassS;

=======
        
    sequence<MyEnum> MyEnumS;
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
    ["cs:generic:List"]
    sequence<bool> BoolList;
    ["cs:generic:List"]
    sequence<byte> ByteList;
    ["cs:generic:List"]
    sequence<MyEnum> MyEnumList;
    ["cs:generic:List"]
    sequence<LargeStruct> LargeStructList;
    ["cs:generic:List"]
    sequence<MyClass> MyClassList;
    ["cs:generic:List"]
    sequence<MyInterface*> MyInterfaceProxyList;
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    ["cs:generic:LinkedList"]
    sequence<short> ShortLinkedList;
    ["cs:generic:LinkedList"]
    sequence<int> IntLinkedList;
    ["cs:generic:LinkedList"]
    sequence<MyEnum> MyEnumLinkedList;
    ["cs:generic:LinkedList"]
    sequence<LargeStruct> LargeStructLinkedList;
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    ["cs:generic:Stack"]
    sequence<long> LongStack;
    ["cs:generic:Stack"]
    sequence<float> FloatStack;
    ["cs:generic:Stack"]
    sequence<LargeStruct> LargeStructStack;
    ["cs:generic:Stack"]
    sequence<MyInterface*> MyInterfaceProxyStack;
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    //
    // This will produce a warning and use the default
    // sequence mapping. The generic:Stack metadata cannot be use
    // with object sequences.
    //
    ["cs:generic:Stack"]
    sequence<Object> ObjectStack;
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    //
    // This will produce a warning and use the default
    // sequence mapping. The generic:Stack metadata cannot be use
    // with object sequences.
    //
    ["cs:generic:Stack"]
    sequence<MyClass> MyClassStack;
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    ["cs:generic:Queue"]
    sequence<double> DoubleQueue;
    ["cs:generic:Queue"]
    sequence<string> StringQueue;
    ["cs:generic:Queue"]
    sequence<LargeStruct> LargeStructQueue;
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    ["cs:generic:List"]
    sequence<Ice::StringSeq> StringSList;
    ["cs:generic:Stack"]
    sequence<Ice::StringSeq> StringSStack;
<<<<<<< Updated upstream

    ["cs:generic:SortedDictionary"]
    dictionary<string, string> SortedStringStringD;

=======
        
    ["cs:generic:SortedDictionary"]
    dictionary<string, string> SortedStringStringD;
        
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

=======
        
>>>>>>> Stashed changes
    exception MyException
    {
        MyClass c;
    }
<<<<<<< Updated upstream

=======
        
>>>>>>> Stashed changes
    interface MyInterface
    {
    }
}
