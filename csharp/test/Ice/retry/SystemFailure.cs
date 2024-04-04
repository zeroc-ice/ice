// Copyright (c) ZeroC, Inc.

namespace Ice
{
    namespace retry
    {
        public class SystemFailure : Ice.SystemException
        {
            public override string
            ice_id()
            {
                return "::SystemFailure";
            }
        }
    }
}
