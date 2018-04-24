%{
**********************************************************************

Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef ClassResolver < handle
    methods
        function obj = ClassResolver(props)
            obj.typeToConstructorMap = containers.Map('KeyType', 'char', 'ValueType', 'any');
        end
        function r = resolve(obj, typeId)
            %
            % NOTE: The return value is the constructor function.
            %

            %
            % See if we've already translated this type ID before.
            %
            try
                %
                % A type is only added to this map if its class exists, so we're done.
                %
                r = obj.typeToConstructorMap(typeId);
                return;
            catch ex
                %
                % The map raises an exception if a key doesn't exist.
                %
            end

            %
            % If it's a new type ID, first convert it into a class name.
            %
            className = IceInternal.Util.idToClass(typeId);

            %
            % See if we can find the class.
            %
            found = exist(className, 'class');

            %
            % If we found the class, update our map so we don't have to translate this type ID again.
            %
            if found
                constructor = str2func(className);
                obj.typeToConstructorMap(typeId) = constructor;
                r = constructor;
            else
                r = [];
            end
        end
    end
    properties(Access=private)
        typeToConstructorMap
    end
end
