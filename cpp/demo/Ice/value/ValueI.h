// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


#ifndef VALUE_I_H
#define VALUE_I_H

#include <Value.h>

class InitialI : public Initial
{
public:

    InitialI(::Ice::ObjectAdapterPtr);

    virtual SimplePtr getSimple(const Ice::Current&);
    virtual ::Ice::ObjectPtr getPrinterAsObject(const Ice::Current&);
    virtual void getPrinter(PrinterPtr&, PrinterPrx&, const Ice::Current&);
    virtual PrinterPtr getDerivedPrinter(const Ice::Current&);
    virtual void throwDerivedPrinter(const Ice::Current&);

private:

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
