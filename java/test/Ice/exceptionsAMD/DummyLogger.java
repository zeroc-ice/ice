// **********************************************************************
//
// Copyright (c) 2003
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

public final class DummyLogger extends Ice.LocalObjectImpl implements Ice.Logger
{
    public void
    trace(String category, String message)
    {
    }

    public void
    warning(String message)
    {
    }

    public void
    error(String message)
    {
    }
}
