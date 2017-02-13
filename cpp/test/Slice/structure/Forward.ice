// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


//
// Ensure that a struct that use a fordward declared proxy
// compiles event if the Slice file does not contains classes
// or interface definitions.
//
module Test
{

interface Forward;

struct Init
{
	Forward* member;
	int x = 0;
};

};
