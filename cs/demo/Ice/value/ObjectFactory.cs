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

using System.Diagnostics;

class ObjectFactory : Ice.LocalObjectImpl, Ice.ObjectFactory
{
    public virtual Ice.Object create(string type)
    {
        if(type.Equals("::Printer"))
        {
            return new PrinterI();
        }
        
        if(type.Equals("::DerivedPrinter"))
        {
            return new DerivedPrinterI();
        }
        
        Debug.Assert(false);
        return null;
    }
    
    public virtual void  destroy()
    {
        // Nothing to do
    }
}
