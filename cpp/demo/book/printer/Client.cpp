// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Printer.h>

using namespace std;
using namespace Demo;

int
main(int argc, char * argv[])
{
    int status = 0;
    Ice::CommunicatorPtr ic;
    try
    {
        ic = Ice::initialize(argc, argv);
        Ice::ObjectPrx base = ic->stringToProxy("SimplePrinter:default -h localhost -p 10000");
        PrinterPrx printer = PrinterPrx::checkedCast(base);
        if(!printer)
        {
            throw "Invalid proxy";
        }

        printer->printString("Hello World!");
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        status = 1;
    }
    catch(const char* msg)
    {
        cerr << msg << endl;
        status = 1;
    }
    if(ic)
    {
        try
        {
            ic->destroy();
        }
        catch (const Ice::Exception& ex)
        {
            cerr << ex << endl;
            status = 1;
        }
    }
    return status;
}
