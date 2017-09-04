%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef Communicator < Ice.WrapperObject
    methods
        function self = Communicator(impl)
            self = self@Ice.WrapperObject(impl);
        end
        function r = stringToProxy(self, str)
            impl = libpointer('voidPtr');
            Ice.Util.callMethod(self, 'stringToProxy', str, impl);
            r = Ice.ObjectPrx(impl);
        end
        function r = getProperties(self)
            impl = libpointer('voidPtr');
            Ice.Util.callMethod(self, 'getProperties', impl);
            r = Ice.Properties(impl);
        end
        function destroy(self)
            Ice.Util.callMethod(self, 'destroy');
        end
    end
end
