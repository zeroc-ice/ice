// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************






module Test
{

interface Intf1;
local interface Intf1;
local interface Intf1 { void op(); }

interface Intf2 { void op(); }
local interface Intf2;

local interface Intf3;
interface Intf3;
interface Intf3 { void op(); }

local interface Intf4 { void op(); }
interface Intf4;

interface Class1;
local interface Class1;
local interface Class1 { void op(); }

interface Class2 { void op(); }
local interface Class2;

local interface Class3;
interface Class3;
interface Class3 { void op(); }

local interface Class4 { void op(); }
interface Class4;

}
