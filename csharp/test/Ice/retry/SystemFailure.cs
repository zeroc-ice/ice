//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice.retry
{
    [System.Serializable]
    public class SystemFailure : SystemException
    {
        public override string ice_id() => "::SystemFailure";
    }
}
