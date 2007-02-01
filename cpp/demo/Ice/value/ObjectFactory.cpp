// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
    if(type == "::Demo::Printer")
    {
        return new PrinterI;
    }

    if(type == "::Demo::DerivedPrinter")
    {
        return new DerivedPrinterI;
    }

    assert(false);
    return 0;
}

void
ObjectFactory::destroy()
{
    // Nothing to do
}
