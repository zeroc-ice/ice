// **********************************************************************
//
// Copyright (c) 2002
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
