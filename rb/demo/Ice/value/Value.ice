// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef VALUE_ICE
#define VALUE_ICE

module Demo
{

class Simple
{
    string message;
};

class Printer
{
    string message;
    void printBackwards();
};

class DerivedPrinter extends Printer
{
    string derivedMessage;
    void printUppercase();
};

exception DerivedPrinterException
{
    DerivedPrinter derived;
};

class Initial
{
    Simple getSimple();
    void getPrinter(out Printer impl, out Printer* proxy);
    Printer getDerivedPrinter();
    void throwDerivedPrinter() throws DerivedPrinterException;
    void shutdown();
};

};

#endif
