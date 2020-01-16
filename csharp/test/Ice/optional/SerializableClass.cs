//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

namespace Ice.optional.Test
{
    [Serializable]
    public class SerializableClass
    {
        public SerializableClass(int v) => _v = v;

        public override bool Equals(object obj) =>
            obj is SerializableClass && _v == (obj as SerializableClass)._v;

        public override int GetHashCode() => base.GetHashCode();

        private int _v;
    }

}
