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

#ifndef VALUE_I_H
#define VALUE_I_H

#include <Value.h>

class InitialI : public Initial
{
public:

    InitialI(const Ice::ObjectAdapterPtr&);

    virtual SimplePtr getSimple(const Ice::Current&);
    virtual ::Ice::ObjectPtr getPrinterAsObject(const Ice::Current&);
    virtual void getPrinter(PrinterPtr&, PrinterPrx&, const Ice::Current&);
    virtual PrinterPtr getDerivedPrinter(const Ice::Current&);
    virtual void throwDerivedPrinter(const Ice::Current&);

private:

    Ice::ObjectAdapterPtr _adapter;
    SimplePtr _simple;
    PrinterPtr _printer;
    PrinterPrx _printerProxy;
    DerivedPrinterPtr _derivedPrinter;
};

class PrinterI : virtual public Printer
{
public:

    virtual void printBackwards(const Ice::Current&);
};

class DerivedPrinterI : virtual public DerivedPrinter, virtual public PrinterI
{
public:

    virtual void printUppercase(const Ice::Current&);
};

#endif
