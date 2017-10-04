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
            if ~isa(impl, 'lib.pointer')
                throw(MException('Ice:ArgumentException', 'invalid argument'));
            end
            obj = obj@IceInternal.WrapperObject(impl);
        end
        function r = getProperty(obj, key)
            r = obj.iceCallWithResult('getProperty', key);
        end
        function r = getPropertyWithDefault(obj, key, def)
            r = obj.iceCallWithResult('getPropertyWithDefault', key, def);
        end
        function r = getPropertyAsInt(obj, key)
            v = libpointer('int32Ptr', 0);
            obj.iceCall('getPropertyAsInt', key, v);
            r = v.Value;
        end
        function r = getPropertyAsIntWithDefault(obj, key, def)
            v = libpointer('int32Ptr', 0);
            obj.iceCall('getPropertyAsIntWithDefault', key, def, v);
            r = v.Value;
        end
        function r = getPropertyAsList(obj, key)
            r = obj.iceCallWithResult('getPropertyAsList', key);
        end
        function r = getPropertyAsListWithDefault(obj, key, def)
            r = obj.iceCallWithResult('getPropertyAsListWithDefault', key, def);
        end
        function r = getPropertiesForPrefix(obj, prefix)
            r = obj.iceCallWithResult('getPropertiesForPrefix', prefix);
        end
        function setProperty(obj, key, value)
            obj.iceCall('setProperty', key, value);
        end
        function r = getCommandLineOptions(obj)
            r = obj.iceCallWithResult('getCommandLineOptions');
        end
        function r = parseCommandLineOptions(obj, prefix, options)
            r = obj.iceCallWithResult('parseCommandLineOptions', prefix, options);
        end
        function r = parseIceCommandLineOptions(obj, options)
            r = obj.iceCallWithResult('parseIceCommandLineOptions', options);
        end
        function load(obj, file)
            obj.iceCall('load', file);
        end
        function r = clone(obj)
            impl = libpointer('voidPtr');
            obj.iceCall('clone', impl);
            r = Ice.Properties(impl);
        end
    end
end
