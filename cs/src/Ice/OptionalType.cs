// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
    /// <summary>
    /// The optional type.
    ///
    /// An optional value is encoded with a specific optional type. This optional
    /// type describes how the data is encoded and how it can be skipped by the
    /// unmarshaling code if the optional is not known to the receiver.
    /// </summary>
    public enum OptionalType
    {
        F1 = 0,
        F2 = 1,
        F4 = 2,
        F8 = 3,
        Size = 4,
        VSize = 5,
        FSize = 6,
        EndMarker = 7
    }
}
