// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef VALUE_I_H
#define VALUE_I_H

#include <Value.h>

class InitialI : public Initial
{
public:

    InitialI(const Ice::ObjectAdapterPtr&);

    virtual SimplePtr simple();
    virtual void printer(PrinterPtr&, PrinterPrx&);
    virtual PrinterPtr derivedPrinter();
    virtual void throwDerivedPrinter();

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

    virtual void printBackwards();
};

class DerivedPrinterI : virtual public DerivedPrinter, virtual public PrinterI
{
public:

    virtual void printUppercase();
};

#endif
