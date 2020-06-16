//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice
{
    /// <summary>IClassFactory is a publicly visible Ice internal interface used by InputStream and implemented by
    /// the generated code.</summary>
    public interface IClassFactory
    {
        /// <summary>Creates a new instance of the associated class.</summary>
        /// <returns>The new class instance.</returns>
        AnyClass Create();
    }
}
