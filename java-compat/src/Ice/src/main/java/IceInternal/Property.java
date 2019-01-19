//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

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
