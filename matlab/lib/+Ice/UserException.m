%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef (Abstract) UserException < Ice.Exception
    methods
        function obj = UserException(id, msg)
            obj = obj@Ice.Exception(id, msg)
        end
        function r = ice_getSlicedData(obj)
            r = obj.slicedData_;
        end
    end
    methods(Hidden=true)
        function obj = read_(obj, is)
            is.startException();
            if obj.usesAnyClasses_()
                %
                % Exceptions are value types. We use a shared map (which is a handle type) to keep track of class
                % instances as they are unmarshaled.
                %
                obj.valueTable_ = containers.Map('KeyType', 'char', 'ValueType', 'any');
            end
            obj = obj.readImpl_(is);
            obj.slicedData_ = is.endException(obj.preserve_());
            if obj.usesClasses_()
                is.readPendingValues();
            end
            if obj.usesAnyClasses_()
                %
                % Retrieve the value instances from the map and update our data members.
                %
                obj = obj.resolveValues_();
            end
        end
    end
    methods(Access=protected)
        function r = preserve_(obj)
            %
            % Overridden by subclasses that have the "preserve-slice" metadata.
            %
            r = false;
        end
        function r = usesClasses_(obj)
            %
            % Overridden by subclasses to indicate whether they contain any *required* data members that use classes.
            %
            r = false;
        end
        function r = usesAnyClasses_(obj)
            %
            % Overridden by subclasses to indicate whether they contain any data members that use classes.
            %
            r = false;
        end
        function obj = resolveValues_(obj)
            %
            % Overridden by subclasses that have class members.
            %
        end
        function setValueMember_(obj, k, v)
            %
            % Store an unmarshaled class instance in our map.
            %
            obj.valueTable_(k) = v;
        end
    end
    methods(Abstract,Access=protected)
        obj = readImpl_(obj, is)
    end
    properties(Access=protected)
        valueTable_
        slicedData_
    end
end
