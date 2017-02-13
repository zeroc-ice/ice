// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************



module Test
{

interface Foo1;
class Foo1;
class Foo1 { void op(); };

interface Foo2 { void op(); };
class Foo2;

class Foo3;
interface Foo3;
interface Foo3 { void op(); };

class Foo4 { long l; };
interface Foo4;

};
