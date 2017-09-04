%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef InputStream < Ice.WrapperObject
    methods
        function self = InputStream(impl)
            self = self@Ice.WrapperObject(impl);
        end
        function r = readBool(self)
            v = libpointer('uint8Ptr', 0);
            Ice.Util.callMethod(self, 'readBool', v);
            r = v.Value == 1;
        end
        function r = readBoolSeq(self)
            r = Ice.Util.callMethodWithResult(self, 'readBoolSeq');
        end
        function r = readBoolOpt(self, tag)
            r = Ice.Util.callMethodWithResult(self, 'readBoolOpt', tag);
        end
        function r = readBoolSeqOpt(self, tag)
            r = Ice.Util.callMethodWithResult(self, 'readBoolSeqOpt', tag);
        end
        function r = readByte(self)
            v = libpointer('uint8Ptr', 0);
            Ice.Util.callMethod(self, 'readByte', v);
            r = v.Value;
        end
        function r = readByteSeq(self)
            r = Ice.Util.callMethodWithResult(self, 'readByteSeq');
        end
        function r = readByteOpt(self, tag)
            r = Ice.Util.callMethodWithResult(self, 'readByteOpt', tag);
        end
        function r = readByteSeqOpt(self, tag)
            r = Ice.Util.callMethodWithResult(self, 'readByteSeqOpt', tag);
        end
        function r = readShort(self)
            v = libpointer('int16Ptr', 0);
            Ice.Util.callMethod(self, 'readShort', v);
            r = v.Value;
        end
        function r = readShortSeq(self)
            r = Ice.Util.callMethodWithResult(self, 'readShortSeq');
        end
        function r = readShortOpt(self, tag)
            r = Ice.Util.callMethodWithResult(self, 'readShortOpt', tag);
        end
        function r = readShortSeqOpt(self, tag)
            r = Ice.Util.callMethodWithResult(self, 'readShortSeqOpt', tag);
        end
        function r = readInt(self)
            v = libpointer('int32Ptr', 0);
            Ice.Util.callMethod(self, 'readInt', v);
            r = v.Value;
        end
        function r = readIntSeq(self)
            r = Ice.Util.callMethodWithResult(self, 'readIntSeq');
        end
        function r = readIntOpt(self, tag)
            r = Ice.Util.callMethodWithResult(self, 'readIntOpt', tag);
        end
        function r = readIntSeqOpt(self, tag)
            r = Ice.Util.callMethodWithResult(self, 'readIntSeqOpt', tag);
        end
        function r = readLong(self)
            v = libpointer('int64Ptr', 0);
            Ice.Util.callMethod(self, 'readLong', v);
            r = v.Value;
        end
        function r = readLongSeq(self)
            r = Ice.Util.callMethodWithResult(self, 'readLongSeq');
        end
        function r = readLongOpt(self, tag)
            r = Ice.Util.callMethodWithResult(self, 'readLongOpt', tag);
        end
        function r = readLongSeqOpt(self, tag)
            r = Ice.Util.callMethodWithResult(self, 'readLongSeqOpt', tag);
        end
        function r = readFloat(self)
            v = libpointer('singlePtr', 0);
            Ice.Util.callMethod(self, 'readFloat', v);
            r = v.Value;
        end
        function r = readFloatSeq(self)
            r = Ice.Util.callMethodWithResult(self, 'readFloatSeq');
        end
        function r = readFloatOpt(self, tag)
            r = Ice.Util.callMethodWithResult(self, 'readFloatOpt', tag);
        end
        function r = readFloatSeqOpt(self, tag)
            r = Ice.Util.callMethodWithResult(self, 'readFloatSeqOpt', tag);
        end
        function r = readDouble(self)
            v = libpointer('doublePtr', 0);
            Ice.Util.callMethod(self, 'readDouble', v);
            r = v.Value;
        end
        function r = readDoubleSeq(self)
            r = Ice.Util.callMethodWithResult(self, 'readDoubleSeq');
        end
        function r = readDoubleOpt(self, tag)
            r = Ice.Util.callMethodWithResult(self, 'readDoubleOpt', tag);
        end
        function r = readDoubleSeqOpt(self, tag)
            r = Ice.Util.callMethodWithResult(self, 'readDoubleSeqOpt', tag);
        end
        function r = readString(self)
            r = Ice.Util.callMethodWithResult(self, 'readString');
        end
        function r = readStringSeq(self)
            r = Ice.Util.callMethodWithResult(self, 'readStringSeq');
        end
        function r = readStringOpt(self, tag)
            r = Ice.Util.callMethodWithResult(self, 'readStringOpt', tag);
        end
        function r = readStringSeqOpt(self, tag)
            r = Ice.Util.callMethodWithResult(self, 'readStringSeqOpt', tag);
        end
        function skip(self, n)
            Ice.Util.callMethod(self, 'skip', n);
        end
        function startException(self)
            Ice.Util.callMethod(self, 'startException');
        end
        function endException(self)
            Ice.Util.callMethod(self, 'endException');
        end
        function startEncapsulation(self)
            Ice.Util.callMethod(self, 'startEncapsulation');
        end
        function endEncapsulation(self)
            Ice.Util.callMethod(self, 'endEncapsulation');
        end
        function skipEmptyEncapsulation(self)
            Ice.Util.callMethod(self, 'skipEmptyEncapsulation');
        end
        function skipEncapsulation(self)
            Ice.Util.callMethod(self, 'skipEncapsulation');
        end
        function r = getEncoding(self)
            r = Ice.Util.callMethodWithResult(self, 'getEncoding');
        end
        function startSlice(self)
            Ice.Util.callMethod(self, 'startSlice');
        end
        function endSlice(self)
            Ice.Util.callMethod(self, 'endSlice');
        end
        function skipSlice(self)
            Ice.Util.callMethod(self, 'skipSlice');
        end
        function r = readSize(self)
            v = libpointer('int32Ptr', 0);
            Ice.Util.callMethod(self, 'readSize', v);
            r = v.Value;
        end
        function r = readOptional(self, tag, fmt)
            v = libpointer('uint8Ptr', 0);
            Ice.Util.callMethod(self, 'readOptional', tag, fmt);
            r = v.Value == 1;
        end
        function r = readProxy(self)
            v = libpointer('voidPtr');
            Ice.Util.callMethod(self, 'readProxy', v);
            if ~isNull(v)
                r = Ice.ObjectPrx(v);
            else
                r = [];
            end
        end
        function r = readProxyOpt(self, tag)
            v = libpointer('voidPtr');
            Ice.Util.callMethod(self, 'readProxyOpt', tag, v);
            if ~isNull(v)
                r = Ice.ObjectPrx(v);
            else
                r = [];
            end
        end
        function r = readEnum(self, maxValue)
            v = libpointer('int32Ptr', 0);
            Ice.Util.callMethod(self, 'readEnum', maxValue, v);
            r = v.Value;
        end
        function throwException(self)
            Ice.Util.callMethod(self, 'throwException', self);
        end
    end
end
