// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
