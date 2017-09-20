%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef (Abstract) Value < matlab.mixin.Copyable
    methods
        function obj = Value()
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
        end
        function ice_postUnmarshal(obj)
        end
        function r = ice_getSlicedData(obj)
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
        function iceWrite_(obj, os)
            os.startValue([]);
            obj.iceWriteImpl_(os);
            os.endValue();
        end
        function iceRead_(obj, is)
            is.startValue();
            obj.iceReadImpl_(is);
            is.endValue(false);
        end
        function r = iceDelayPostUnmarshal_(obj)
            %
            % Overridden by subclasses that need to do some post-processing after the initial round of
            % unmarshaling is complete.
            %
            r = false;
        end
        function icePostUnmarshal_(obj)
            %
            % Overridden by subclasses that need to do some post-processing after the initial round of
            % unmarshaling is complete.
            %
        end
    end
    methods(Abstract,Access=protected)
        iceWriteImpl_(obj, os)
        iceReadImpl_(obj, is)
    end
    properties(Hidden, NonCopyable)
        iceInternal_ int32
    end
end
