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


#include <Ice/Ice.h>
#include <ObjectFactory.h>
#include <ValueI.h>

using namespace std;

Ice::ObjectPtr
ObjectFactory::create(const string& type)
{
    if(type == "::Printer")
    {
	return new PrinterI;
    }

    if(type == "::DerivedPrinter")
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
