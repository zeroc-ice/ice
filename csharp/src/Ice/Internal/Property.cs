// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice.Internal;

internal sealed record class PropertyArray(string name, bool prefixOnly, bool isOptIn, Property[] properties);

internal sealed record class Property(
    string pattern,
    bool usesRegex,
    string defaultValue,
    bool deprecated,
    PropertyArray? propertyArray);
