%{
**********************************************************************

Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef ValueFactoryManagerI < Ice.ValueFactoryManager
    methods
        function obj = ValueFactoryManagerI()
            obj.factories = containers.Map('KeyType', 'char', 'ValueType', 'any');
            obj.hasFactories = false;
            obj.defaultFactory = [];
        end
        function add(obj, factory, id)
            if isempty(id)
                if ~isempty(obj.defaultFactory)
                    throw(Ice.AlreadyRegisteredException('', '', 'value factory', id));
                end
                obj.defaultFactory = factory;
            else
                if obj.factories.isKey(id)
                    throw(Ice.AlreadyRegisteredException('', '', 'value factory', id));
                elseif ~isa(factory, 'function_handle')
                    throw(MException('Ice:ArgumentException', 'factory argument is not a function handle'));
                else
                    obj.factories(id) = factory;
                    obj.hasFactories = true;
                end
            end
        end
        function r = find(obj, id)
            if isempty(id)
                r = obj.defaultFactory;
            elseif obj.hasFactories && obj.factories.isKey(id)
                r = obj.factories(id);
            else
                r = [];
            end
        end
    end
    properties(Access=private)
        factories
        hasFactories
        defaultFactory
    end
end
