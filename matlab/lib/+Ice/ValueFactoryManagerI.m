%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef ValueFactoryManagerI < Ice.ValueFactoryManager
    methods
        function obj = ValueFactoryManagerI()
            obj.factories = containers.Map('KeyType', 'char', 'ValueType', 'any');
        end
        function add(obj, factory, id)
            if obj.factories.isKey(id)
                throw(Ice.AlreadyRegisteredException('', '', 'value factory', id));
            elseif ~isa(factory, 'function_handle')
                throw(MException('Ice:ArgumentException', 'factory argument is not a function handle'));
            else
                obj.factories(id) = factory;
            end
        end
        function r = find(obj, id)
            if obj.factories.isKey(id)
                r = obj.factories(id);
            else
                r = [];
            end
        end
    end
    properties(Access=private)
        factories
    end
end
