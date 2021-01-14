//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

module Test
{

exception e {}
exception e2 {}
interface i2 { void op() throws ::Test::e, e; }
interface i3 { void op() throws e, ::Test::e; }
interface i4 { void op() throws e2, e, e2; }
interface i5 { void op() throws e2, e, e2, ::Test::e; }

}
