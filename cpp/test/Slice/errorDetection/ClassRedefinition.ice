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

class Class1 { byte b; long l; };
class Class1;
class Class1 { long l; };

class Class2 { byte b; };
class Class2;
class Class2 { byte b; };

};
