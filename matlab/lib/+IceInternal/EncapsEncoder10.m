%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef EncapsEncoder10 < IceInternal.EncapsEncoder
    methods
        function obj = EncapsEncoder10(os, encaps)
            obj = obj@IceInternal.EncapsEncoder(os, encaps);
            obj.sliceType = IceInternal.SliceType.NoSlice;
            obj.valueIdIndex = 0;
            obj.toBeMarshaledMap = containers.Map('KeyType', 'int32', 'ValueType', 'any');
            obj.marshaledMap = containers.Map('KeyType', 'int32', 'ValueType', 'any');
        end

        function writeValue(obj, v)
            %
            % Object references are encoded as a negative integer in 1.0.
            %
            if ~isempty(v)
                obj.os.writeInt(-obj.registerValue(v));
            else
                obj.os.writeInt(0);
            end
        end

        function startInstance(obj, sliceType, slicedData)
            obj.sliceType = sliceType;
        end

        function endInstance(obj)
            if obj.sliceType == IceInternal.SliceType.ValueSlice
                %
                % Write the Object slice.
                %
                obj.startSlice(Ice.Value.ice_staticId(), -1, true);
                obj.os.writeSize(0); % For compatibility with the old AFM.
                obj.endSlice();
            end
            obj.sliceType = IceInternal.SliceType.NoSlice;
        end

        function startSlice(obj, typeId, compactId, last)
            %
            % For instance slices, encode a boolean to indicate how the type ID
            % is encoded and the type ID either as a string or index. For
            % exception slices, always encode the type ID as a string.
            %
            if obj.sliceType == IceInternal.SliceType.ValueSlice
                index = obj.registerTypeId(typeId);
                if index < 0
                    obj.os.writeBool(false);
                    obj.os.writeString(typeId);
                else
                    obj.os.writeBool(true);
                    obj.os.writeSize(index);
                end
            else
                obj.os.writeString(typeId);
            end

            %obj.os.writeInt(0); % Placeholder for the slice length.
            obj.os.buf.resize(obj.os.buf.size + 4);

            obj.writeSlice = obj.os.getPos() + 1;
        end

        function endSlice(obj)
            %
            % Write the slice length.
            %
            sz = obj.os.getPos() - obj.writeSlice + 4 + 1;
            obj.os.rewriteInt(sz, obj.writeSlice - 4);
        end

        function writePendingValues(obj)
            while obj.toBeMarshaledMap.Count > 0
                %
                % Consider the to be marshalled instances as marshaled now,
                % this is necessary to avoid adding again the "to be
                % marshaled instances" into toBeMarshaledMap while writing
                % instances.
                %
                obj.marshaledMap = [obj.marshaledMap; obj.toBeMarshaledMap];

                savedMap = obj.toBeMarshaledMap;
                obj.toBeMarshaledMap = containers.Map('KeyType', 'int32', 'ValueType', 'any');
                obj.os.writeSize(savedMap.Count);
                keys = savedMap.keys();
                for i = 1:length(keys)
                    %
                    % Ask the instance to marshal itself. Any new class
                    % instances that are triggered by the classes marshaled
                    % are added to toBeMarshaledMap.
                    %
                    obj.os.writeInt(keys{i});

                    v = savedMap(keys{i});
                    try
                        v.ice_preMarshal();
                    catch ex
                        msg = sprintf('exception raised by ice_preMarshal:\n%s', getReport(ex, 'extended'));
                        obj.os.getCommunicator().getLogger().warning(msg);
                    end

                    v.iceWrite_(obj.os);
                end
            end
            obj.os.writeSize(0); % Zero marker indicates end of sequence of sequences of instances.
        end
    end
    methods(Access=protected)
        function r = registerValue(obj, v)
            assert(~isempty(v));

            %
            % Use the identifier assigned by the Value constructor.
            %
            r = v.iceInternal_;

            %
            % Look for this instance in the to-be-marshaled map.
            %
            if obj.toBeMarshaledMap.isKey(r)
                return;
            end

            %
            % Didn't find it, try the marshaled map next.
            %
            if obj.marshaledMap.isKey(r)
                return;
            end

            %
            % We haven't seen this instance previously, create a new
            % index, and insert it into the to-be-marshaled map.
            %
            obj.toBeMarshaledMap(r) = v;
        end
    end
    properties(Access=private)
        sliceType
        writeSlice
        valueIdIndex
        toBeMarshaledMap
        marshaledMap
    end
end
