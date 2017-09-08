%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef Properties < IceInternal.WrapperObject
    methods
        function obj = Properties(impl)
            obj = obj@IceInternal.WrapperObject(impl);
        end
        function r = getProperty(obj, key)
            r = Ice.Util.callMethodWithResult(obj, 'getProperty', key);
        end
        function r = getPropertyWithDefault(obj, key, def)
            r = Ice.Util.callMethodWithResult(obj, 'getPropertyWithDefault', key, def);
        end
        function r = getPropertyAsInt(obj, key)
            v = libpointer('int32Ptr', 0);
            Ice.Util.callMethod(obj, 'getPropertyAsInt', key, v);
            r = v.Value;
        end
        function r = getPropertyAsIntWithDefault(obj, key, def)
            v = libpointer('int32Ptr', 0);
            Ice.Util.callMethod(obj, 'getPropertyAsIntWithDefault', key, def, v);
            r = v.Value;
        end
        function r = getPropertyAsList(obj, key)
            r = Ice.Util.callMethodWithResult(obj, 'getPropertyAsList', key);
        end
        function r = getPropertyAsListWithDefault(obj, key, def)
            r = Ice.Util.callMethodWithResult(obj, 'getPropertyAsListWithDefault', key, def);
        end
        function r = getPropertiesForPrefix(obj, prefix)
            r = Ice.Util.callMethodWithResult(obj, 'getPropertiesForPrefix', prefix);
        end
        function setProperty(obj, key, value)
            Ice.Util.callMethod(obj, 'setProperty', key, value);
        end
        function r = getCommandLineOptions(obj)
            r = Ice.Util.callMethodWithResult(obj, 'getCommandLineOptions');
        end
        function r = parseCommandLineOptions(obj, prefix, options)
            r = Ice.Util.callMethodWithResult(obj, 'parseCommandLineOptions', prefix, options);
        end
        function r = parseIceCommandLineOptions(obj, options)
            r = Ice.Util.callMethodWithResult(obj, 'parseIceCommandLineOptions', options);
        end
        function load(obj, file)
            Ice.Util.callMethod(obj, 'load', file);
        end
        function r = clone(obj)
            impl = libpointer('voidPtr');
            Ice.Util.callMethod(obj, 'clone', impl);
            r = Ice.Properties(impl);
        end
    end
end
