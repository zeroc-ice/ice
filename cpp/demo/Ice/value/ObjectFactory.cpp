// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <ObjectFactory.h>
#include <ValueI.h>

using namespace std;

Ice::ObjectPtr
ObjectFactory::create(const string& type)
{
    if(type == Demo::Printer::ice_staticId())
    {
        return new PrinterI;
    }

    if(type == Demo::DerivedPrinter::ice_staticId())
    {
        return new DerivedPrinterI;
    }

    if(type == Demo::ClientPrinter::ice_staticId())
    {
        return new ClientPrinterI;
    }

    assert(false);
    return 0;
}

void
ObjectFactory::destroy()
{
    // Nothing to do
}
