%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef FormatType < uint32
    enumeration
        DefaultFormat (0)
        CompactFormat (1)
        SlicedFormat (2)
    end
end
