//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

module Test
{

exception Base {}
exception Base { int i; }
exception Derived extends Base { int j; }
exception Derived { int j; }

}
