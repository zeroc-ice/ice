//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    /// <summary>IClassFactory is a publicly visible Ice internal interface used by InputStream and implemented by
    /// the generated code.</summary>
    public interface IClassFactory
    {
        /// <summary>Creates a new instance of the associated class and reads its fields from the stream.</summary>
        /// <param name="istr">The input stream.</param>
        /// <returns>The new class instance.</returns>
        AnyClass Read(InputStream istr);
    }
}
