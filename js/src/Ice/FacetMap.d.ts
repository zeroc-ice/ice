//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

export namespace Ice
{
    /**
     * A mapping from facet name to servant.
     */
    class FacetMap extends Map<string, Ice.Object>
    {
    }

    class FacetMapHelper
    {
        static write(outs:OutputStream, value:FacetMap):void;
        static read(ins:InputStream):FacetMap;
    }
}
