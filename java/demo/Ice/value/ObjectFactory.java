// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
