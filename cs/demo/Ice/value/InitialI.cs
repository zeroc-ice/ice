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

class InitialI : Initial
{
    internal InitialI(Ice.ObjectAdapter adapter)
    {
        _simple = new Simple();
        _simple.message = "a message 4 u";
        
        _printer = new PrinterI();
        _printer.message = "Ice rulez!";
        _printerProxy = PrinterPrxHelper.uncheckedCast(adapter.addWithUUID(_printer));
        
        _derivedPrinter = new DerivedPrinterI();
        _derivedPrinter.message = _printer.message;
        _derivedPrinter.derivedMessage = "a derived message 4 u";
        adapter.addWithUUID(_derivedPrinter);
    }
    
    public override Simple getSimple(Ice.Current current)
    {
        return _simple;
    }
    
    public override Ice.Object getPrinterAsObject(Ice.Current current)
    {
        return _printer;
    }
    
    public override void getPrinter(out Printer impl, out PrinterPrx proxy, Ice.Current current)
    {
        impl = _printer;
        proxy = _printerProxy;
    }
    
    public override Printer getDerivedPrinter(Ice.Current current)
    {
        return _derivedPrinter;
    }
    
    public override void throwDerivedPrinter(Ice.Current current)
    {
        DerivedPrinterException ex = new DerivedPrinterException();
        ex.derived = _derivedPrinter;
        throw ex;
    }
    
    private Simple _simple;
    private Printer _printer;
    private PrinterPrx _printerProxy;
    private DerivedPrinter _derivedPrinter;
}
