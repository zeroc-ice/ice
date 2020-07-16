//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

namespace ZeroC.Ice
{
    public interface IPlugin : IAsyncDisposable
    {
        /// <summary>Perform any necessary initialization steps.</summary>
        void Initialize();
    }
}
