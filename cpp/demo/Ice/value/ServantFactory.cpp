// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <ServantFactory.h>
#include <ValueI.h>

using namespace std;

Ice::ObjectPtr
ServantFactory::create(const string& type)
{
    if (type == "::Printer")
    {
	return new PrinterI;
    }

    if (type == "::DerivedPrinter")
    {
	return new DerivedPrinterI;
    }

    assert(false);
    return 0;
}

void
ServantFactory::destroy()
{
    // Nothing to do
}
