// Copyright (c) ZeroC, Inc.

#pragma once

#include <Ice/BuiltinSequences.ice>
#include <Ice/Identity.ice>
#include <Ice/Version.ice>

module Test
{
    enum FlagColor { Red, White, ["cpp:identifier:OldGloryBlue"] Blue }

    ["cpp:custom-print"]
    enum Fruit { Apple, Banana, Cucumber }

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

    interface Printer
    {
        void print(string s);
    }

    struct ProxyStruct
    {
        Object* \object;
        Printer* nullPrinter;
        Printer* printer;
    }

    ["cpp:identifier:MyStruct"]
    struct RemappedIdentifierStruct
    {
        ["cpp:identifier:myString"] string s;
    }

    exception MyException
    {
        string message;
        int errorCode;
    }

    exception MyDerivedException extends MyException
    {
        int extraCode;
    }

    ["cpp:custom-print"]
    exception MyCustomPrintException
    {
        string message;
    }
}
