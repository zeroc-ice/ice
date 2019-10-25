//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
namespace IceSSL
{
    [global::System.Serializable]
    public abstract partial class EndpointInfo : global::Ice.EndpointInfo
    {
        partial void ice_initialize();

        protected EndpointInfo() : base()
        {
            ice_initialize();
        }

        protected EndpointInfo(global::Ice.EndpointInfo underlying, int timeout, bool compress) :
            base(underlying, timeout, compress)
        {
            ice_initialize();
        }
    }
}
