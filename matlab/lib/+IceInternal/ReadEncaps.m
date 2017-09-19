%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef ReadEncaps < handle
    properties
        start
        sz
        encoding
        encoding_1_0
        decoder
        next
    end
    methods
        function reset(obj)
            obj.decoder = [];
        end
        function setEncoding(obj, encoding)
            obj.encoding = encoding;
            obj.encoding_1_0 = encoding == IceInternal.Protocol.Encoding_1_0;
        end
    end
end
