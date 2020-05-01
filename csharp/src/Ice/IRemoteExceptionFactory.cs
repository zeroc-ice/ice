//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    public interface IRemoteExceptionFactory
    {
        RemoteException Read(InputStream istr);
    }
}
