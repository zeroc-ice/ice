// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

struct s1		// Illegal empty struct
{
};

struct s2
{
    long l;		// One member, OK
};

struct s4
{
    [ "Hi" ] long l;	// One member with metadata, OK
};

struct s5		// Two members, OK
{
    long l;
    byte b;
};

struct s6
{			// Two members with metadata, OK
    [ "Hi" ]	long l;
		byte b;
};

struct s7		// Two members with metadata, OK
{
    [ "Hi" ]	long l;
    [ "there" ]	byte b;
};

struct s8
{
    [ "hi" ] long ;	// Missing data member name
};

struct s9
{
    [ "there" ] long    // Missing data member name
};

struct s10
{
    long ;		// Missing data member name
};

struct s11
{
    long		// Missing data member name
};
