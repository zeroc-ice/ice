// **********************************************************************
//
// Copyright (c) 2003
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

class ObjectFactory extends Ice.LocalObjectImpl implements Ice.ObjectFactory
{
    public Ice.Object
    create(String type)
    {
        if(type.equals("::Printer"))
        {
            return new PrinterI();
        }

        if(type.equals("::DerivedPrinter"))
        {
            return new DerivedPrinterI();
        }

        assert(false);
        return null;
    }

    public void
    destroy()
    {
        // Nothing to do
    }
}
