//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    public sealed class Property
    {
        public Property(string pattern, bool deprecated, string? deprecatedBy)
        {
            _pattern = pattern;
            _deprecated = deprecated;
            _deprecatedBy = deprecatedBy;
        }

        public string Pattern() => _pattern;

        public bool Deprecated() => _deprecated;

        public string? DeprecatedBy() => _deprecatedBy;

        private readonly string _pattern;
        private readonly bool _deprecated;
        private readonly string? _deprecatedBy;
    }
}
