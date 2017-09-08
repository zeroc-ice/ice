%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

%
% A dictionary<struct-type, Value-type> is mapped to a StructArrayHandle object. This is necessary to support
% the staged nature of Value unmarshaling.
%
classdef StructArrayHandle < handle
    properties
        array
    end
end
