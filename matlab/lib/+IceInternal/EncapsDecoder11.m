%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef EncapsDecoder11 < IceInternal.EncapsDecoder
    methods
        function obj = EncapsDecoder11(is, encaps)
            obj = obj@IceInternal.EncapsDecoder(is, encaps);
        end
        function throwException(obj)
            assert(isempty(obj.current));

            obj.push(IceInternal.SliceType.ExceptionSlice);

            %
            % Read the first slice header.
            %
            obj.startSlice();
            mostDerivedId = obj.current.typeId;
            while true
                %
                % Translate the type ID into a class name.
                %
                cls = Ice.Util.idToClass(obj.current.typeId);

                %
                % Try to instantiate the class.
                %
                ex = [];
                try
                    ex = eval(cls);
                catch e
                    %
                    % Instantiation failed.
                    %
                end

                if ~isempty(ex)
                    ex = ex.read_(obj.is);
                    throw(ex);
                else
                    %
                    % Slice off what we don't understand.
                    %
                    obj.skipSlice();

                    %
                    % If this is the last slice, raise an exception and stop unmarshaling.
                    %
                    if bitand(obj.current.sliceFlags, obj.FLAG_IS_LAST_SLICE)
                        throw(Ice.UnknownUserException('', '', mostDerivedId));
                    end

                    obj.startSlice();
                end
            end
        end
        function startInstance(obj, sliceType)
            assert(obj.current.sliceType == sliceType);
            obj.current.skipFirstSlice = true;
        end
        function endInstance(obj) % TODO: SlicedData
            %{
            SlicedDataPtr slicedData;
            if(preserve)
            {
                slicedData = readSlicedData();
            }
            _current->slices.clear();
            _current->indirectionTables.clear();
            _current = _current->previous;
            return slicedData;
            %}
            obj.current.slices = [];
            obj.current = obj.current.previous;
        end
        function r = startSlice(obj)
            %
            % If first slice, don't read the header, it was already read in
            % readInstance or throwException to find the factory.
            %
            if obj.current.skipFirstSlice
                obj.current.skipFirstSlice = false;
                r = obj.current.typeId;
                return;
            end

            obj.current.sliceFlags = obj.is.readByte();

            %
            % Read the type ID, for value slices the type ID is encoded as a
            % string or as an index, for exceptions it's always encoded as a
            % string.
            %
            if obj.current.sliceType == IceInternal.SliceType.ValueSlice
                % Must be checked first!
                if bitand(obj.current.sliceFlags, obj.FLAG_HAS_TYPE_ID_COMPACT) == obj.FLAG_HAS_TYPE_ID_COMPACT
                    obj.current.typeId = '';
                    obj.current.compactId = obj.is.readSize();
                elseif bitand(obj.current.sliceFlags, bitor(obj.FLAG_HAS_TYPE_ID_STRING, obj.FLAG_HAS_TYPE_ID_INDEX))
                    obj.current.typeId = obj.readTypeId(bitand(obj.current.sliceFlags, obj.FLAG_HAS_TYPE_ID_INDEX) > 0);
                    obj.current.compactId = -1;
                else
                    % Only the most derived slice encodes the type ID for the compact format.
                    obj.current.typeId = '';
                    obj.current.compactId = -1;
                end
            else
                obj.current.typeId = obj.is.readString();
            end

            %
            % Read the slice size if necessary.
            %
            if bitand(obj.current.sliceFlags, obj.FLAG_HAS_SLICE_SIZE)
                obj.current.sliceSize = obj.is.readInt();
                if obj.current.sliceSize < 4
                    throw(Ice.UnmarshalOutOfBoundsException());
                end
            else
                obj.current.sliceSize = 0;
            end

            r = obj.current.typeId;
        end
        function endSlice(obj)
            if bitand(obj.current.sliceFlags, obj.FLAG_HAS_OPTIONAL_MEMBERS)
                obj.is.skipOptionals();
            end

            %
            % Read the indirect object table if one is present.
            %
            if bitand(obj.current.sliceFlags, obj.FLAG_HAS_INDIRECTION_TABLE)
                %{
                % TODO
                IndexList indirectionTable(obj.is.readAndCheckSeqSize(1));
                for(IndexList::iterator p = indirectionTable.begin(); p != indirectionTable.end(); ++p)
                {
                    *p = readInstance(obj.is.readSize(), 0, 0);
                }

                %
                % Sanity checks. If there are optional members, it's possible
                % that not all object references were read if they are from
                % unknown optional data members.
                %
                if(indirectionTable.empty())
                {
                    throw MarshalException(__FILE__, __LINE__, "empty indirection table");
                }
                if(obj.current.indirectPatchList.empty() && !(obj.current.sliceFlags & FLAG_HAS_OPTIONAL_MEMBERS))
                {
                    throw MarshalException(__FILE__, __LINE__, "no references to indirection table");
                }

                %
                % Convert indirect references into direct references.
                %
                IndirectPatchList::iterator p;
                for(p = obj.current.indirectPatchList.begin(); p != obj.current.indirectPatchList.end(); ++p)
                {
                    assert(p->index >= 0);
                    if(p->index >= static_cast<Int>(indirectionTable.size()))
                    {
                        throw MarshalException(__FILE__, __LINE__, "indirection out of range");
                    }
                    addPatchEntry(indirectionTable[p->index], p->patchFunc, p->patchAddr);
                }
                obj.current.indirectPatchList.clear();
                %}
            end
        end
        function skipSlice(obj)
            %obj.is.traceSkipSlice(obj.current.typeId, obj.current.sliceType);

            start = obj.is.pos();

            if bitand(obj.current.sliceFlags, obj.FLAG_HAS_SLICE_SIZE)
                assert(obj.current.sliceSize >= 4);
                obj.is.skip(obj.current.sliceSize - 4);
            else
                if obj.current.sliceType == IceInternal.SliceType.ValueSlice
                    throw(Ice.NoValueFactoryException('', '', ...
                            ['no value factory found and compact format prevents ', ...
                            'slicing (the sender should use the sliced format instead)'], obj.current.typeId));
                else
                    throw(Ice.UnknownUserException('', '', obj.current.typeId));
                end
            end

            %
            % Preserve this slice.
            %
            info = Ice.SliceInfo();
            info.typeId = obj.current.typeId;
            info.compactId = obj.current.compactId;
            info.hasOptionalMembers = bitand(obj.current.sliceFlags, obj.FLAG_HAS_OPTIONAL_MEMBERS) > 0;
            info.isLastSlice = bitand(obj.current.sliceFlags, obj.FLAG_IS_LAST_SLICE) > 0;
            if info.hasOptionalMembers
                %
                % Don't include the optional member end marker. It will be re-written by
                % endSlice when the sliced data is re-written.
                %
                info.bytes = obj.is.getBytes(start, obj.is.pos() - 1);
            else
                info.bytes = obj.is.getBytes(start, obj.is.pos());
            end

            %{
            % TODO
            obj.current.indirectionTables.push_back(IndexList());

            %
            % Read the indirect object table. We read the instances or their
            % IDs if the instance is a reference to an already un-marhsaled
            % object.
            %
            % The SliceInfo object sequence is initialized only if
            % readSlicedData is called.
            %
            if bitand(obj.current.sliceFlags, obj.FLAG_HAS_INDIRECTION_TABLE)
                IndexList& table = obj.current.indirectionTables.back();
                table.resize(obj.is.readAndCheckSeqSize(1));
                for(IndexList::iterator p = table.begin(); p != table.end(); ++p)
                {
                    *p = readInstance(obj.is.readSize(), 0, 0);
                }

            obj.current.slices.push_back(info);
            %}
        end
        function r = readOptional(obj, readTag, expectedFormat)
            if isempty(obj.current)
                r = obj.is.readOptImpl(readTag, expectedFormat);
                return;
            elseif bitand(obj.current.sliceFlags, obj.FLAG_HAS_OPTIONAL_MEMBERS)
                r = obj.is.readOptImpl(readTag, expectedFormat);
                return;
            end
            r = false;
        end
    end
    methods(Access=private)
        function push(obj, sliceType)
            obj.current = IceInternal.InstanceData(obj.current);
            obj.current.sliceType = sliceType;
            obj.current.skipFirstSlice = false;
        end
    end
    properties(Access=private)
        current
    end
    properties(Constant,Access=private)
        FLAG_HAS_TYPE_ID_STRING    = bitshift(1, 0)
        FLAG_HAS_TYPE_ID_INDEX     = bitshift(1, 1)
        FLAG_HAS_TYPE_ID_COMPACT   = bitor(bitshift(1, 0), bitshift(1, 1))
        FLAG_HAS_OPTIONAL_MEMBERS  = bitshift(1, 2)
        FLAG_HAS_INDIRECTION_TABLE = bitshift(1, 3)
        FLAG_HAS_SLICE_SIZE        = bitshift(1, 4)
        FLAG_IS_LAST_SLICE         = bitshift(1, 5)
    end
end
