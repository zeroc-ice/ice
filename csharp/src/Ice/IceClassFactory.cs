//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice
{
    /// <summary>Delegate use by InputStream to create class instances</summary>
    /// <returns>The new class instance.</returns>
    internal delegate AnyClass IceClassFactory();
}
