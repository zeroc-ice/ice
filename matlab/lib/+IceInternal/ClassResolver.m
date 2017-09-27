%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef ClassResolver < handle
    methods
        function obj = ClassResolver(props)
            obj.defaultPrefix = props.getProperty('Ice.Default.Package');
            obj.prefixMap = containers.Map('KeyType', 'char', 'ValueType', 'char');
            obj.typeToConstructorMap = containers.Map('KeyType', 'char', 'ValueType', 'any');

            dict = props.getPropertiesForPrefix('Ice.Package.');
            keys = dict.keys();
            for i = 1:length(keys)
                key = keys{i};
                name = strrep(key, 'Ice.Package.', ''); % Strip property prefix
                obj.prefixMap(name) = dict(key);
            end
        end
        function r = resolve(obj, typeId)
            %
            % NOTE: The return value is the constructor function.
            %
            % To convert a Slice type id into a class, we do the following:
            %
            % 1. Convert the Slice type id into a classname (e.g., ::M::X -> M.X).
            % 2. If that fails, extract the top-level module (if any) from the type id
            %    and check for a Package property. If found, prepend the property
            %    value to the classname.
            % 3. If that fails, check for a default package. If defined,
            %    prepend the property value to the classname.
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
            % See if we can find the class without any prefix.
            %
            found = exist(className, 'class');

            if ~found
                %
                % See if the application defined an Ice.Package.MODULE property.
                %
                topLevelModule = strtok(typeId, ':');
                if obj.prefixMap.isKey(topLevelModule)
                    cls = [obj.prefixMap(topLevelModule), '.', className];
                    if exist(cls, 'class')
                        found = true;
                        className = cls;
                    end
                end
            end

            %
            % See if the application defined a default package.
            %
            if ~found && ~isempty(obj.defaultPrefix)
                cls = [obj.defaultPrefix, '.', className];
                if exist(cls, 'class')
                    found = true;
                    className = cls;
                end
            end

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
        defaultPrefix
        prefixMap
        typeToConstructorMap
    end
end
