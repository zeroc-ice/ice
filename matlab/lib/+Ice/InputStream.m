%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef InputStream < IceInternal.WrapperObject
    methods
        function obj = InputStream(impl, communicator)
            obj = obj@IceInternal.WrapperObject(impl);
            obj.communicator = communicator;
            obj.encoding = obj.callWithResult_('getEncoding');
            obj.sliceValues = true;
        end
        function r = getCommunicator(obj)
            r = obj.communicator;
        end
        function r = readBool(obj)
            v = libpointer('uint8Ptr', 0);
            obj.call_('readBool', v);
            r = v.Value == 1;
        end
        function r = readBoolSeq(obj)
            r = obj.callWithResult_('readBoolSeq');
        end
        function r = readBoolOpt(obj, tag)
            if obj.readOptional(tag, Ice.OptionalFormat.F1)
                r = obj.readBool();
            else
                r = Ice.Unset;
            end
        end
        function r = readBoolSeqOpt(obj, tag)
            if obj.readOptional(tag, Ice.OptionalFormat.VSize)
                r = obj.readBoolSeq();
            else
                r = Ice.Unset;
            end
        end
        function r = readByte(obj)
            v = libpointer('uint8Ptr', 0);
            obj.call_('readByte', v);
            r = v.Value;
        end
        function r = readByteSeq(obj)
            r = obj.callWithResult_('readByteSeq');
        end
        function r = readByteOpt(obj, tag)
            if obj.readOptional(tag, Ice.OptionalFormat.F1)
                r = obj.readByte();
            else
                r = Ice.Unset;
            end
        end
        function r = readByteSeqOpt(obj, tag)
            if obj.readOptional(tag, Ice.OptionalFormat.VSize)
                r = obj.readByteSeq();
            else
                r = Ice.Unset;
            end
        end
        function r = readShort(obj)
            v = libpointer('int16Ptr', 0);
            obj.call_('readShort', v);
            r = v.Value;
        end
        function r = readShortSeq(obj)
            r = obj.callWithResult_('readShortSeq');
        end
        function r = readShortOpt(obj, tag)
            if obj.readOptional(tag, Ice.OptionalFormat.F2)
                r = obj.readShort();
            else
                r = Ice.Unset;
            end
        end
        function r = readShortSeqOpt(obj, tag)
            if obj.readOptional(tag, Ice.OptionalFormat.VSize)
                obj.skipSize();
                r = obj.readShortSeq();
            else
                r = Ice.Unset;
            end
        end
        function r = readInt(obj)
            v = libpointer('int32Ptr', 0);
            obj.call_('readInt', v);
            r = v.Value;
        end
        function r = readIntSeq(obj)
            r = obj.callWithResult_('readIntSeq');
        end
        function r = readIntOpt(obj, tag)
            if obj.readOptional(tag, Ice.OptionalFormat.F4)
                r = obj.readInt();
            else
                r = Ice.Unset;
            end
        end
        function r = readIntSeqOpt(obj, tag)
            if obj.readOptional(tag, Ice.OptionalFormat.VSize)
                obj.skipSize();
                r = obj.readIntSeq();
            else
                r = Ice.Unset;
            end
        end
        function r = readLong(obj)
            v = libpointer('int64Ptr', 0);
            obj.call_('readLong', v);
            r = v.Value;
        end
        function r = readLongSeq(obj)
            r = obj.callWithResult_('readLongSeq');
        end
        function r = readLongOpt(obj, tag)
            if obj.readOptional(tag, Ice.OptionalFormat.F8)
                r = obj.readLong();
            else
                r = Ice.Unset;
            end
        end
        function r = readLongSeqOpt(obj, tag)
            if obj.readOptional(tag, Ice.OptionalFormat.VSize)
                obj.skipSize();
                r = obj.readLongSeq();
            else
                r = Ice.Unset;
            end
        end
        function r = readFloat(obj)
            v = libpointer('singlePtr', 0);
            obj.call_('readFloat', v);
            r = v.Value;
        end
        function r = readFloatSeq(obj)
            r = obj.callWithResult_('readFloatSeq');
        end
        function r = readFloatOpt(obj, tag)
            if obj.readOptional(tag, Ice.OptionalFormat.F4)
                r = obj.readFloat();
            else
                r = Ice.Unset;
            end
        end
        function r = readFloatSeqOpt(obj, tag)
            if obj.readOptional(tag, Ice.OptionalFormat.VSize)
                obj.skipSize();
                r = obj.readFloatSeq();
            else
                r = Ice.Unset;
            end
        end
        function r = readDouble(obj)
            v = libpointer('doublePtr', 0);
            obj.call_('readDouble', v);
            r = v.Value;
        end
        function r = readDoubleSeq(obj)
            r = obj.callWithResult_('readDoubleSeq');
        end
        function r = readDoubleOpt(obj, tag)
            if obj.readOptional(tag, Ice.OptionalFormat.F8)
                r = obj.readDouble();
            else
                r = Ice.Unset;
            end
        end
        function r = readDoubleSeqOpt(obj, tag)
            if obj.readOptional(tag, Ice.OptionalFormat.VSize)
                obj.skipSize();
                r = obj.readDoubleSeq();
            else
                r = Ice.Unset;
            end
        end
        function r = readString(obj)
            r = obj.callWithResult_('readString');
        end
        function r = readStringSeq(obj)
            r = obj.callWithResult_('readStringSeq');
        end
        function r = readStringOpt(obj, tag)
            if obj.readOptional(tag, Ice.OptionalFormat.VSize)
                r = obj.readString();
            else
                r = Ice.Unset;
            end
        end
        function r = readStringSeqOpt(obj, tag)
            if obj.readOptional(tag, Ice.OptionalFormat.FSize)
                obj.skip(4);
                r = obj.readStringSeq();
            else
                r = Ice.Unset;
            end
        end
        function skip(obj, n)
            obj.call_('skip', n);
        end
        function skipSize(obj)
            obj.call_('skipSize');
        end
        function startException(obj)
            assert(~isempty(obj.currentEncaps) && ~isempty(obj.currentEncaps.decoder));
            obj.currentEncaps.decoder.startInstance(IceInternal.SliceType.ExceptionSlice);
        end
        function r = endException(obj, preserve)
            assert(~isempty(obj.currentEncaps) && ~isempty(obj.currentEncaps.decoder));
            r = obj.currentEncaps.decoder.endInstance(preserve);
        end
        function startEncapsulation(obj)
            oldEncaps = obj.currentEncaps;
            if isempty(oldEncaps) % First allocated encaps?
                obj.currentEncaps = IceInternal.ReadEncaps();
            else
                obj.currentEncaps = IceInternal.ReadEncaps();
                obj.currentEncaps.previous = oldEncaps;
            end
            obj.currentEncaps.start = obj.pos();

            %
            % I don't use readSize() and writeSize() for encapsulations,
            % because when creating an encapsulation, I must know in advance
            % how many bytes the size information will require in the data
            % stream. If I use an Int, it is always 4 bytes. For
            % readSize()/writeSize(), it could be 1 or 5 bytes.
            %
            sz = uint32(obj.readInt());
            if sz < 6
                throw(Ice.UnmarshalOutOfBoundsException());
            end
            if obj.pos() - 4 + sz > obj.size()
                throw(Ice.UnmarshalOutOfBoundsException());
            end
            obj.currentEncaps.sz = sz;

            obj.currentEncaps.encoding = Ice.EncodingVersion.ice_read(obj);
            % TODO
            %IceInternal::checkSupportedEncoding(_currentEncaps->encoding); // Make sure the encoding is supported

            r = obj.currentEncaps.encoding;
        end
        function endEncapsulation(obj)
            assert(~isempty(obj.currentEncaps));

            if ~isequal(obj.currentEncaps.encoding, IceInternal.Protocol.Encoding_1_0)
                obj.skipOptionals();
                if obj.pos() ~= obj.currentEncaps.start + obj.currentEncaps.sz
                    throw(Ice.EncapsulationException());
                end
            elseif obj.pos() ~= obj.currentEncaps.start + obj.currentEncaps.sz
                if obj.pos() + 1 ~= obj.currentEncaps.start + obj.currentEncaps.sz
                    throw(Ice.EncapsulationException());
                end

                %
                % Ice version < 3.3 had a bug where user exceptions with
                % class members could be encoded with a trailing byte
                % when dispatched with AMD. So we tolerate an extra byte
                % in the encapsulation.
                %
                obj.skip(1);
            end

            oldEncaps = obj.currentEncaps;
            obj.currentEncaps = obj.currentEncaps.previous;
        end
        function r = skipEmptyEncapsulation(obj)
            sz = uint32(obj.readInt());
            if sz < 6
                throw(Ice.EncapsulationException());
            end
            if obj.pos() - 4 + sz > obj.size()
                throw(Ice.UnmarshalOutOfBoundsException());
            end
            encoding = Ice.EncodingVersion.ice_read(obj);
            %IceInternal::checkSupportedEncoding(encoding); // Make sure the encoding is supported % TODO

            if isequal(encoding, IceInternal.Protocol.Encoding_1_0)
                if sz ~= 6
                    throw(Ice.EncapsulationException());
                end
            else
                % Skip the optional content of the encapsulation if we are expecting an
                % empty encapsulation.
                obj.skip(sz - 6);
            end
            r = encoding;
        end
        function skipEncapsulation(obj)
            obj.call_('skipEncapsulation');
        end
        function r = getEncoding(obj)
            if isempty(obj.currentEncaps)
                r = obj.encoding;
            else
                r = obj.currentEncaps.encoding;
            end
        end
        function r = startSlice(obj)
            assert(~isempty(obj.currentEncaps) && ~isempty(obj.currentEncaps.decoder));
            r = obj.currentEncaps.decoder.startSlice();
        end
        function endSlice(obj)
            assert(~isempty(obj.currentEncaps) && ~isempty(obj.currentEncaps.decoder));
            obj.currentEncaps.decoder.endSlice();
        end
        function skipSlice(obj)
            assert(~isempty(obj.currentEncaps) && ~isempty(obj.currentEncaps.decoder));
            obj.currentEncaps.decoder.skipSlice();
        end
        function r = readSize(obj)
            v = libpointer('int32Ptr', 0);
            obj.call_('readSize', v);
            r = v.Value;
        end
        function r = readOptional(obj, tag, fmt)
            assert(~isempty(obj.currentEncaps));
            if ~isempty(obj.currentEncaps.decoder)
                r = obj.currentEncaps.decoder.readOptional(tag, fmt);
            else
                r = obj.readOptionalImpl(tag, fmt);
            end
        end
        function skipOptionals(obj)
            %
            % Skip remaining un-read optional members.
            %
            while true
                if obj.pos() >= obj.currentEncaps.start + obj.currentEncaps.sz
                    return; % End of encapsulation also indicates end of optionals.
                end

                v = obj.readByte();
                if v == IceInternal.Protocol.OPTIONAL_END_MARKER
                    return;
                end

                format = bitand(v, 7); % Read first 3 bits.
                if bitshift(v, 3) == 30
                    obj.skipSize();
                end
                obj.skipOptional(format);
            end
        end
        function skipOptional(format)
            switch format
                case Ice.OptionalFormat.F1
                    obj.skip(1);
                case Ice.OptionalFormat.F2
                    obj.skip(2);
                case Ice.OptionalFormat.F4
                    obj.skip(4);
                case Ice.OptionalFormat.F8
                    obj.skip(8);
                case Ice.OptionalFormat.Size
                    obj.skipSize();
                case Ice.OptionalFormat.VSize
                    obj.skip(obj.readSize());
                case Ice.OptionalFormat.FSize
                    sz = obj.readInt();
                    if sz < 0
                        throw(Ice.UnmarshalOutOfBoundsException());
                    end
                    obj.skip(sz);
                case Ice.OptionalFormat.Class
                    obj.readValue(); % TODO
            end
        end
        function r = readProxy(obj)
            v = libpointer('voidPtr');
            obj.call_('readProxy', v);
            if ~isNull(v)
                r = Ice.ObjectPrx(v, obj.communicator);
            else
                r = [];
            end
        end
        function r = readProxyOpt(obj, tag)
            if obj.readOptional(tag, Ice.OptionalFormat.FSize)
                obj.skip(4);
                r = obj.readProxy();
            else
                r = Ice.Unset;
            end
        end
        function r = readEnum(obj, maxValue)
            v = libpointer('int32Ptr', 0);
            obj.call_('readEnum', maxValue, v);
            r = v.Value;
        end
        function readValue(obj, cb, formalType)
            obj.initEncaps();
            function check(v)
                if isempty(v) || isa(v, formalType)
                    cb(v);
                else
                    Ice.InputStream.throwUOE(formalType, v);
                end
            end
            if isempty(cb)
                obj.currentEncaps.decoder.readValue([]);
            else
                obj.currentEncaps.decoder.readValue(@(v) check(v));
            end
        end
        function readValueOpt(obj, tag, cb, formalType)
            if obj.readOptional(tag, Ice.OptionalFormat.Class)
                obj.readValue(cb, formalType);
            elseif ~isempty(cb)
                cb(Ice.Unset);
            end
        end
        function readPendingValues(obj)
            if ~isempty(obj.currentEncaps) && ~isempty(obj.currentEncaps.decoder)
                obj.currentEncaps.decoder.readPendingValues();
            elseif isequal(obj.getEncoding(), IceInternal.Protocol.Encoding_1_0)
                %
                % If using the 1.0 encoding and no instances were read, we
                % still read an empty sequence of pending instances if
                % requested (i.e.: if this is called).
                %
                % This is required by the 1.0 encoding, even if no instances
                % are written we do marshal an empty sequence if marshaled
                % data types use classes.
                %
                obj.skipSize();
            end
        end
        function startValue(obj)
            assert(~isempty(obj.currentEncaps) && ~isempty(obj.currentEncaps.decoder));
            obj.currentEncaps.decoder.startInstance(IceInternal.SliceType.ValueSlice);
        end
        function r = endValue(obj, preserve)
            assert(~isempty(obj.currentEncaps) && ~isempty(obj.currentEncaps.decoder));
            r = obj.currentEncaps.decoder.endInstance(preserve);
        end
        function throwException(obj)
            obj.initEncaps();
            obj.currentEncaps.decoder.throwException();
        end
        function r = pos(obj)
            v = libpointer('uint32Ptr', 0);
            obj.call_('pos', v);
            r = v.Value;
        end
        function setPos(obj, pos)
            obj.call_('setPos', pos);
        end
        function r = size(obj)
            v = libpointer('uint32Ptr', 0);
            obj.call_('size', v);
            r = v.Value;
        end
        function r = readOptionalImpl(obj, readTag, expectedFormat)
            if isequal(obj.getEncoding(), IceInternal.Protocol.Encoding_1_0)
                r = false; % Optional members aren't supported with the 1.0 encoding.
                return;
            end

            while true
                if obj.pos() >= obj.currentEncaps.start + obj.currentEncaps.sz
                    r = false; % End of encapsulation also indicates end of optionals.
                    return;
                end

                b = obj.readByte();
                v = b;
                if v == IceInternal.Protocol.OPTIONAL_END_MARKER
                    obj.setPos(obj.pos() - 1); % Rewind.
                    r = false;
                    return;
                end

                format = bitand(v, 7); % First 3 bits.
                tag = bitshift(v, -3);
                if tag == 30
                    tag = obj.readSize();
                end

                if tag > readTag
                    offset = 0;
                    if tag < 30
                        offset = 1;
                    elseif tag < 255
                        offset = 2;
                    else
                        offset = 6;
                    end
                    obj.setPos(obj.pos() - offset); % Rewind
                    r = false; % No optional data members with the requested tag.
                    return;
                elseif tag < readTag
                    obj.skipOptional(format); % Skip optional data members
                else
                    if format ~= expectedFormat
                        throw(Ice.MarshalException('', '', ...
                                sprintf('invalid optional data member ''%d'': unexpected format', tag)));
                    end
                    r = true;
                    return;
                end
            end
        end
        function r = getBytes(obj, startPos, endPos)
            r = obj.callWithResult_('getBytes', startPos, endPos);
        end
        function r = readAndCheckSeqSize(obj, minSize)
            v = libpointer('int32Ptr', 0);
            obj.call_('readAndCheckSeqSize', minSize, v);
            r = v.Value;
        end
    end
    methods(Access=private)
        function initEncaps(obj)
            if isempty(obj.currentEncaps)
                obj.currentEncaps = IceInternal.ReadEncaps();
                obj.currentEncaps.encoding = obj.encoding;
            end

            valueFactoryManager = obj.communicator.getValueFactoryManager();
            compactIdResolver = obj.communicator.getCompactIdResolver();
            if isempty(obj.currentEncaps.decoder)
                if obj.currentEncaps.encoding.major == 1 && obj.currentEncaps.encoding.minor == 0
                    obj.currentEncaps.decoder = ...
                        IceInternal.EncapsDecoder10(obj, obj.currentEncaps, obj.sliceValues, valueFactoryManager, ...]
                                                    obj.communicator.getClassResolver());
                else
                    obj.currentEncaps.decoder = ...
                        IceInternal.EncapsDecoder11(obj, obj.currentEncaps, obj.sliceValues, valueFactoryManager, ...
                                                    obj.communicator.getClassResolver(), compactIdResolver);
                end
            end
        end
    end
    methods(Static,Access=private)
        function throwUOE(expectedType, v)
            %
            % If the object is an unknown sliced object, we didn't find a
            % value factory, in this case raise a NoValueFactoryException
            % instead.
            %
            if isa(v, 'Ice.UnknownSlicedValue')
                throw(Ice.NoValueFactoryException('', '', '', v.ice_id()));
            end

            type = class(v);
            throw(Ice.UnexpectedObjectException('', '', ...
                sprintf('expected element of type ''%s'' but received ''%s''', expectedType, type), type, ...
                expectedType));
        end
    end
    properties(Access=private)
        communicator
        encoding
        currentEncaps
        sliceValues
    end
end
