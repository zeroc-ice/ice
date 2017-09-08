%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef Communicator < IceInternal.WrapperObject
    methods
        function obj = Communicator(impl)
            obj = obj@IceInternal.WrapperObject(impl);
        end
        function r = stringToProxy(obj, str)
            impl = libpointer('voidPtr');
            Ice.Util.callMethod(obj, 'stringToProxy', str, impl);
            r = Ice.ObjectPrx(impl);
        end
        function r = getProperties(obj)
            impl = libpointer('voidPtr');
            Ice.Util.callMethod(obj, 'getProperties', impl);
            r = Ice.Properties(impl);
        end
        function destroy(obj)
            Ice.Util.callMethod(obj, 'destroy');
        end
    end
end
