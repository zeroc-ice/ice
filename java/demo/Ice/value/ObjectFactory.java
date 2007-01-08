// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

class ObjectFactory extends Ice.LocalObjectImpl implements Ice.ObjectFactory
{
    public Ice.Object
    create(String type)
    {
        if(type.equals("::Demo::Printer"))
        {
            return new PrinterI();
        }

        if(type.equals("::Demo::DerivedPrinter"))
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
