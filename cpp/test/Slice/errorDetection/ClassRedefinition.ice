// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************



module Test
{

interface Foo { long op(); };
class Class1 implements Foo { byte b; long l; };
class Class1;
class Class1 implements Foo { long l; };

local class Class2 implements Foo { byte b; };
local class Class2;
local class Class2 implements Foo { byte b; };

};
