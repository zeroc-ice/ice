// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;

class InitialI : InitialDisp_
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
    
    public override void getPrinter(out Printer impl, out PrinterPrx proxy, Ice.Current current)
    {
        impl = _printer;
        proxy = _printerProxy;
    }
    
    public override Printer getDerivedPrinter(Ice.Current current)
    {
        return _derivedPrinter;
    }
    
    public override Printer updatePrinterMessage(Printer printer, Ice.Current current)
    {
	printer.message = "a modified message 4 u";
        return printer;
    }

    public override void throwDerivedPrinter(Ice.Current current)
    {
        DerivedPrinterException ex = new DerivedPrinterException();
        ex.derived = _derivedPrinter;
        throw ex;
    }
    
    public override void shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
    
    private Simple _simple;
    private Printer _printer;
    private PrinterPrx _printerProxy;
    private DerivedPrinter _derivedPrinter;
}
