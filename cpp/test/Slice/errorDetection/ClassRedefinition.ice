// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

interface Foo { long op(); };
class Class1 implements Foo { byte b; };
class Class1 implements Foo { long l; };
class Class1;

local class Class2 implements Foo { byte b; };
local class Class2 implements Foo { byte b; };
local class Class2;
