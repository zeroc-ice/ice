// Copyright (c) ZeroC, Inc.

namespace Ice.Internal;

public sealed record class Property(string pattern, bool usesRegex, string defaultValue, bool deprecated);
