// **********************************************************************
//
// Copyright (c) 2001
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

interface Foo { long op(); };
class Class1 implements Foo { byte b; };
class Class1 implements Foo { long l; };
class Class1;

local class Class2 implements Foo { byte b; };
local class Class2 implements Foo { byte b; };
local class Class2;
