// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
