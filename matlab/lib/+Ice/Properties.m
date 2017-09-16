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
            r = obj.callWithResult_('getProperty', key);
        end
        function r = getPropertyWithDefault(obj, key, def)
            r = obj.callWithResult_('getPropertyWithDefault', key, def);
        end
        function r = getPropertyAsInt(obj, key)
            v = libpointer('int32Ptr', 0);
            obj.call_('getPropertyAsInt', key, v);
            r = v.Value;
        end
        function r = getPropertyAsIntWithDefault(obj, key, def)
            v = libpointer('int32Ptr', 0);
            obj.call_('getPropertyAsIntWithDefault', key, def, v);
            r = v.Value;
        end
        function r = getPropertyAsList(obj, key)
            r = obj.callWithResult_('getPropertyAsList', key);
        end
        function r = getPropertyAsListWithDefault(obj, key, def)
            r = obj.callWithResult_('getPropertyAsListWithDefault', key, def);
        end
        function r = getPropertiesForPrefix(obj, prefix)
            r = obj.callWithResult_('getPropertiesForPrefix', prefix);
        end
        function setProperty(obj, key, value)
            obj.call_('setProperty', key, value);
        end
        function r = getCommandLineOptions(obj)
            r = obj.callWithResult_('getCommandLineOptions');
        end
        function r = parseCommandLineOptions(obj, prefix, options)
            r = obj.callWithResult_('parseCommandLineOptions', prefix, options);
        end
        function r = parseIceCommandLineOptions(obj, options)
            r = obj.callWithResult_('parseIceCommandLineOptions', options);
        end
        function load(obj, file)
            obj.call_('load', file);
        end
        function r = clone(obj)
            impl = libpointer('voidPtr');
            obj.call_('clone', impl);
            r = Ice.Properties(impl);
        end
    end
end
