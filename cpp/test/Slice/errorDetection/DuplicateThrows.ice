// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************






exception e {};
exception e2 {};
interface i2 { void op() throws ::e, e; };
interface i3 { void op() throws e, ::e; };
interface i4 { void op() throws e2, e, e2; };
interface i5 { void op() throws e2, e, e2, ::e; };
