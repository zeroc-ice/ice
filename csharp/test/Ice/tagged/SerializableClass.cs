//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

namespace ZeroC.Ice.Test.Tagged
{
    [Serializable]
    public class SerializableClass
    {
        private readonly int _v;

        public SerializableClass(int v) => _v = v;

        public override bool Equals(object? obj) => obj is SerializableClass value && _v == value._v;

        public override int GetHashCode() => base.GetHashCode();
    }

}
