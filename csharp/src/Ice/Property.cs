//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice
{
    public sealed class Property
    {
        public bool Deprecated { get; }
        public string? DeprecatedBy { get; }
        public string Pattern { get; }

        public Property(string pattern, bool deprecated, string? deprecatedBy)
        {
            Pattern = pattern;
            Deprecated = deprecated;
            DeprecatedBy = deprecatedBy;
        }
    }
}
