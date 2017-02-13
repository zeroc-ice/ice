// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

public class Property
{
    public Property(String pattern, boolean deprecated, String deprecatedBy)
    {
        _pattern = pattern;
        _deprecated = deprecated;
        _deprecatedBy = deprecatedBy;
    }

    public String
    pattern()
    {
        return _pattern;
    }

    public boolean
    deprecated()
    {
        return _deprecated;
    }

    public String
    deprecatedBy()
    {
        return _deprecatedBy;
    }

    private String _pattern;
    private boolean _deprecated;
    private String _deprecatedBy;
}
