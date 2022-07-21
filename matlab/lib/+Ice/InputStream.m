classdef InputStream < handle
    % InputStream   Summary of InputStream
    %
    % Unmarshals Slice types.

    % Copyright (c) ZeroC, Inc. All rights reserved.

    methods
        function obj = InputStream(communicator, encoding, buf)
            obj.communicator = communicator;
            obj.buf = buf;
            obj.pos = int32(1);
            obj.encoding = encoding;
            obj.encoding_1_0 = encoding.major == 1 && encoding.minor == 0;
            obj.size = length(buf);
            obj.classGraphDepthMax = ...
                communicator.getProperties().getPropertyAsIntWithDefault('Ice.ClassGraphDepthMax', 100);
            if obj.classGraphDepthMax < 1 || obj.classGraphDepthMax > intmax('int32')
                obj.classGraphDepthMax = intmax('int32');
            end
        end
        function r = getCommunicator(obj)
            r = obj.communicator;
        end
        function reset(obj, data)
            obj.buf = data;
            obj.pos = int32(1);
            obj.size = length(data);
            obj.startSeq = -1;
            obj.minSeqSize = 0;
            obj.encapsStack = [];
            obj.encapsStackDecoder = [];
        end
        function r = readBool(obj)
            pos = obj.pos;
            if pos > obj.size
                throw(Ice.UnmarshalOutOfBoundsException());
            end
            if obj.buf(pos) == 0
                r = false;
            else
                r = true;
            end
            obj.pos = pos + 1;
        end
        function r = readBoolSeq(obj)
            sz = obj.readAndCheckSeqSize(1);
            if sz == 0
                r = logical([]);
            else
                pos = obj.pos;
                if pos + sz - 1 > obj.size
                    throw(Ice.UnmarshalOutOfBoundsException());
                end
                r = logical(obj.buf(pos:pos + sz - 1));
                obj.pos = pos + sz;
            end
        end
        function r = readBoolOpt(obj, tag)
            if obj.readOptional(tag, Ice.OptionalFormat.F1)
                r = obj.readBool();
            else
                r = IceInternal.UnsetI.Instance;
            end
        end
        function r = readBoolSeqOpt(obj, tag)
            if obj.readOptional(tag, Ice.OptionalFormat.VSize)
                r = obj.readBoolSeq();
            else
                r = IceInternal.UnsetI.Instance;
            end
        end
        function r = readByte(obj)
            pos = obj.pos;
            if pos > obj.size
                throw(Ice.UnmarshalOutOfBoundsException());
            end
            r = obj.buf(pos);
            obj.pos = pos + 1;
        end
        function r = readByteSeq(obj)
            sz = obj.readAndCheckSeqSize(1);
            if sz == 0
                r = uint8([]);
            else
                pos = obj.pos;
                if pos + sz - 1 > obj.size
                    throw(Ice.UnmarshalOutOfBoundsException());
                end
                r = obj.buf(pos:pos + sz - 1);
                obj.pos = pos + sz;
            end
        end
        function r = readByteOpt(obj, tag)
            if obj.readOptional(tag, Ice.OptionalFormat.F1)
                r = obj.readByte();
            else
                r = IceInternal.UnsetI.Instance;
            end
        end
        function r = readByteSeqOpt(obj, tag)
            if obj.readOptional(tag, Ice.OptionalFormat.VSize)
                r = obj.readByteSeq();
            else
                r = IceInternal.UnsetI.Instance;
            end
        end
        function r = readShort(obj)
            pos = obj.pos;
            if pos + 1 > obj.size
                throw(Ice.UnmarshalOutOfBoundsException());
            end
            r = typecast(uint8(obj.buf(pos:pos + 1)), 'int16');
            obj.pos = pos + 2;
        end
        function r = readShortSeq(obj)
            sz = obj.readAndCheckSeqSize(2);
            if sz == 0
                r = int16([]);
            else
                pos = obj.pos;
                if pos + sz * 2 - 1 > obj.size
                    throw(Ice.UnmarshalOutOfBoundsException());
                end
                r = typecast(uint8(obj.buf(pos:pos + sz * 2 - 1)), 'int16');
                obj.pos = pos + sz * 2;
            end
        end
        function r = readShortOpt(obj, tag)
            if obj.readOptional(tag, Ice.OptionalFormat.F2)
                r = obj.readShort();
            else
                r = IceInternal.UnsetI.Instance;
            end
        end
        function r = readShortSeqOpt(obj, tag)
            if obj.readOptional(tag, Ice.OptionalFormat.VSize)
                obj.skipSize();
                r = obj.readShortSeq();
            else
                r = IceInternal.UnsetI.Instance;
            end
        end
        function r = readInt(obj)
            pos = obj.pos;
            if pos + 3 > obj.size
                throw(Ice.UnmarshalOutOfBoundsException());
            end
            r = typecast(uint8(obj.buf(pos:pos + 3)), 'int32');
            obj.pos = pos + 4;
        end
        function r = readIntSeq(obj)
            sz = obj.readAndCheckSeqSize(4);
            if sz == 0
                r = int32([]);
            else
                pos = obj.pos;
                if pos + sz * 4 - 1 > obj.size
                    throw(Ice.UnmarshalOutOfBoundsException());
                end
                r = typecast(uint8(obj.buf(pos:pos + sz * 4 - 1)), 'int32');
                obj.pos = pos + sz * 4;
            end
        end
        function r = readIntOpt(obj, tag)
            if obj.readOptional(tag, Ice.OptionalFormat.F4)
                r = obj.readInt();
            else
                r = IceInternal.UnsetI.Instance;
            end
        end
        function r = readIntSeqOpt(obj, tag)
            if obj.readOptional(tag, Ice.OptionalFormat.VSize)
                obj.skipSize();
                r = obj.readIntSeq();
            else
                r = IceInternal.UnsetI.Instance;
            end
        end
        function r = readLong(obj)
            pos = obj.pos;
            if pos + 7 > obj.size
                throw(Ice.UnmarshalOutOfBoundsException());
            end
            r = typecast(uint8(obj.buf(pos:pos + 7)), 'int64');
            obj.pos = pos + 8;
        end
        function r = readLongSeq(obj)
            sz = obj.readAndCheckSeqSize(8);
            if sz == 0
                r = int64([]);
            else
                pos = obj.pos;
                if pos + sz * 8 - 1 > obj.size
                    throw(Ice.UnmarshalOutOfBoundsException());
                end
                r = typecast(uint8(obj.buf(pos:pos + sz * 8 - 1)), 'int64');
                obj.pos = pos + sz * 8;
            end
        end
        function r = readLongOpt(obj, tag)
            if obj.readOptional(tag, Ice.OptionalFormat.F8)
                r = obj.readLong();
            else
                r = IceInternal.UnsetI.Instance;
            end
        end
        function r = readLongSeqOpt(obj, tag)
            if obj.readOptional(tag, Ice.OptionalFormat.VSize)
                obj.skipSize();
                r = obj.readLongSeq();
            else
                r = IceInternal.UnsetI.Instance;
            end
        end
        function r = readFloat(obj)
            pos = obj.pos;
            if pos + 3 > obj.size
                throw(Ice.UnmarshalOutOfBoundsException());
            end
            r = typecast(uint8(obj.buf(pos:pos + 3)), 'single');
            obj.pos = pos + 4;
        end
        function r = readFloatSeq(obj)
            sz = obj.readAndCheckSeqSize(4);
            if sz == 0
                r = single([]);
            else
                pos = obj.pos;
                if pos + sz * 4 - 1 > obj.size
                    throw(Ice.UnmarshalOutOfBoundsException());
                end
                r = typecast(uint8(obj.buf(pos:pos + sz * 4 - 1)), 'single');
                obj.pos = pos + sz * 4;
            end
        end
        function r = readFloatOpt(obj, tag)
            if obj.readOptional(tag, Ice.OptionalFormat.F4)
                r = obj.readFloat();
            else
                r = IceInternal.UnsetI.Instance;
            end
        end
        function r = readFloatSeqOpt(obj, tag)
            if obj.readOptional(tag, Ice.OptionalFormat.VSize)
                obj.skipSize();
                r = obj.readFloatSeq();
            else
                r = IceInternal.UnsetI.Instance;
            end
        end
        function r = readDouble(obj)
            pos = obj.pos;
            if pos + 7 > obj.size
                throw(Ice.UnmarshalOutOfBoundsException());
            end
            r = typecast(uint8(obj.buf(pos:pos + 7)), 'double');
            obj.pos = pos + 8;
        end
        function r = readDoubleSeq(obj)
            sz = obj.readAndCheckSeqSize(8);
            if sz == 0
                r = double([]);
            else
                pos = obj.pos;
                if pos + sz * 8 - 1 > obj.size
                    throw(Ice.UnmarshalOutOfBoundsException());
                end
                r = typecast(uint8(obj.buf(pos:pos + sz * 8 - 1)), 'double');
                obj.pos = pos + sz * 8;
            end
        end
        function r = readDoubleOpt(obj, tag)
            if obj.readOptional(tag, Ice.OptionalFormat.F8)
                r = obj.readDouble();
            else
                r = IceInternal.UnsetI.Instance;
            end
        end
        function r = readDoubleSeqOpt(obj, tag)
            if obj.readOptional(tag, Ice.OptionalFormat.VSize)
                obj.skipSize();
                r = obj.readDoubleSeq();
            else
                r = IceInternal.UnsetI.Instance;
            end
        end
        function r = readString(obj)
            sz = obj.readSize();
            if sz == 0
                r = '';
            else
                pos = obj.pos;
                if pos + sz - 1 > obj.size
                    throw(Ice.UnmarshalOutOfBoundsException());
                end
                r = native2unicode(obj.buf(pos:pos + sz - 1), 'utf-8');
                obj.pos = pos + sz;
            end
        end
        function r = readStringSeq(obj)
            sz = obj.readAndCheckSeqSize(1);
            r = cell(1, sz);
            for i = 1:sz
                r{i} = obj.readString();
            end
        end
        function r = readStringOpt(obj, tag)
            if obj.readOptional(tag, Ice.OptionalFormat.VSize)
                r = obj.readString();
            else
                r = IceInternal.UnsetI.Instance;
            end
        end
        function r = readStringSeqOpt(obj, tag)
            if obj.readOptional(tag, Ice.OptionalFormat.FSize)
                obj.skip(4);
                r = obj.readStringSeq();
            else
                r = IceInternal.UnsetI.Instance;
            end
        end
        function skip(obj, n)
            pos = obj.pos;
            if n < 0 || pos + n > obj.size + 1
                throw(Ice.UnmarshalOutOfBoundsException());
            end
            obj.pos = pos + n;
        end
        function skipSize(obj)
            b = obj.readByte();
            if b == 255
                obj.skip(4);
            end
        end
        function startException(obj)
            %assert(isobject(obj.encapsStackDecoder));
            obj.encapsStackDecoder.startInstance(IceInternal.SliceType.ExceptionSlice);
        end
        function r = endException(obj, preserve)
            %assert(isobject(obj.encapsStackDecoder));
            r = obj.encapsStackDecoder.endInstance(preserve);
        end
        function startEncapsulation(obj)
            curr = obj.encapsCache;
            if isobject(curr)
                curr.reset();
                obj.encapsCache = obj.encapsCache.next;
            else
                curr = IceInternal.ReadEncaps();
            end
            curr.next = obj.encapsStack;
            obj.encapsStack = curr;
            obj.encapsStackDecoder = curr.decoder;
            pos = obj.pos;

            %
            % I don't use readSize() for encapsulations, because when creating an encapsulation,
            % I must know in advance how many bytes the size information will require in the data
            % stream. If I use an Int, it is always 4 bytes. For readSize(), it could be 1 or 5 bytes.
            %
            sz = obj.readInt();
            if sz < 6
                throw(Ice.UnmarshalOutOfBoundsException());
            end
            if obj.pos - 4 + sz > obj.size + 1
                throw(Ice.UnmarshalOutOfBoundsException());
            end
            obj.encapsStack.endPos = pos + sz;

            %
            % Inline the encoding version unmarshaling
            %
            % encoding = Ice.EncodingVersion.ice_read(obj);
            %
            pos = obj.pos;
            obj.encoding.major = obj.buf(pos);
            obj.encoding.minor = obj.buf(pos + 1);
            obj.pos = pos + 2;

            %
            % NOTE: Hardcoding the current encoding version at 1.1
            %
            % IceInternal.Protocol.checkSupportedEncoding(encoding); % Make sure the encoding is supported
            %
            if obj.encoding.major ~= 1 || (obj.encoding.minor ~= 0 && obj.encoding.minor ~= 1)
                throw(Ice.UnsupportedEncodingException('', '', '', obj.encoding, Ice.EncodingVersion(1, 1)));
            end

            obj.encapsStack.setEncoding(obj.encoding);
            obj.encoding_1_0 = obj.encapsStack.encoding_1_0;
        end
        function endEncapsulation(obj)
            %assert(isobject(obj.encapsStack));

            if ~obj.encoding_1_0
                obj.skipOptionals();
                if obj.pos ~= obj.encapsStack.endPos
                    throw(Ice.EncapsulationException());
                end
            elseif obj.pos ~= obj.encapsStack.endPos
                if obj.pos + 1 ~= obj.encapsStack.endPos
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

            %
            % Give the decoder a chance to clean up.
            %
            if isobject(obj.encapsStackDecoder)
                obj.encapsStackDecoder.finish();
            end

            curr = obj.encapsStack;
            obj.encapsStack = curr.next;
            if isobject(obj.encapsStack)
                obj.encapsStackDecoder = obj.encapsStack.decoder;
            end
            curr.next = obj.encapsCache;
            obj.encapsCache = curr;
            obj.encapsCache.reset();

            if isobject(obj.encapsStack)
                obj.encoding_1_0 = obj.encapsStack.encoding_1_0;
            else
                obj.encoding_1_0 = obj.encoding.major == 1 && obj.encoding.minor == 0;
            end
        end
        function skipEmptyEncapsulation(obj)
            sz = obj.readInt();
            if sz < 6
                throw(Ice.EncapsulationException());
            end
            if obj.pos - 4 + sz > obj.size + 1
                throw(Ice.UnmarshalOutOfBoundsException());
            end

            %
            % Inline the encoding version unmarshaling
            %
            % encoding = Ice.EncodingVersion.ice_read(obj);
            %
            pos = obj.pos;
            major = obj.buf(pos);
            minor = obj.buf(pos + 1);
            obj.pos = pos + 2;

            %
            % NOTE: Hardcoding the current encoding version at 1.1
            %
            % IceInternal.Protocol.checkSupportedEncoding(encoding); % Make sure the encoding is supported
            %
            if major ~= 1 || (minor ~= 0 && minor ~= 1)
                throw(Ice.UnsupportedEncodingException('', '', '', Ice.EncodingVersion(major, minor), ...
                                                       Ice.EncodingVersion(1, 1)));
            end

            if major == 1 && minor == 0
                if sz ~= 6
                    throw(Ice.EncapsulationException());
                end
            else
                %
                % Skip the optional content of the encapsulation if we are expecting an empty encapsulation.
                %
                % obj.skip(sz - 6);
                %
                obj.pos = obj.pos + sz - 6; % No need to validate the size again.
            end
        end
        function skipEncapsulation(obj)
            sz = obj.readInt();
            if sz < 6
                throw(Ice.EncapsulationException());
            end
            if obj.pos - 4 + sz > obj.size + 1
                throw(Ice.UnmarshalOutOfBoundsException());
            end

            obj.pos = obj.pos + sz - 4;
        end
        function r = getEncoding(obj)
            if ~isobject(obj.encapsStack)
                r = obj.encoding;
            else
                r = obj.encapsStack.encoding;
            end
        end
        function r = startSlice(obj)
            %assert(isobject(obj.encapsStackDecoder));
            r = obj.encapsStackDecoder.startSlice();
        end
        function endSlice(obj)
            %assert(isobject(obj.encapsStackDecoder));
            obj.encapsStackDecoder.endSlice();
        end
        function skipSlice(obj)
            %assert(isobject(obj.encapsStackDecoder));
            obj.encapsStackDecoder.skipSlice();
        end
        function r = readSize(obj)
            pos = obj.pos;
            if pos > obj.size
                throw(Ice.UnmarshalOutOfBoundsException());
            end
            b = obj.buf(pos);
            pos = pos + 1;
            if b == 255
                if pos + 3 > obj.size
                    throw(Ice.UnmarshalOutOfBoundsException());
                end
                r = typecast(uint8(obj.buf(pos:pos + 3)), 'int32');
                obj.pos = pos + 4;
                if r < 0
                    throw(Ice.UnmarshalOutOfBoundsException());
                end
            else
                obj.pos = pos;
                r = int32(b);
            end
        end
        function r = readOptional(obj, tag, fmt)
            %assert(isobject(obj.encapsStack));
            if obj.encoding_1_0
                r = false; % Optional members aren't supported with the 1.0 encoding.
            elseif isobject(obj.encapsStackDecoder)
                import IceInternal.Protocol;
                current = obj.encapsStackDecoder.current;
                if ~isobject(current) || bitand(current.sliceFlags, Protocol.FLAG_HAS_OPTIONAL_MEMBERS)
                    r = readOptionalImpl(tag, fmt, obj.encapsStack.endPos);
                else
                    r = false;
                end
            else
                r = readOptionalImpl(tag, fmt, obj.size);
            end

            function r = readOptionalImpl(readTag, expectedFormat, encapsStackEnd)
                while true
                    pos = obj.pos;
                    if pos >= encapsStackEnd
                        r = false; % End of encapsulation also indicates end of optionals.
                        break;
                    end

                    v = obj.buf(pos);
                    if v == 255 %IceInternal.Protocol.OPTIONAL_END_MARKER
                        r = false;
                        break;
                    end
                    obj.pos = pos + 1;

                    format = bitand(v, uint8(7)); % First 3 bits.
                    tag = uint32(bitshift(v, -3));
                    if tag == 30
                        tag = obj.readSize();
                    end

                    if tag > readTag
                        if tag < 30
                            offset = 1;
                        elseif tag < 255
                            offset = 2;
                        else
                            offset = 6;
                        end
                        obj.pos = obj.pos - offset; % Rewind
                        r = false; % No optional data members with the requested tag.
                        break;
                    elseif tag < readTag
                        obj.skipOptional(format); % Skip optional data members
                    elseif format ~= expectedFormat
                        throw(Ice.MarshalException('', '', ...
                                sprintf('invalid optional data member ''%d'': unexpected format', tag)));
                    else
                        r = true;
                        break;
                    end
                end
            end
        end
        function r = skipValue(obj, pos)
            obj.pos = pos(0)
            obj.readValue([], '');
            r = obj.pos;
        end
        function skipOptionals(obj)
            %
            % Skip remaining unread optional members.
            %
            encapsStackEnd = obj.encapsStack.endPos;
            while true
                if obj.pos >= encapsStackEnd
                    return; % End of encapsulation also indicates end of optionals.
                end

                v = obj.readByte();
                if v == IceInternal.Protocol.OPTIONAL_END_MARKER
                    return;
                end

                format = bitand(v, uint8(7)); % Read first 3 bits.
                if bitshift(v, 3) == uint8(30)
                    obj.skipSize();
                end
                obj.skipOptional(format);
            end
        end
        function skipOptional(obj, format)
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
                    obj.readValue([], '');
            end
        end
        function r = readProxy(obj, cls)
            %
            % Manually unmarshal a proxy just to discover how many bytes it consumes.
            %

            start = obj.pos;

            id = Ice.Identity.ice_read(obj);

            %
            % A nil proxy is marshaled as an identity with empty category and name.
            %
            if isempty(id.category) && isempty(id.name)
                r = [];
                return;
            end

            obj.readStringSeq();
            obj.readByte();
            obj.readBool();

            %
            % The versions are only included in encoding >= 1.1.
            %
            if ~obj.encoding_1_0
                Ice.ProtocolVersion.ice_read(obj);
                Ice.EncodingVersion.ice_read(obj);
            end

            numEndpoints = obj.readSize();

            if numEndpoints > 0
                for i = 1:numEndpoints
                    obj.readShort();
                    obj.skipEncapsulation();
                end
            else
                obj.readString();
            end

            %
            % Now that we've reached the end, extract all of the bytes representing the marshaled form of the proxy.
            %
            bytes = obj.buf(start:obj.pos - 1);

            if nargin == 2
                %
                % Instantiate a proxy of the requested type.
                %
                constructor = str2func(cls);
                r = constructor(obj.communicator, obj.getEncoding(), [], bytes);
            else
                r = Ice.ObjectPrx(obj.communicator, obj.getEncoding(), [], bytes);
            end
        end
        function r = readProxyOpt(obj, tag)
            if obj.readOptional(tag, Ice.OptionalFormat.FSize)
                obj.skip(4);
                r = obj.readProxy();
            else
                r = IceInternal.UnsetI.Instance;
            end
        end
        function r = readEnum(obj, maxValue)
            if obj.encoding_1_0
                if maxValue < 127
                    r = obj.readByte();
                elseif maxValue < 32767
                    r = obj.readShort();
                else
                    r = obj.readInt();
                end
            else
                r = obj.readSize();
            end
        end
        function readValue(obj, cb, formalType)
            if ~isobject(obj.encapsStackDecoder)
                obj.initEncaps();
            end
            function check(v)
                if isempty(v) || isa(v, formalType)
                    cb(v);
                else
                    Ice.InputStream.throwUOE(formalType, v);
                end
            end
            if isempty(cb)
                obj.encapsStackDecoder.readValue([]);
            else
                obj.encapsStackDecoder.readValue(@(v) check(v));
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
            if isobject(obj.encapsStackDecoder)
                obj.encapsStackDecoder.readPendingValues();
            elseif obj.encoding_1_0
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
            %assert(isobject(obj.encapsStackDecoder));
            obj.encapsStackDecoder.startInstance(IceInternal.SliceType.ValueSlice);
        end
        function r = endValue(obj, preserve)
            %assert(isobject(obj.encapsStackDecoder));
            r = obj.encapsStackDecoder.endInstance(preserve);
        end
        function throwException(obj)
            if ~isobject(obj.encapsStackDecoder)
                obj.initEncaps();
            end
            obj.encapsStackDecoder.throwException();
        end
        function r = getPos(obj)
            r = obj.pos;
        end
        function setPos(obj, pos)
            obj.pos = pos;
        end
        function r = getSize(obj)
            r = obj.size;
        end
        function r = getBytes(obj, startPos, endPos) % The start and end positions are inclusive
            if startPos > obj.size || endPos > obj.size
                throw(Ice.UnmarshalOutOfBoundsException());
            elseif startPos < 1 || endPos < 1 || endPos < startPos
                throw(Ice.MarshalException());
            end
            r = obj.buf(startPos:endPos);
        end
        function r = readAndCheckSeqSize(obj, minSize)
            sz = obj.readSize();

            if sz == 0
                r = sz;
                return;
            end

            %
            % The _startSeq variable points to the start of the sequence for which
            % we expect to read at least _minSeqSize bytes from the stream.
            %
            % If not initialized or if we already read more data than _minSeqSize,
            % we reset _startSeq and _minSeqSize for this sequence (possibly a
            % top-level sequence or enclosed sequence it doesn't really matter).
            %
            % Otherwise, we are reading an enclosed sequence and we have to bump
            % _minSeqSize by the minimum size that this sequence will  require on
            % the stream.
            %
            % The goal of this check is to ensure that when we start un-marshalling
            % a new sequence, we check the minimal size of this new sequence against
            % the estimated remaining buffer size. This estimation is based on
            % the minimum size of the enclosing sequences, it's _minSeqSize.
            %
            if obj.startSeq == -1 || obj.pos > (obj.startSeq + obj.minSeqSize - 1)
                obj.startSeq = obj.pos;
                obj.minSeqSize = int32(sz * minSize);
            else
                obj.minSeqSize = int32(obj.minSeqSize + sz * minSize);
            end

            %
            % If there isn't enough data to read on the stream for the sequence (and
            % possibly enclosed sequences), something is wrong with the marshalled
            % data: it's claiming to have more data than what is possible to read.
            %
            if obj.startSeq + obj.minSeqSize > obj.size + int32(1)
                throw(Ice.UnmarshalOutOfBoundsException());
            end

            r = sz;
        end
    end
    methods(Access=private)
        function initEncaps(obj)
            if ~isobject(obj.encapsStack) % Lazy initialization
                obj.encapsStack = obj.encapsCache;
                if isobject(obj.encapsStack)
                    obj.encapsCache = obj.encapsCache.next;
                else
                    obj.encapsStack = IceInternal.ReadEncaps();
                end
                obj.encapsStack.setEncoding(obj.encoding);
                obj.encapsStack.endPos = obj.size;
            end

            if ~isobject(obj.encapsStack.decoder) % Lazy initialization
                valueFactoryManager = obj.communicator.getValueFactoryManager();
                if obj.encapsStack.encoding_1_0
                    obj.encapsStack.decoder = ...
                        IceInternal.EncapsDecoder10(obj, obj.encapsStack, obj.sliceValues, valueFactoryManager, ...
                                                    obj.communicator.getClassResolver(), obj.classGraphDepthMax);
                else
                    obj.encapsStack.decoder = ...
                        IceInternal.EncapsDecoder11(obj, obj.encapsStack, obj.sliceValues, valueFactoryManager, ...
                                                    obj.communicator.getClassResolver(), obj.classGraphDepthMax);
                end
                obj.encapsStackDecoder = obj.encapsStack.decoder;
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
        encoding_1_0 logical
        encapsStack
        encapsStackDecoder
        encapsCache
        sliceValues logical = true
        buf
        pos int32 = 1
        size int32
        startSeq int32 = -1
        minSeqSize int32 = 0
        classGraphDepthMax = 0
    end
end
