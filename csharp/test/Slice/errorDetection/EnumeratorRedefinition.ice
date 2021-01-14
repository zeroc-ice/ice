//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

module Test
{

enum Enum1 { A, B, C }  // Ok as of 3.7
enum Enum2 { A, B, E }  // Ok as of 3.7
enum Enum3 { A, B, C, A }

}
