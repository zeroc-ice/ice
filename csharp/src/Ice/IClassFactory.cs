//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    /// <summary>IClassFactory is a publicly visible Ice internal interface used by InputStream and implemented by
    /// the generated code.</summary>
    public interface IClassFactory
    {
        /// <summary>Creates a default instance of the associated class (mapped from a Slice class). A default instance
        /// is constructed using the parameterless constructor of the class.</summary>
        /// <returns>A new instance of the class.</returns>
        AnyClass CreateDefaultInstance();
    }
}
