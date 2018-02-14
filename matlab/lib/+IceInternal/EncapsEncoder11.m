%{
**********************************************************************

Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef EncapsEncoder11 < IceInternal.EncapsEncoder
    methods
        function obj = EncapsEncoder11(os, encaps)
            obj = obj@IceInternal.EncapsEncoder(os, encaps);
            obj.current = [];
            obj.valueIdIndex = 1;
            obj.marshaledMap = containers.Map('KeyType', 'int32', 'ValueType', 'int32');
        end

        function writeValue(obj, v)
            if isempty(v)
                obj.os.writeSize(0);
            else
                if ~isempty(obj.current) && obj.encaps.format == Ice.FormatType.SlicedFormat
                    %
                    % If writing an instance within a slice and using the sliced
                    % format, write an index from the instance indirection
                    % table. The indirect instance table is encoded at the end of
                    % each slice and is always read (even if the Slice is
                    % unknown).
                    %
                    if ~obj.current.indirectionMap.isKey(v.iceInternal_)
                        obj.current.indirectionTable{end + 1} = v;
                        idx = length(obj.current.indirectionTable); % 0 is reserved for nil
                        obj.current.indirectionMap(v.iceInternal_) = idx;
                        obj.os.writeSize(idx);
                    else
                        obj.os.writeSize(obj.current.indirectionMap(v.iceInternal_));
                    end
                else
                    obj.writeInstance(v); % Write the instance or a reference if already marshaled.
                end
            end
        end

        function startInstance(obj, sliceType, slicedData)
            if isempty(obj.current)
                obj.current = IceInternal.EncapsEncoder11_InstanceData([]);
            else
                if isempty(obj.current.next)
                    obj.current = IceInternal.EncapsEncoder11_InstanceData(obj.current);
                else
                    obj.current = obj.current.next;
                end
            end
            obj.current.sliceType = sliceType;
            obj.current.firstSlice = true;

            if ~isempty(slicedData)
                obj.writeSlicedData(slicedData);
            end
        end

        function endInstance(obj)
            obj.current = obj.current.previous;
        end

        function startSlice(obj, typeId, compactId, last)
            import IceInternal.Protocol;
            assert(isempty(obj.current.indirectionTable) && isempty(obj.current.indirectionMap));

            obj.current.sliceFlagsPos = obj.os.getPos() + 1;

            obj.current.sliceFlags = uint8(0);
            if obj.encaps.format == Ice.FormatType.SlicedFormat
                % Encode the slice size if using the sliced format.
                obj.current.sliceFlags = bitor(obj.current.sliceFlags, Protocol.FLAG_HAS_SLICE_SIZE);
            end
            if last
                % This is the last slice.
                obj.current.sliceFlags = bitor(obj.current.sliceFlags, Protocol.FLAG_IS_LAST_SLICE);
            end

            %obj.os.writeByte(0); % Placeholder for the slice flags
            obj.os.buf.resize(obj.os.buf.size + 1);

            %
            % For instance slices, encode the flag and the type ID either as a
            % string or index. For exception slices, always encode the type ID
            % as a string.
            %
            if obj.current.sliceType == IceInternal.SliceType.ValueSlice
                %
                % Encode the type ID (only in the first slice for the compact encoding).
                %
                if obj.encaps.format == Ice.FormatType.SlicedFormat || obj.current.firstSlice
                    if compactId >= 0
                        obj.current.sliceFlags = bitor(obj.current.sliceFlags, Protocol.FLAG_HAS_TYPE_ID_COMPACT);
                        obj.os.writeSize(compactId);
                    else
                        index = obj.registerTypeId(typeId);
                        if index < 0
                            obj.current.sliceFlags = bitor(obj.current.sliceFlags, Protocol.FLAG_HAS_TYPE_ID_STRING);
                            obj.os.writeString(typeId);
                        else
                            obj.current.sliceFlags = bitor(obj.current.sliceFlags, Protocol.FLAG_HAS_TYPE_ID_INDEX);
                            obj.os.writeSize(index);
                        end
                    end
                end
            else
                obj.os.writeString(typeId);
            end

            if bitand(obj.current.sliceFlags, Protocol.FLAG_HAS_SLICE_SIZE)
                %obj.os.writeInt(0); % Placeholder for the slice length.
                obj.os.buf.resize(obj.os.buf.size + 4);
            end

            obj.current.writeSlice = obj.os.getPos() + 1;
            obj.current.firstSlice = false;
        end

        function endSlice(obj)
            import IceInternal.Protocol;
            %
            % Write the optional member end marker if some optional members
            % were encoded. Note that the optional members are encoded before
            % the indirection table and are included in the slice size.
            %
            if bitand(obj.current.sliceFlags, Protocol.FLAG_HAS_OPTIONAL_MEMBERS)
                obj.os.writeByte(Protocol.OPTIONAL_END_MARKER);
            end

            %
            % Write the slice length if necessary.
            %
            if bitand(obj.current.sliceFlags, Protocol.FLAG_HAS_SLICE_SIZE)
                sz = obj.os.getPos() - obj.current.writeSlice + 4 + 1;
                obj.os.rewriteInt(sz, obj.current.writeSlice - 4);
            end

            %
            % Only write the indirection table if it contains entries.
            %
            if ~isempty(obj.current.indirectionTable)
                assert(obj.encaps.format == Ice.FormatType.SlicedFormat);
                obj.current.sliceFlags = bitor(obj.current.sliceFlags, Protocol.FLAG_HAS_INDIRECTION_TABLE);

                %
                % Write the indirection instance table.
                %
                obj.os.writeSize(length(obj.current.indirectionTable));
                for i = 1:length(obj.current.indirectionTable)
                    obj.writeInstance(obj.current.indirectionTable{i});
                end
                obj.current.indirectionTable = {};
                obj.current.indirectionMap = containers.Map('KeyType', 'int32', 'ValueType', 'int32');
            end

            %
            % Finally, update the slice flags.
            %
            obj.os.rewriteByte(obj.current.sliceFlags, obj.current.sliceFlagsPos);
        end

        function r = writeOptional(obj, tag, format)
            import IceInternal.Protocol;
            if isempty(obj.current)
                r = obj.os.writeOptionalImpl(tag, format);
            else
                if obj.os.writeOptionalImpl(tag, format)
                    obj.current.sliceFlags = bitor(obj.current.sliceFlags, Protocol.FLAG_HAS_OPTIONAL_MEMBERS);
                    r = true;
                else
                    r = false;
                end
            end
        end
    end
    methods(Access=private)
        function writeSlicedData(obj, slicedData)
            import IceInternal.Protocol;
            assert(~isempty(slicedData));

            %
            % We only remarshal preserved slices if we are using the sliced
            % format. Otherwise, we ignore the preserved slices, which
            % essentially "slices" the instance into the most-derived type
            % known by the sender.
            %
            if obj.encaps.format ~= Ice.FormatType.SlicedFormat
                return;
            end

            for i = 1:length(slicedData.slices)
                info = slicedData.slices{i};

                obj.startSlice(info.typeId, info.compactId, info.isLastSlice);

                %
                % Write the bytes associated with this slice.
                %
                obj.os.writeBlob(info.bytes);

                if info.hasOptionalMembers
                    obj.current.sliceFlags = bitor(obj.current.sliceFlags, Protocol.FLAG_HAS_OPTIONAL_MEMBERS);
                end

                %
                % Make sure to also re-write the instance indirection table.
                %
                if ~isempty(info.instances)
                    for j = 1:length(info.instances)
                        v = info.instances{j};
                        obj.current.indirectionTable{end + 1} = v;
                    end
                end

                obj.endSlice();
            end
        end

        function writeInstance(obj, v)
            assert(~isempty(v));

            %
            % If the instance was already marshaled, just write its ID.
            %
            if obj.marshaledMap.isKey(v.iceInternal_)
                obj.os.writeSize(obj.marshaledMap(v.iceInternal_));
                return;
            end

            %
            % We haven't seen this instance previously, create a new ID,
            % insert it into the marshaled map, and write the instance.
            %
            obj.valueIdIndex = obj.valueIdIndex + 1;
            obj.marshaledMap(v.iceInternal_) = obj.valueIdIndex;

            try
                v.ice_preMarshal();
            catch ex
                msg = sprintf('exception raised by ice_preMarshal:\n%s', getReport(ex, 'extended'));
                obj.os.getCommunicator().getLogger().warning(msg);
            end

            obj.os.writeSize(1); % Class instance marker.
            v.iceWrite(obj.os);
        end
    end
    properties(Access=private)
        current
        valueIdIndex
        marshaledMap
    end
end
