//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

module Test
{

interface Foo1;
class Foo1;
class Foo1 {  int x; }

interface Foo2 { void op(); }
class Foo2;

class Foo3;
interface Foo3;
interface Foo3 { void op(); }

class Foo4 { long l; }
interface Foo4;

}
