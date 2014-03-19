// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef VALUE_I_H
#define VALUE_I_H

#include <Value.h>

class InitialI : public Demo::Initial
{
public:

    InitialI(const Ice::ObjectAdapterPtr&);

    virtual Demo::SimplePtr getSimple(const Ice::Current&);
    virtual void getPrinter(::Demo::PrinterPtr&, Demo::PrinterPrx&, const Ice::Current&);
    virtual Demo::PrinterPtr getDerivedPrinter(const Ice::Current&);
    virtual Demo::PrinterPtr updatePrinterMessage(const Demo::PrinterPtr&, const Ice::Current&);
    virtual void throwDerivedPrinter(const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

private:

    const Demo::SimplePtr _simple;
    const Demo::PrinterPtr _printer;
    const Demo::PrinterPrx _printerProxy;
    const Demo::DerivedPrinterPtr _derivedPrinter;
};

class PrinterI : virtual public Demo::Printer
{
public:

    virtual void printBackwards(const Ice::Current&);
};

class DerivedPrinterI : virtual public Demo::DerivedPrinter, virtual public PrinterI
{
public:

    virtual void printUppercase(const Ice::Current&);
};

class ClientPrinterI : virtual public Demo::ClientPrinter, virtual public PrinterI
{
};

#endif
