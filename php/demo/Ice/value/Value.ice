// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

module Demo
{

class Simple
{
    string message;
};

["preserve-slice"]
class Printer
{
    string message;
    void printBackwards();
};

["cpp:virtual"]class DerivedPrinter extends Printer
{
    string derivedMessage;
    void printUppercase();
};

["cpp:virtual"]class ClientPrinter extends Printer
{
};

exception DerivedPrinterException
{
    DerivedPrinter derived;
};

interface Initial
{
    Simple getSimple();
    void getPrinter(out Printer impl, out Printer* proxy);
    ["format:sliced"] Printer getDerivedPrinter();
    ["format:sliced"] Printer updatePrinterMessage(Printer impl);
    void throwDerivedPrinter() throws DerivedPrinterException;
    void shutdown();
};

};

