// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef VALUE_ICE
#define VALUE_ICE

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
};

#endif
