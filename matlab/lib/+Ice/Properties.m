%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef Properties < Ice.WrapperObject
    methods
        function self = Properties(impl)
            self = self@Ice.WrapperObject(impl);
        end
        function r = getProperty(self, key)
            r = Ice.Util.callMethodWithResult(self, 'getProperty', key);
        end
        function r = getPropertyWithDefault(self, key, def)
            r = Ice.Util.callMethodWithResult(self, 'getPropertyWithDefault', key, def);
        end
        function r = getPropertyAsInt(self, key)
            v = libpointer('int32Ptr', 0);
            Ice.Util.callMethod(self, 'getPropertyAsInt', key, v);
            r = v.Value;
        end
        function r = getPropertyAsIntWithDefault(self, key, def)
            v = libpointer('int32Ptr', 0);
            Ice.Util.callMethod(self, 'getPropertyAsIntWithDefault', key, def, v);
            r = v.Value;
        end
        function r = getPropertyAsList(self, key)
            r = Ice.Util.callMethodWithResult(self, 'getPropertyAsList', key);
        end
        function r = getPropertyAsListWithDefault(self, key, def)
            r = Ice.Util.callMethodWithResult(self, 'getPropertyAsListWithDefault', key, def);
        end
        function r = getPropertiesForPrefix(self, prefix)
            r = Ice.Util.callMethodWithResult(self, 'getPropertiesForPrefix', prefix);
        end
        function setProperty(self, key, value)
            Ice.Util.callMethod(self, 'setProperty', key, value);
        end
        function r = getCommandLineOptions(self)
            r = Ice.Util.callMethodWithResult(self, 'getCommandLineOptions');
        end
        function r = parseCommandLineOptions(self, prefix, options)
            r = Ice.Util.callMethodWithResult(self, 'parseCommandLineOptions', prefix, options);
        end
        function r = parseIceCommandLineOptions(self, options)
            r = Ice.Util.callMethodWithResult(self, 'parseIceCommandLineOptions', options);
        end
        function load(self, file)
            Ice.Util.callMethod(self, 'load', file);
        end
        function r = clone(self)
            impl = libpointer('voidPtr');
            Ice.Util.callMethod(self, 'clone', impl);
            r = Ice.Properties(impl);
        end
    end
end
