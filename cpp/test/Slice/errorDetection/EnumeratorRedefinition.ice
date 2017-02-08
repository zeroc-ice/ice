// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************



module Test
{

enum Enum1 { A, B, C };  // Ok as of 3.7
enum Enum2 { A, B, E };  // Ok as of 3.7
enum Enum3 { A, B, C, A };

};
