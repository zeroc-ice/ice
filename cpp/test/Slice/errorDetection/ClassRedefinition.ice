// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


interface Foo { long op(); };
class Class1 implements Foo { byte b; };
class Class1 implements Foo { long l; };
class Class1;

local class Class2 implements Foo { byte b; };
local class Class2 implements Foo { byte b; };
local class Class2;
