//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    /// <summary>
    /// The optional format.
    ///
    /// An optional value is encoded with a specific optional format. This optional
    /// format describes how the data is encoded and how it can be skipped by the
    /// unmarshaling code if the optional is not known to the receiver.
    /// </summary>
    public enum OptionalFormat
    {
        F1 = 0,
        F2 = 1,
        F4 = 2,
        F8 = 3,
        Size = 4,
        VSize = 5,
        FSize = 6,
        Class = 7
    }
}
