//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;
using System.Runtime.Serialization;

namespace IceInternal
{
    public class RetryException : Exception
    {
        internal RetryException(Exception innerException)
            : base("", innerException)
        {
        }
    }
}
