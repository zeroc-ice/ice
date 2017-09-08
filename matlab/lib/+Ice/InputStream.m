%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef InputStream < IceInternal.WrapperObject
    methods
        function obj = InputStream(impl)
            obj = obj@IceInternal.WrapperObject(impl);
            obj.encoding = Ice.Util.callMethodWithResult(obj, 'getEncoding');
        end
        function r = readBool(obj)
            v = libpointer('uint8Ptr', 0);
            Ice.Util.callMethod(obj, 'readBool', v);
            r = v.Value == 1;
        end
        function r = readBoolSeq(obj)
            r = Ice.Util.callMethodWithResult(obj, 'readBoolSeq');
        end
        function r = readBoolOpt(obj, tag)
            r = Ice.Util.callMethodWithResult(obj, 'readBoolOpt', tag);
        end
        function r = readBoolSeqOpt(obj, tag)
            r = Ice.Util.callMethodWithResult(obj, 'readBoolSeqOpt', tag);
        end
        function r = readByte(obj)
            v = libpointer('uint8Ptr', 0);
            Ice.Util.callMethod(obj, 'readByte', v);
            r = v.Value;
        end
        function r = readByteSeq(obj)
            r = Ice.Util.callMethodWithResult(obj, 'readByteSeq');
        end
        function r = readByteOpt(obj, tag)
            r = Ice.Util.callMethodWithResult(obj, 'readByteOpt', tag);
        end
        function r = readByteSeqOpt(obj, tag)
            r = Ice.Util.callMethodWithResult(obj, 'readByteSeqOpt', tag);
        end
        function r = readShort(obj)
            v = libpointer('int16Ptr', 0);
            Ice.Util.callMethod(obj, 'readShort', v);
            r = v.Value;
        end
        function r = readShortSeq(obj)
            r = Ice.Util.callMethodWithResult(obj, 'readShortSeq');
        end
        function r = readShortOpt(obj, tag)
            r = Ice.Util.callMethodWithResult(obj, 'readShortOpt', tag);
        end
        function r = readShortSeqOpt(obj, tag)
            r = Ice.Util.callMethodWithResult(obj, 'readShortSeqOpt', tag);
        end
        function r = readInt(obj)
            v = libpointer('int32Ptr', 0);
            Ice.Util.callMethod(obj, 'readInt', v);
            r = v.Value;
        end
        function r = readIntSeq(obj)
            r = Ice.Util.callMethodWithResult(obj, 'readIntSeq');
        end
        function r = readIntOpt(obj, tag)
            r = Ice.Util.callMethodWithResult(obj, 'readIntOpt', tag);
        end
        function r = readIntSeqOpt(obj, tag)
            r = Ice.Util.callMethodWithResult(obj, 'readIntSeqOpt', tag);
        end
        function r = readLong(obj)
            v = libpointer('int64Ptr', 0);
            Ice.Util.callMethod(obj, 'readLong', v);
            r = v.Value;
        end
        function r = readLongSeq(obj)
            r = Ice.Util.callMethodWithResult(obj, 'readLongSeq');
        end
        function r = readLongOpt(obj, tag)
            r = Ice.Util.callMethodWithResult(obj, 'readLongOpt', tag);
        end
        function r = readLongSeqOpt(obj, tag)
            r = Ice.Util.callMethodWithResult(obj, 'readLongSeqOpt', tag);
        end
        function r = readFloat(obj)
            v = libpointer('singlePtr', 0);
            Ice.Util.callMethod(obj, 'readFloat', v);
            r = v.Value;
        end
        function r = readFloatSeq(obj)
            r = Ice.Util.callMethodWithResult(obj, 'readFloatSeq');
        end
        function r = readFloatOpt(obj, tag)
            r = Ice.Util.callMethodWithResult(obj, 'readFloatOpt', tag);
        end
        function r = readFloatSeqOpt(obj, tag)
            r = Ice.Util.callMethodWithResult(obj, 'readFloatSeqOpt', tag);
        end
        function r = readDouble(obj)
            v = libpointer('doublePtr', 0);
            Ice.Util.callMethod(obj, 'readDouble', v);
            r = v.Value;
        end
        function r = readDoubleSeq(obj)
            r = Ice.Util.callMethodWithResult(obj, 'readDoubleSeq');
        end
        function r = readDoubleOpt(obj, tag)
            r = Ice.Util.callMethodWithResult(obj, 'readDoubleOpt', tag);
        end
        function r = readDoubleSeqOpt(obj, tag)
            r = Ice.Util.callMethodWithResult(obj, 'readDoubleSeqOpt', tag);
        end
        function r = readString(obj)
            r = Ice.Util.callMethodWithResult(obj, 'readString');
        end
        function r = readStringSeq(obj)
            r = Ice.Util.callMethodWithResult(obj, 'readStringSeq');
        end
        function r = readStringOpt(obj, tag)
            r = Ice.Util.callMethodWithResult(obj, 'readStringOpt', tag);
        end
        function r = readStringSeqOpt(obj, tag)
            r = Ice.Util.callMethodWithResult(obj, 'readStringSeqOpt', tag);
        end
        function skip(obj, n)
            Ice.Util.callMethod(obj, 'skip', n);
        end
        function skipSize(obj)
            Ice.Util.callMethod(obj, 'skipSize');
        end
        function startException(obj)
            assert(~isempty(obj.currentEncaps) && ~isempty(obj.currentEncaps.decoder));
            obj.currentEncaps.decoder.startInstance(IceInternal.SliceType.ExceptionSlice);
        end
        function endException(obj) % TODO: preserve
            assert(~isempty(obj.currentEncaps) && ~isempty(obj.currentEncaps.decoder));
            obj.currentEncaps.decoder.endInstance();
        end
        function startEncapsulation(obj)
            oldEncaps = obj.currentEncaps;
            if isempty(oldEncaps) % First allocated encaps?
                obj.currentEncaps = IceInternal.Encaps();
            else
                obj.currentEncaps = IceInternal.Encaps();
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

            if ~isequal(obj.currentEncaps.encoding, obj.Encoding_1_0)
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

            if isequal(encoding, obj.Encoding_1_0)
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
            Ice.Util.callMethod(obj, 'skipEncapsulation');
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
            Ice.Util.callMethod(obj, 'readSize', v);
            r = v.Value;
        end
        function r = readOptional(obj, tag, fmt)
            assert(~isempty(obj.currentEncaps));
            if ~isempty(obj.currentEncaps.decoder)
                r = obj.currentEncaps.decoder.readOptional(tag, fmt);
            else
                r = obj.readOptImpl(tag, fmt);
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
                if v == obj.OPTIONAL_END_MARKER
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
                case obj.OptionalFormatF1
                    obj.skip(1);
                case OptionalFormatF2
                    obj.skip(2);
                case obj.OptionalFormatF4
                    obj.skip(4);
                case obj.OptionalFormatF8
                    obj.skip(8);
                case obj.OptionalFormatSize
                    obj.skipSize();
                case obj.OptionalFormatVSize
                    obj.skip(obj.readSize());
                case obj.OptionalFormatFSize
                    sz = obj.readInt();
                    if sz < 0
                        throw(Ice.UnmarshalOutOfBoundsException());
                    end
                    obj.skip(sz);
                case obj.OptionalFormatClass
                    obj.readValue(); % TODO
            end
        end
        function r = readProxy(obj)
            v = libpointer('voidPtr');
            Ice.Util.callMethod(obj, 'readProxy', v);
            if ~isNull(v)
                r = Ice.ObjectPrx(v);
            else
                r = [];
            end
        end
        function r = readProxyOpt(obj, tag)
            v = libpointer('voidPtr');
            Ice.Util.callMethod(obj, 'readProxyOpt', tag, v);
            if ~isNull(v)
                r = Ice.ObjectPrx(v);
            else
                r = [];
            end
        end
        function r = readEnum(obj, maxValue)
            v = libpointer('int32Ptr', 0);
            Ice.Util.callMethod(obj, 'readEnum', maxValue, v);
            r = v.Value;
        end
        function readValue(obj, func, formalType)
            fprintf('About to call readValue\n');
            if isempty(obj.valueFunctions)
                obj.valueFunctions = {};
            end
            obj.valueFunctions{end + 1} = func;
            Ice.Util.callMethod(obj, 'readValue', obj, func, formalType);
        end
        function readPendingValues(obj)
            Ice.Util.callMethod(obj, 'readPendingValues');
            obj.valueFunctions = {};
        end
        function startValue(obj)
            Ice.Util.callMethod(obj, 'startValue');
        end
        function endValue(obj, preserve)
            Ice.Util.callMethod(obj, 'endValue');
        end
        function throwException(obj)
            obj.initEncaps();
            obj.currentEncaps.decoder.throwException();
        end
        function r = pos(obj)
            v = libpointer('uint32Ptr', 0);
            Ice.Util.callMethod(obj, 'pos', v);
            r = v.Value;
        end
        function r = size(obj)
            v = libpointer('uint32Ptr', 0);
            Ice.Util.callMethod(obj, 'size', v);
            r = v.Value;
        end
    end
    methods(Access=private)
        function initEncaps(obj)
            if isempty(obj.currentEncaps)
                obj.currentEncaps = IceInternal.Encaps();
                obj.currentEncaps.encoding = obj.encoding;
            end

            if isempty(obj.currentEncaps.decoder)
                if obj.currentEncaps.encoding.major == 1 && obj.currentEncaps.encoding.minor == 0
                    obj.currentEncaps.decoder = IceInternal.EncapsDecoder10(obj, obj.currentEncaps);
                else
                    obj.currentEncaps.decoder = IceInternal.EncapsDecoder11(obj, obj.currentEncaps);
                end
            end
        end
    end
    properties(Access=private)
        valueFunctions
        encoding
        currentEncaps
    end
    properties(Constant)
        Encoding_1_0 = Ice.EncodingVersion(1, 0)
        Encoding_1_1 = Ice.EncodingVersion(1, 1)
        OPTIONAL_END_MARKER = hex2dec('ff')
        OptionalFormatF1 = 0
        OptionalFormatF2 = 1
        OptionalFormatF4 = 2
        OptionalFormatF8 = 3
        OptionalFormatSize = 4
        OptionalFormatVSize = 5
        OptionalFormatFSize = 6
        OptionalFormatClass = 7
    end
end
