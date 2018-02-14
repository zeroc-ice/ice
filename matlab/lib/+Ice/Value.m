classdef (Abstract) Value < matlab.mixin.Copyable
    % Value   Summary of Value
    %
    % The base class for instances of Slice classes.
    %
    % Value Methods:
    %   ice_preMarshal - The Ice run time invokes this method prior to
    %     marshaling an object's data members.
    %   ice_postUnmarshal - The Ice run time invokes this method after
    %     unmarshaling an object's data members.
    %   ice_getSlicedData - Returns the sliced data if the value has a
    %     preserved-slice base class and has been sliced during unmarshaling.

    % Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.

    methods
        function obj = Value()
            % Value constructor

            %
            % We need to assign each Value instance a unique identifier for marshaling purposes. This persistent
            % variable retains its setting as long as there is at least one Value instance still active. Once the
            % last instance is collected, the variable gets cleared and we start over again.
            %
            persistent index;
            if isempty(index)
                index = int32(0);
            end
            index = index + 1;
            assert(index > 0); % Check for rollover
            obj.iceInternal_ = index;
        end
        function ice_preMarshal(obj)
            % ice_preMarshal - The Ice run time invokes this method prior to
            %   marshaling an object's data members. This allows a subclass
            %   to override this method in order to validate its data members.
        end
        function ice_postUnmarshal(obj)
            % ice_postUnmarshal - The Ice run time invokes this method after
            %   unmarshaling an object's data members. This allows a subclass
            %   to override this method in order to perform additional
            %   initialization.
        end
        function r = ice_getSlicedData(obj)
            % ice_getSlicedData - Returns the sliced data if the value has a
            %   preserve-slice base class and has been sliced during
            %   unmarshaling of the value; an empty array is returned otherwise.

            %
            % Overridden by subclasses that have the "preserve-slice" metadata.
            %
            r = [];
        end
    end
    methods(Abstract)
        id = ice_id(obj)
    end
    methods(Static)
        function id = ice_staticId()
            id = '::Ice::Object';
        end
    end
    methods(Hidden=true)
        function iceWrite(obj, os)
            os.startValue([]);
            obj.iceWriteImpl(os);
            os.endValue();
        end
        function iceRead(obj, is)
            is.startValue();
            obj.iceReadImpl(is);
            is.endValue(false);
        end
        function r = iceDelayPostUnmarshal(obj)
            %
            % Overridden by subclasses that need to do some post-processing after the initial round of
            % unmarshaling is complete.
            %
            r = false;
        end
        function icePostUnmarshal(obj)
            %
            % Overridden by subclasses that need to do some post-processing after the initial round of
            % unmarshaling is complete.
            %
        end
    end
    methods(Abstract,Access=protected)
        iceWriteImpl(obj, os)
        iceReadImpl(obj, is)
    end
    properties(Hidden, NonCopyable)
        iceInternal_ int32
    end
end
