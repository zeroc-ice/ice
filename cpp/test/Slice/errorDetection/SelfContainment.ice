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

struct x
{
    int i;
    x j;	// Error
};

// TODO: The following should cause an error, but doesn't. Need to add a check for that.
#if 0
class y
{
    int i;
    y j;	// Error
};
#endif
