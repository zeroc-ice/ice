// Copyright (c) ZeroC, Inc.

namespace IceInternal;

public sealed class Property
{
    public Property(string pattern, string defaultValue, bool deprecated, string deprecatedBy)
    {
        _pattern = pattern;
        _defaultValue = defaultValue;
        _deprecated = deprecated;
        _deprecatedBy = deprecatedBy;
    }

    public string
    pattern()
    {
        return _pattern;
    }

    public string
    defaultValue()
    {
        return _defaultValue;
    }

    public bool
    deprecated()
    {
        return _deprecated;
    }

    public string
    deprecatedBy()
    {
        return _deprecatedBy;
    }

    private string _pattern;
    private string _defaultValue;
    private bool _deprecated;
    private string _deprecatedBy;
}
