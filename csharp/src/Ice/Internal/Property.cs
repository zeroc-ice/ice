// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice.Internal;

public sealed record class PropertyArray(
    string name,
    Property[] properties);

public sealed record class Property(
    string pattern,
    bool usesRegex,
    string defaultValue,
    bool deprecated,
    PropertyArray? propertyClass,
    bool prefixOnly);
