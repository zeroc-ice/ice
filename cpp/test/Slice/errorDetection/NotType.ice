// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

module Module1 { };
module Module2 { };
module Module3 { };
module Module
{

vector<Module> Vector;
interface BarIntf extends Module { };
class BarClass1 extends Module { };
class BarClass2 implements Module1, Module2, Module3 { };
class BarClass3 extends Module implements Module1, Module2, Module3 { };

};
