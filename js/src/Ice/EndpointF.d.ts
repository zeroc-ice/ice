//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

export namespace Ice
{
    /**
     * A sequence of endpoints.
     */
    type EndpointSeq = Ice.Endpoint[];

    class EndpointSeqHelper
    {
        static write(outs:OutputStream, value:EndpointSeq):void;
        static read(ins:InputStream):EndpointSeq;
    }
}
