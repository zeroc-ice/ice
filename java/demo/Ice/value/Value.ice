// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef VALUE_ICE
#define VALUE_ICE

class Simple
{
    string _message;
};

class Printer
{
    string _message;
    void printBackwards();
};

class DerivedPrinter extends Printer
{
    string _derivedMessage;
    void printUppercase();
};

exception DerivedPrinterException
{
    DerivedPrinter derived;
};

class Initial
{
    Simple getSimple();
    void getPrinter(; Printer impl, Printer* proxy);
    Printer getDerivedPrinter();
    void throwDerivedPrinter() throws DerivedPrinterException;
};

#endif
