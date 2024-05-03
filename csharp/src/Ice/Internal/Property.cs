// Copyright (c) ZeroC, Inc.

namespace Ice.Internal;

public sealed record class Property(string pattern, string defaultValue, bool deprecated, string deprecatedBy);
