%{
**********************************************************************

Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef AllTests
    methods(Static)
        function r = allTests(helper)
            import Test.*;

            communicator = helper.communicator();

            ref = ['test:', helper.getTestEndpoint()];
            obj = communicator.stringToProxy(ref);
            assert(~isempty(obj));
            proxy = TestIntfPrx.checkedCast(obj);
            assert(~isempty(proxy));

            fprintf('testing enum values... ');

            assert(int32(ByteEnum.benum1) == 0);
            assert(int32(ByteEnum.benum2) == 1);
            assert(int32(ByteEnum.benum3) == ByteConst1.value);
            assert(int32(ByteEnum.benum4) == ByteConst1.value + 1);
            assert(int32(ByteEnum.benum5) == ShortConst1.value);
            assert(int32(ByteEnum.benum6) == ShortConst1.value + 1);
            assert(int32(ByteEnum.benum7) == IntConst1.value);
            assert(int32(ByteEnum.benum8) == IntConst1.value + 1);
            assert(int32(ByteEnum.benum9) == LongConst1.value);
            assert(int32(ByteEnum.benum10) == LongConst1.value + 1);
            assert(int32(ByteEnum.benum11) == ByteConst2.value);

            assert(ByteEnum.ice_getValue(0) == ByteEnum.benum1);
            assert(ByteEnum.ice_getValue(1) == ByteEnum.benum2);
            assert(ByteEnum.ice_getValue(ByteConst1.value) == ByteEnum.benum3);
            assert(ByteEnum.ice_getValue(ByteConst1.value + 1) == ByteEnum.benum4);
            assert(ByteEnum.ice_getValue(ShortConst1.value) == ByteEnum.benum5);
            assert(ByteEnum.ice_getValue(ShortConst1.value + 1) == ByteEnum.benum6);
            assert(ByteEnum.ice_getValue(IntConst1.value) == ByteEnum.benum7);
            assert(ByteEnum.ice_getValue(IntConst1.value + 1) == ByteEnum.benum8);
            assert(ByteEnum.ice_getValue(LongConst1.value) == ByteEnum.benum9);
            assert(ByteEnum.ice_getValue(LongConst1.value + 1) == ByteEnum.benum10);
            assert(ByteEnum.ice_getValue(ByteConst2.value) == ByteEnum.benum11);

            assert(int32(ShortEnum.senum1) == 3);
            assert(int32(ShortEnum.senum2) == 4);
            assert(int32(ShortEnum.senum3) == ByteConst1.value);
            assert(int32(ShortEnum.senum4) == ByteConst1.value + 1);
            assert(int32(ShortEnum.senum5) == ShortConst1.value);
            assert(int32(ShortEnum.senum6) == ShortConst1.value + 1);
            assert(int32(ShortEnum.senum7) == IntConst1.value);
            assert(int32(ShortEnum.senum8) == IntConst1.value + 1);
            assert(int32(ShortEnum.senum9) == LongConst1.value);
            assert(int32(ShortEnum.senum10) == LongConst1.value + 1);
            assert(int32(ShortEnum.senum11) == ShortConst2.value);

            assert(ShortEnum.ice_getValue(3) == ShortEnum.senum1);
            assert(ShortEnum.ice_getValue(4) == ShortEnum.senum2);
            assert(ShortEnum.ice_getValue(ByteConst1.value) == ShortEnum.senum3);
            assert(ShortEnum.ice_getValue(ByteConst1.value + 1) == ShortEnum.senum4);
            assert(ShortEnum.ice_getValue(ShortConst1.value) == ShortEnum.senum5);
            assert(ShortEnum.ice_getValue(ShortConst1.value + 1) == ShortEnum.senum6);
            assert(ShortEnum.ice_getValue(IntConst1.value) == ShortEnum.senum7);
            assert(ShortEnum.ice_getValue(IntConst1.value + 1) == ShortEnum.senum8);
            assert(ShortEnum.ice_getValue(LongConst1.value) == ShortEnum.senum9);
            assert(ShortEnum.ice_getValue(LongConst1.value + 1) == ShortEnum.senum10);
            assert(ShortEnum.ice_getValue(ShortConst2.value) == ShortEnum.senum11);

            assert(int32(IntEnum.ienum1) == 0);
            assert(int32(IntEnum.ienum2) == 1);
            assert(int32(IntEnum.ienum3) == ByteConst1.value);
            assert(int32(IntEnum.ienum4) == ByteConst1.value + 1);
            assert(int32(IntEnum.ienum5) == ShortConst1.value);
            assert(int32(IntEnum.ienum6) == ShortConst1.value + 1);
            assert(int32(IntEnum.ienum7) == IntConst1.value);
            assert(int32(IntEnum.ienum8) == IntConst1.value + 1);
            assert(int32(IntEnum.ienum9) == LongConst1.value);
            assert(int32(IntEnum.ienum10) == LongConst1.value + 1);
            assert(int32(IntEnum.ienum11) == IntConst2.value);
            assert(int32(IntEnum.ienum12) == LongConst2.value);

            assert(IntEnum.ice_getValue(0) == IntEnum.ienum1);
            assert(IntEnum.ice_getValue(1) == IntEnum.ienum2);
            assert(IntEnum.ice_getValue(ByteConst1.value) == IntEnum.ienum3);
            assert(IntEnum.ice_getValue(ByteConst1.value + 1) == IntEnum.ienum4);
            assert(IntEnum.ice_getValue(ShortConst1.value) == IntEnum.ienum5);
            assert(IntEnum.ice_getValue(ShortConst1.value + 1) == IntEnum.ienum6);
            assert(IntEnum.ice_getValue(IntConst1.value) == IntEnum.ienum7);
            assert(IntEnum.ice_getValue(IntConst1.value + 1) == IntEnum.ienum8);
            assert(IntEnum.ice_getValue(LongConst1.value) == IntEnum.ienum9);
            assert(IntEnum.ice_getValue(LongConst1.value + 1) == IntEnum.ienum10);
            assert(IntEnum.ice_getValue(IntConst2.value) == IntEnum.ienum11);
            assert(IntEnum.ice_getValue(LongConst2.value) == IntEnum.ienum12);

            assert(int32(SimpleEnum.red) == 0);
            assert(int32(SimpleEnum.green) == 1);
            assert(int32(SimpleEnum.blue) == 2);

            assert(SimpleEnum.ice_getValue(0) == SimpleEnum.red);
            assert(SimpleEnum.ice_getValue(1) == SimpleEnum.green);
            assert(SimpleEnum.ice_getValue(2) == SimpleEnum.blue);

            fprintf('ok\n');

            fprintf('testing enum streaming... ');

            encoding_1_0 = strcmp(communicator.getProperties().getProperty('Ice.Default.EncodingVersion'), '1.0');

            os = communicator.createOutputStream();
            ByteEnum.ice_write(os, ByteEnum.benum11);
            bytes = os.finished();
            assert(length(bytes) == 1); % ByteEnum should require one byte

            os = communicator.createOutputStream();
            ShortEnum.ice_write(os, ShortEnum.senum11);
            bytes = os.finished();
            if encoding_1_0
                assert(length(bytes) == 2);
            else
                assert(length(bytes) == 5);
            end

            os = communicator.createOutputStream();
            IntEnum.ice_write(os, IntEnum.ienum11);
            bytes = os.finished();
            if encoding_1_0
                assert(length(bytes) == 4);
            else
                assert(length(bytes) == 5);
            end

            os = communicator.createOutputStream();
            SimpleEnum.ice_write(os, SimpleEnum.blue);
            bytes = os.finished();
            assert(length(bytes) == 1); % SimpleEnum should require one byte

            fprintf('ok\n');

            fprintf('testing enum operations... ');

            [r, b2] = proxy.opByte(ByteEnum.benum1);
            assert(r == ByteEnum.benum1 && b2 == ByteEnum.benum1);
            [r, b2] = proxy.opByte(ByteEnum.benum11);
            assert(r == ByteEnum.benum11 && b2 == ByteEnum.benum11);

            [r, s2] = proxy.opShort(ShortEnum.senum1);
            assert(r == ShortEnum.senum1 && r == ShortEnum.senum1);
            [r, s2] = proxy.opShort(ShortEnum.senum11);
            assert(r == ShortEnum.senum11 && r == ShortEnum.senum11);

            [r, i2] = proxy.opInt(IntEnum.ienum1);
            assert(r == IntEnum.ienum1 && i2 == IntEnum.ienum1);
            [r, i2] = proxy.opInt(IntEnum.ienum11);
            assert(r == IntEnum.ienum11 && i2 == IntEnum.ienum11);
            [r, i2] = proxy.opInt(IntEnum.ienum12);
            assert(r == IntEnum.ienum12 && i2 == IntEnum.ienum12);

            [r, s2] = proxy.opSimple(SimpleEnum.green);
            assert(r == SimpleEnum.green && s2 == SimpleEnum.green);

            fprintf('ok\n');

            fprintf('testing enum sequences operations... ');

            b1 = [ ByteEnum.benum1, ByteEnum.benum2, ByteEnum.benum3, ByteEnum.benum4, ByteEnum.benum5, ...
                   ByteEnum.benum6, ByteEnum.benum7, ByteEnum.benum8, ByteEnum.benum9, ByteEnum.benum10, ...
                   ByteEnum.benum11 ];

            [r, b2] = proxy.opByteSeq(b1);

            for i = 1:length(b1)
                assert(b1(i) == b2(i));
                assert(b1(i) == r(i));
            end

            s1 = [ ShortEnum.senum1, ShortEnum.senum2, ShortEnum.senum3, ShortEnum.senum4, ShortEnum.senum5, ...
                   ShortEnum.senum6, ShortEnum.senum7, ShortEnum.senum8, ShortEnum.senum9, ShortEnum.senum10, ...
                   ShortEnum.senum11 ];

            [r, s2] = proxy.opShortSeq(s1);

            for i = 1:length(s1)
                assert(s1(i) == s2(i));
                assert(s1(i) == r(i));
            end

            i1 = [ IntEnum.ienum1, IntEnum.ienum2, IntEnum.ienum3, IntEnum.ienum4, IntEnum.ienum5, ...
                   IntEnum.ienum6, IntEnum.ienum7, IntEnum.ienum8, IntEnum.ienum9, IntEnum.ienum10, ...
                   IntEnum.ienum11 ];

            [r, i2] = proxy.opIntSeq(i1);

            for i = 1:length(i1)
                assert(i1(i) == i2(i));
                assert(i1(i) == r(i));
            end

            s1 = [ SimpleEnum.red, SimpleEnum.green, SimpleEnum.blue ];

            [r, s2] = proxy.opSimpleSeq(s1);

            for i = 1:length(s1)
                assert(s1(i) == s2(i));
                assert(s1(i) == r(i));
            end

            fprintf('ok\n');

            fprintf('testing enum exceptions... ');

            try
                os = communicator.createOutputStream();
                os.writeByte(2); % Invalid enumerator
                in = os.createInputStream();
                ByteEnum.ice_read(in);
                assert(false);
            catch ex
                assert(isa(ex, 'Ice.MarshalException'));
            end

            try
                os = communicator.createOutputStream();
                os.writeByte(128); % Invalid enumerator
                in = os.createInputStream();
                ByteEnum.ice_read(in);
                assert(false);
            catch ex
                assert(isa(ex, 'Ice.MarshalException'));
            end

            try
                os = communicator.createOutputStream();
                os.writeShort(-1); % Negative enumerators are not supported
                in = os.createInputStream();
                ShortEnum.ice_read(in);
                assert(false);
            catch ex
                assert(isa(ex, 'Ice.MarshalException'));
            end

            try
                os = communicator.createOutputStream();
                os.writeShort(0); % Invalid enumerator
                in = os.createInputStream();
                ShortEnum.ice_read(in);
                assert(false);
            catch ex
                assert(isa(ex, 'Ice.MarshalException'));
            end

            try
                os = communicator.createOutputStream();
                os.writeShort(32767); % Invalid enumerator
                in = os.createInputStream();
                ShortEnum.ice_read(in);
                assert(false);
            catch ex
                assert(isa(ex, 'Ice.MarshalException'));
            end

            try
                os = communicator.createOutputStream();
                os.writeInt(-1); % Negative enumerators are not supported
                in = os.createInputStream();
                IntEnum.ice_read(in);
                assert(false);
            catch ex
                assert(isa(ex, 'Ice.MarshalException'));
            end

            try
                os = communicator.createOutputStream();
                os.writeInt(2); % Invalid enumerator
                in = os.createInputStream();
                IntEnum.ice_read(in);
                assert(false);
            catch ex
                assert(isa(ex, 'Ice.MarshalException'));
            end

            fprintf('ok\n');

            r = proxy;
        end
    end
end
