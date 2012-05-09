// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
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

exception DerivedPrinterException
{
    DerivedPrinter derived;
};

interface Initial
{
    Simple getSimple();
    void getPrinter(out Printer impl, out Printer* proxy);
    ["format:sliced"] Printer getDerivedPrinter();
    void throwDerivedPrinter() throws DerivedPrinterException;
    void shutdown();
};

};

