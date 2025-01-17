// Copyright (c) ZeroC, Inc.

#pragma once

#include <Ice/BuiltinSequences.ice>
#include <Ice/Identity.ice>
#include <Ice/Version.ice>

module Test
{
    enum FlagColor { Red, White, Blue }

    struct SimpleStruct
    {
        bool myBool;
        byte myByte;
        short myShort;
        int myInt;
        long myLong;
        float myFloat;
        double myDouble;
        string myString;
        FlagColor myEnum;
    }

    struct ByteBoolStruct
    {
        Ice::ByteSeq myByteSeq;
        Ice::BoolSeq myBoolSeq;
    }

    struct CustomStruct
    {
        Ice::Identity myIdentity;
        Ice::ProtocolVersion myProtocolVersion;
        Ice::EncodingVersion myEncodingVersion;
    }

    dictionary<string, Ice::IntSeq> MyDict;

    struct DictionaryStruct
    {
        MyDict myDict;
    }

    class Person
    {
        string name;
        optional(1) int age;
    }

    class Employee;
    sequence<Employee> EmployeeSeq;

    class Employee extends Person
    {
        string title;
        Employee manager;
        EmployeeSeq directReports;
    }

    ["cpp:custom-print"]
    class Neighbor extends Person
    {
        string address;
    }
}
