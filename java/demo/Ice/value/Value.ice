// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
    Object getPrinterAsObject();
    void getPrinter(out Printer impl, out Printer* proxy);
    Printer getDerivedPrinter();
    void throwDerivedPrinter() throws DerivedPrinterException;
};

#endif
