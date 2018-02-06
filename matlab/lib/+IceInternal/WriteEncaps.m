%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef WriteEncaps < handle
    properties
        start
        format = Ice.FormatType.DefaultFormat
        encoding
        encoder
        next
    end
end
