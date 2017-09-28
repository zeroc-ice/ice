%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef (Sealed) InitializationData
    methods
        function r = clone(obj)
            r = Ice.InitializationData();
            r.properties_ = obj.properties_;
            r.compactIdResolver = obj.compactIdResolver;
            r.valueFactoryManager = obj.valueFactoryManager;
        end
    end
    properties
        properties_
        compactIdResolver
        valueFactoryManager
    end
end
