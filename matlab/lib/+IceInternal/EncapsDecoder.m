% Copyright (c) ZeroC, Inc.

classdef (Abstract) EncapsDecoder < handle
    methods
        function obj = EncapsDecoder(is, encaps, classGraphDepthMax)
            obj.is = is;
            obj.encaps = encaps;
            obj.classGraphDepthMax = classGraphDepthMax;
            obj.classGraphDepth = 0;
            obj.patchMap = {};
            obj.patchMapLength = 0;
            obj.unmarshaledMap = {};
            obj.typeIdMap = {};
            obj.valueList = {};
            obj.delayedPostUnmarshal = {};
        end

        function readPendingValues(~)
        end

        function finish(obj)
            %
            % This is our opportunity for unmarshaled values to do some post processing after the initial round
            % of unmarshaling is complete.
            %
            if ~isempty(obj.delayedPostUnmarshal)
                %
                % First call icePostUnmarshal on every instance. This allows the generated code to finish its tasks.
                %
                for i = 1:length(obj.delayedPostUnmarshal)
                    v = obj.delayedPostUnmarshal{i};
                    v.icePostUnmarshal();
                end
                %
                % Then call ice_postUnmarshal on every instance. This is the application's interception point.
                %
                for i = 1:length(obj.delayedPostUnmarshal)
                    v = obj.delayedPostUnmarshal{i};
                    v.ice_postUnmarshal();
                end

                obj.delayedPostUnmarshal = {};
            end
        end
    end
    methods(Abstract)
        readValue(obj, cb)
        throwException(obj)
        startInstance(obj, sliceType)
        r = endInstance(obj)
        r = startSlice(obj)
        endSlice(obj)
        skipSlice(obj)
    end
    methods(Access=protected)
        function r = getTypeId(obj, typeIdIndex)
            if typeIdIndex <= length(obj.typeIdMap)
                r = obj.typeIdMap{typeIdIndex};
            else
                throw(Ice.MarshalException('unknown type id index'));
            end
        end
        function [typeIdIndex, typeId] = readTypeId(obj)
            typeId = obj.is.readString();
            obj.typeIdMap{end + 1} = typeId;
            typeIdIndex = length(obj.typeIdMap);
        end
        function r = newInstance(obj, typeId)
            r = obj.is.getCommunicator().getSliceLoader().newInstance(typeId);
        end

        function addPatchEntry(obj, index, cb)
            %assert(index > 0);
            %
            % Check if we have already unmarshaled the instance. If that's the case,
            % just invoke the callback and we're done.
            %
            if index <= length(obj.unmarshaledMap)
                v = obj.unmarshaledMap{index};
                if ~isempty(v)
                    cb(v);
                    return;
                end
            end

            %
            % Add patch entry if the instance isn't unmarshaled yet,
            % the callback will be called when the instance is
            % unmarshaled.
            %
            if index <= length(obj.patchMap)
                pl = obj.patchMap{index};
            else
                %
                % We have no outstanding instances to be patched for this
                % index, so make a new entry in the patch map.
                %
                pl = {};
                obj.patchMapLength = obj.patchMapLength + 1;
            end

            %
            % Append a patch entry for this instance.
            %
            e = IceInternal.PatchEntry();
            e.cb = cb;
            e.classGraphDepth = obj.classGraphDepth;
            pl{end + 1} = e;
            obj.patchMap{index} = pl;
        end

        function unmarshal(obj, index, v)
            %
            % Add the instance to the map of unmarshaled instances, this must
            % be done before reading the instances (for circular references).
            %
            obj.unmarshaledMap{index} = v;

            %
            % Read the instance.
            %
            v.iceRead(obj.is);

            if index <= length(obj.patchMap)
                %
                % Patch all instances now that the instance is unmarshaled.
                %
                l = obj.patchMap{index};

                %
                % Patch all pointers that refer to the instance.
                %
                for i = 1:length(l)
                    e = l{i};
                    e.cb(v);
                end

                %
                % Clear out the patch map for that index -- there is nothing left
                % to patch for that index for the time being.
                %
                obj.patchMap{index} = [];
                obj.patchMapLength = obj.patchMapLength - 1;
            end

            if obj.patchMapLength == 0 && isempty(obj.valueList)
                if v.iceDelayPostUnmarshal()
                    obj.delayedPostUnmarshal{end + 1} = v; % See finish()
                else
                    v.ice_postUnmarshal();
                end
            else
                obj.valueList{end + 1} = v;

                if obj.patchMapLength == 0
                    %
                    % Iterate over the instance list and invoke ice_postUnmarshal on
                    % each instance. We must do this after all instances have been
                    % unmarshaled in order to ensure that any instance data members
                    % have been properly patched.
                    %
                    for i = 1:length(obj.valueList)
                        p = obj.valueList{i};
                        if p.iceDelayPostUnmarshal()
                            obj.delayedPostUnmarshal{end + 1} = p; % See finish()
                        else
                            p.ice_postUnmarshal();
                        end
                    end
                    obj.valueList = {};
                end
            end
        end
    end
    properties(Access=protected)
        is
        encaps
        classGraphDepth
        classGraphDepthMax
        patchMap
        patchMapLength
    end
    properties(Access=private)
        unmarshaledMap
        typeIdMap
        valueList
        delayedPostUnmarshal
    end
end
