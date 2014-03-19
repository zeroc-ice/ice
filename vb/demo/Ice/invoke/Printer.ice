// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

module InvokeDemo
{
exception PrintFailure
{
    string reason;
};

sequence<string> StringSeq;

dictionary<string, string> StringDict;

enum Color { red, green, blue };

struct Structure
{
    string name;
    Color value;
};

sequence<Structure> StructureSeq;

class C
{
    Structure s;
};

interface Printer
{
    void printString(string message);
    void printStringSequence(StringSeq seq);
    void printDictionary(StringDict dict);
    void printEnum(Color c);
    void printStruct(Structure st);
    void printStructSequence(StructureSeq seq);
    void printClass(C cls);
    C getValues(out string str);
    void throwPrintFailure() throws PrintFailure;
    void shutdown();
};
};
