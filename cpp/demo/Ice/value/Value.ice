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
    Simple simple();
    void printer(out Printer impl, out Printer* proxy);
    Printer derivedPrinter();
    void throwDerivedPrinter() throws DerivedPrinterException;
};

#endif
