// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

exception e {};
exception e2 {};
interface i2 { void op() throws ::e, e; };
interface i3 { void op() throws e, ::e; };
interface i4 { void op() throws e2, e, e2; };
interface i5 { void op() throws e2, e, e2, ::e; };
