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
Factory::create(const string& id)
{
    if (id == "::Printer")
	return new PrinterI;

    if (id == "::DerivedPrinter")
	return new DerivedPrinterI;

    assert(false);
    return 0;
}
