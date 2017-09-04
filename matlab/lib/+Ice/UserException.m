%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef (Abstract) UserException < Ice.Exception
    methods
        function obj = UserException(id, msg)
            obj = obj@Ice.Exception(id, msg)
        end
        function self = read_(self, is)
            is.startException();
            self = self._readImpl(is);
            is.endException();
        end
        function r = usesClasses_(self)
            r = false
        end
    end
    methods(Abstract)
        self = readImpl_(self, is)
    end
end
