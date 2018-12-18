// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.optional.lambda;
import java.io.PrintWriter;

import test.Ice.optional.Test.*;
import test.Ice.optional.SerializableClass;

public class AllTests
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    private static class CallbackBase
    {
        CallbackBase()
        {
            _called = false;
        }

        public synchronized void check()
        {
            while(!_called)
            {
                try
                {
                    wait();
                }
                catch(InterruptedException ex)
                {
                }
            }

            _called = false;
            test(_value);
            _value = false;
        }

        public synchronized void called(boolean value)
        {
            assert(!_called);
            _value = value;
            _called = true;
            notify();
        }

        private boolean _value;
        private boolean _called;
    }

    public static InitialPrx
    allTests(test.TestHelper helper)
    {
        Ice.Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();

        String ref = "initial:" + helper.getTestEndpoint(0);
        Ice.ObjectPrx base = communicator.stringToProxy(ref);

        InitialPrx initial = InitialPrxHelper.uncheckedCast(base);

        out.print("testing optional parameters with async lambda callbacks... ");
        out.flush();
        final boolean reqParams = initial.supportsRequiredParams();

        final boolean supportsJavaSerializable = initial.supportsJavaSerializable();

        {
            Ice.ByteOptional p1 = new Ice.ByteOptional((byte)56);
            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opByte(p1.get(),
                    (Ice.ByteOptional ret, Ice.ByteOptional p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get() == p2.get() && ret.get() == p1.get()),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }

            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opByte(p1,
                    (Ice.ByteOptional ret, Ice.ByteOptional p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get() == p2.get() && ret.get() == p1.get()),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }
        }

        {
            Ice.BooleanOptional p1 = new Ice.BooleanOptional(true);
            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opBool(p1.get(),
                    (Ice.BooleanOptional ret, Ice.BooleanOptional p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get() == p2.get() && ret.get() == p1.get()),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }

            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opBool(p1,
                    (Ice.BooleanOptional ret, Ice.BooleanOptional p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get() == p2.get() && ret.get() == p1.get()),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }
        }

        {
            Ice.ShortOptional p1 = new Ice.ShortOptional((short)56);
            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opShort(p1.get(),
                    (Ice.ShortOptional ret, Ice.ShortOptional p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get() == p2.get() && ret.get() == p1.get()),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }
            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opShort(p1,
                    (Ice.ShortOptional ret, Ice.ShortOptional p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get() == p2.get() && ret.get() == p1.get()),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }
        }

        {
            Ice.IntOptional p1 = new Ice.IntOptional(56);
            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opInt(p1.get(),
                    (Ice.IntOptional ret, Ice.IntOptional p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get() == p2.get() && ret.get() == p1.get()),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }

            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opInt(p1,
                    (Ice.IntOptional ret, Ice.IntOptional p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get() == p2.get() && ret.get() == p1.get()),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }
        }

        {
            Ice.LongOptional p1 = new Ice.LongOptional(56);
            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opLong(p1.get(),
                    (Ice.LongOptional ret, Ice.LongOptional p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get() == p2.get() && ret.get() == p1.get()),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }

            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opLong(p1,
                    (Ice.LongOptional ret, Ice.LongOptional p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get() == p2.get() && ret.get() == p1.get()),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }
        }

        {
            Ice.FloatOptional p1 = new Ice.FloatOptional(1.0f);
            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opFloat(p1.get(),
                    (Ice.FloatOptional ret, Ice.FloatOptional p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get() == p2.get() && ret.get() == p1.get()),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }

            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opFloat(p1,
                    (Ice.FloatOptional ret, Ice.FloatOptional p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get() == p2.get() && ret.get() == p1.get()),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }
        }

        {
            Ice.DoubleOptional p1 = new Ice.DoubleOptional(1.0);
            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opDouble(p1.get(),
                    (Ice.DoubleOptional ret, Ice.DoubleOptional p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get() == p2.get() && ret.get() == p1.get()),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }

            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opDouble(p1,
                    (Ice.DoubleOptional ret, Ice.DoubleOptional p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get() == p2.get() && ret.get() == p1.get()),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }
        }

        {
            Ice.Optional<String> p1 = new Ice.Optional<String>("test");
            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opString(p1.get(),
                    (Ice.Optional<String> ret, Ice.Optional<String> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get().equals(p2.get()) && ret.get().equals(p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }

            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opString(p1,
                    (Ice.Optional<String> ret, Ice.Optional<String> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get().equals(p2.get()) && ret.get().equals(p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }
        }

        {
            Ice.Optional<MyEnum> p1 = new Ice.Optional<MyEnum>(MyEnum.MyEnumMember);
            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opMyEnum(p1.get(),
                    (Ice.Optional<MyEnum> ret, Ice.Optional<MyEnum> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get() == p2.get() && ret.get() == p1.get()),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }

            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opMyEnum(p1,
                    (Ice.Optional<MyEnum> ret, Ice.Optional<MyEnum> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get() == p2.get() && ret.get() == p1.get()),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }
        }

        {
            Ice.Optional<SmallStruct> p1 = new Ice.Optional<SmallStruct>(new SmallStruct((byte)56));
            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opSmallStruct(p1.get(),
                    (Ice.Optional<SmallStruct> ret, Ice.Optional<SmallStruct> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get().equals(p2.get()) && ret.get().equals(p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }

            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opSmallStruct(p1,
                    (Ice.Optional<SmallStruct> ret, Ice.Optional<SmallStruct> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get().equals(p2.get()) && ret.get().equals(p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }
        }

        {
            Ice.Optional<FixedStruct> p1 = new Ice.Optional<FixedStruct>(new FixedStruct((byte)56));
            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opFixedStruct(p1.get(),
                    (Ice.Optional<FixedStruct> ret, Ice.Optional<FixedStruct> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get().equals(p2.get()) && ret.get().equals(p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }

            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opFixedStruct(p1,
                    (Ice.Optional<FixedStruct> ret, Ice.Optional<FixedStruct> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get().equals(p2.get()) && ret.get().equals(p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }
        }

        {
            Ice.Optional<VarStruct> p1 = new Ice.Optional<VarStruct>(new VarStruct("test"));
            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opVarStruct(p1.get(),
                    (Ice.Optional<VarStruct> ret, Ice.Optional<VarStruct> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get().equals(p2.get()) && ret.get().equals(p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }

            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opVarStruct(p1,
                    (Ice.Optional<VarStruct> ret, Ice.Optional<VarStruct> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get().equals(p2.get()) && ret.get().equals(p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }
        }

        {
            Ice.Optional<OneOptional> p1 = new Ice.Optional<OneOptional>(new OneOptional(58));
            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opOneOptional(p1.get(),
                    (Ice.Optional<OneOptional> ret, Ice.Optional<OneOptional> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get().getA() == p2.get().getA() &&
                                ret.get().getA() == p1.get().getA()),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }

            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opOneOptional(p1,
                    (Ice.Optional<OneOptional> ret, Ice.Optional<OneOptional> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get().getA() == p2.get().getA() &&
                                ret.get().getA() == p1.get().getA()),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }
        }

        {
            Ice.Optional<OneOptionalPrx> p1 = new Ice.Optional<OneOptionalPrx>(
                OneOptionalPrxHelper.uncheckedCast(communicator.stringToProxy("test")));
            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opOneOptionalProxy(p1.get(),
                    (Ice.Optional<OneOptionalPrx> ret, Ice.Optional<OneOptionalPrx> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get().equals(p2.get()) && ret.get().equals(p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }

            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opOneOptionalProxy(p1,
                    (Ice.Optional<OneOptionalPrx> ret, Ice.Optional<OneOptionalPrx> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get().equals(p2.get()) && ret.get().equals(p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }
        }

        {
            Ice.Optional<byte[]> p1 = new Ice.Optional<byte[]>(new byte[100]);
            java.util.Arrays.fill(p1.get(), (byte)56);

            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opByteSeq(p1.get(),
                    (Ice.Optional<byte[]> ret, Ice.Optional<byte[]> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && java.util.Arrays.equals(ret.get(), p2.get()) &&
                                java.util.Arrays.equals(ret.get(), p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }

            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opByteSeq(p1,
                    (Ice.Optional<byte[]> ret, Ice.Optional<byte[]> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && java.util.Arrays.equals(ret.get(), p2.get()) &&
                                java.util.Arrays.equals(ret.get(), p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }
        }

        {
            Ice.Optional<boolean[]> p1 = new Ice.Optional<boolean[]>(new boolean[100]);
            java.util.Arrays.fill(p1.get(), true);
            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opBoolSeq(p1.get(),
                    (Ice.Optional<boolean[]> ret, Ice.Optional<boolean[]> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && java.util.Arrays.equals(ret.get(), p2.get()) &&
                                java.util.Arrays.equals(ret.get(), p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }

            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opBoolSeq(p1,
                    (Ice.Optional<boolean[]> ret, Ice.Optional<boolean[]> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && java.util.Arrays.equals(ret.get(), p2.get()) &&
                                java.util.Arrays.equals(ret.get(), p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }
        }

        {
            Ice.Optional<short[]> p1 = new Ice.Optional<short[]>(new short[100]);
            java.util.Arrays.fill(p1.get(), (short)56);
            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opShortSeq(p1.get(),
                    (Ice.Optional<short[]> ret, Ice.Optional<short[]> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && java.util.Arrays.equals(ret.get(), p2.get()) &&
                                java.util.Arrays.equals(ret.get(), p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }

            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opShortSeq(p1,
                    (Ice.Optional<short[]> ret, Ice.Optional<short[]> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && java.util.Arrays.equals(ret.get(), p2.get()) &&
                                java.util.Arrays.equals(ret.get(), p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }
        }

        {
            Ice.Optional<int[]> p1 = new Ice.Optional<int[]>(new int[100]);
            java.util.Arrays.fill(p1.get(), 56);
            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opIntSeq(p1.get(),
                    (Ice.Optional<int[]> ret, Ice.Optional<int[]> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && java.util.Arrays.equals(ret.get(), p2.get()) &&
                                java.util.Arrays.equals(ret.get(), p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }

            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opIntSeq(p1,
                    (Ice.Optional<int[]> ret, Ice.Optional<int[]> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && java.util.Arrays.equals(ret.get(), p2.get()) &&
                            java.util.Arrays.equals(ret.get(), p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }
        }

        {
            Ice.Optional<long[]> p1 = new Ice.Optional<long[]>(new long[100]);
            java.util.Arrays.fill(p1.get(), 56);
            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opLongSeq(p1.get(),
                    (Ice.Optional<long[]> ret, Ice.Optional<long[]> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && java.util.Arrays.equals(ret.get(), p2.get()) &&
                            java.util.Arrays.equals(ret.get(), p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }

            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opLongSeq(p1,
                    (Ice.Optional<long[]> ret, Ice.Optional<long[]> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && java.util.Arrays.equals(ret.get(), p2.get()) &&
                            java.util.Arrays.equals(ret.get(), p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }
        }

        {
            Ice.Optional<float[]> p1 = new Ice.Optional<float[]>(new float[100]);
            java.util.Arrays.fill(p1.get(), 1.0f);
            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opFloatSeq(p1.get(),
                    (Ice.Optional<float[]> ret, Ice.Optional<float[]> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && java.util.Arrays.equals(ret.get(), p2.get()) &&
                            java.util.Arrays.equals(ret.get(), p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }

            {
                CallbackBase cb = new CallbackBase();
                initial.begin_opFloatSeq(p1,
                    (Ice.Optional<float[]> ret, Ice.Optional<float[]> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && java.util.Arrays.equals(ret.get(), p2.get()) &&
                            java.util.Arrays.equals(ret.get(), p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }
        }

        {
            Ice.Optional<double[]> p1 = new Ice.Optional<double[]>(new double[100]);
            java.util.Arrays.fill(p1.get(), 1.0);
            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opDoubleSeq(p1.get(),
                    (Ice.Optional<double[]> ret, Ice.Optional<double[]> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && java.util.Arrays.equals(ret.get(), p2.get()) &&
                            java.util.Arrays.equals(ret.get(), p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }

            {
                CallbackBase cb = new CallbackBase();
                initial.begin_opDoubleSeq(p1,
                    (Ice.Optional<double[]> ret, Ice.Optional<double[]> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && java.util.Arrays.equals(ret.get(), p2.get()) &&
                            java.util.Arrays.equals(ret.get(), p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }
        }

        {
            Ice.Optional<String[]> p1 = new Ice.Optional<String[]>(new String[10]);
            java.util.Arrays.fill(p1.get(), "test");
            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opStringSeq(p1.get(),
                    (Ice.Optional<String[]> ret, Ice.Optional<String[]> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && java.util.Arrays.equals(ret.get(), p2.get()) &&
                            java.util.Arrays.equals(ret.get(), p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }

            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opStringSeq(p1,
                    (Ice.Optional<String[]> ret, Ice.Optional<String[]> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && java.util.Arrays.equals(ret.get(), p2.get()) &&
                            java.util.Arrays.equals(ret.get(), p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }
        }

        {
            Ice.Optional<SmallStruct[]> p1 = new Ice.Optional<SmallStruct[]>();
            p1.set(new SmallStruct[10]);
            for(int i = 0; i < p1.get().length; ++i)
            {
                p1.get()[i] = new SmallStruct();
            }

            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opSmallStructSeq(p1.get(),
                    (Ice.Optional<SmallStruct[]> ret, Ice.Optional<SmallStruct[]> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && java.util.Arrays.equals(ret.get(), p2.get()) &&
                            java.util.Arrays.equals(ret.get(), p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }

            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opSmallStructSeq(p1,
                    (Ice.Optional<SmallStruct[]> ret, Ice.Optional<SmallStruct[]> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && java.util.Arrays.equals(ret.get(), p2.get()) &&
                            java.util.Arrays.equals(ret.get(), p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }
        }

        {
            Ice.Optional<java.util.List<SmallStruct>> p1 = new Ice.Optional<java.util.List<SmallStruct>>();
            p1.set(new java.util.ArrayList<SmallStruct>());
            for(int i = 0; i < 10; ++i)
            {
                p1.get().add(new SmallStruct());
            }

            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opSmallStructList(p1.get(),
                    (Ice.Optional<java.util.List<SmallStruct>> ret, Ice.Optional<java.util.List<SmallStruct>> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get().equals(p2.get()) && ret.get().equals(p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }

            {
                CallbackBase cb = new CallbackBase();
                initial.begin_opSmallStructList(p1,
                    (Ice.Optional<java.util.List<SmallStruct>> ret, Ice.Optional<java.util.List<SmallStruct>> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get().equals(p2.get()) && ret.get().equals(p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }
        }

        {
            Ice.Optional<FixedStruct[]> p1 = new Ice.Optional<FixedStruct[]>();
            p1.set(new FixedStruct[10]);
            for(int i = 0; i < p1.get().length; ++i)
            {
                p1.get()[i] = new FixedStruct();
            }

            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opFixedStructSeq(p1.get(),
                    (Ice.Optional<FixedStruct[]> ret, Ice.Optional<FixedStruct[]> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && java.util.Arrays.equals(ret.get(), p2.get()) &&
                            java.util.Arrays.equals(ret.get(), p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }

            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opFixedStructSeq(p1,
                    (Ice.Optional<FixedStruct[]> ret, Ice.Optional<FixedStruct[]> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && java.util.Arrays.equals(ret.get(), p2.get()) &&
                            java.util.Arrays.equals(ret.get(), p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }
        }

        {
            Ice.Optional<java.util.List<FixedStruct>> p1 = new Ice.Optional<java.util.List<FixedStruct>>();
            p1.set(new java.util.ArrayList<FixedStruct>());
            for(int i = 0; i < 10; ++i)
            {
                p1.get().add(new FixedStruct());
            }

            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opFixedStructList(p1.get(),
                    (Ice.Optional<java.util.List<FixedStruct>> ret, Ice.Optional<java.util.List<FixedStruct>> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get().equals(p2.get()) && ret.get().equals(p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }

            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opFixedStructList(p1,
                    (Ice.Optional<java.util.List<FixedStruct>> ret, Ice.Optional<java.util.List<FixedStruct>> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get().equals(p2.get()) && ret.get().equals(p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }
        }

        {
            Ice.Optional<VarStruct[]> p1 = new Ice.Optional<VarStruct[]>();
            p1.set(new VarStruct[10]);
            for(int i = 0; i < p1.get().length; ++i)
            {
                p1.get()[i] = new VarStruct("");
            }

            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opVarStructSeq(p1.get(),
                    (Ice.Optional<VarStruct[]> ret, Ice.Optional<VarStruct[]> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && java.util.Arrays.equals(ret.get(), p2.get()) &&
                            java.util.Arrays.equals(ret.get(), p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }

            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opVarStructSeq(p1,
                    (Ice.Optional<VarStruct[]> ret, Ice.Optional<VarStruct[]> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && java.util.Arrays.equals(ret.get(), p2.get()) &&
                            java.util.Arrays.equals(ret.get(), p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }
        }

        if(supportsJavaSerializable)
        {
            Ice.Optional<SerializableClass> p1 = new Ice.Optional<SerializableClass>(new SerializableClass(58));
            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opSerializable(p1.get(),
                    (Ice.Optional<SerializableClass> ret, Ice.Optional<SerializableClass> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get().equals(p2.get()) && ret.get().equals(p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }

            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opSerializable(p1,
                    (Ice.Optional<SerializableClass> ret, Ice.Optional<SerializableClass> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get().equals(p2.get()) && ret.get().equals(p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }
        }

        {
            Ice.Optional<java.util.Map<Integer, Integer>> p1 = new Ice.Optional<java.util.Map<Integer, Integer>>();
            p1.set(new java.util.HashMap<Integer, Integer>());
            p1.get().put(1, 2);
            p1.get().put(2, 3);

            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opIntIntDict(p1.get(),
                    (Ice.Optional<java.util.Map<Integer, Integer>> ret,
                    Ice.Optional<java.util.Map<Integer, Integer>> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get().equals(p2.get()) && ret.get().equals(p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }

            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opIntIntDict(p1,
                    (Ice.Optional<java.util.Map<Integer, Integer>> ret,
                    Ice.Optional<java.util.Map<Integer, Integer>> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get().equals(p2.get()) && ret.get().equals(p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }
        }

        {
            Ice.Optional<java.util.Map<String, Integer>> p1 = new Ice.Optional<java.util.Map<String, Integer>>();
            p1.set(new java.util.HashMap<String, Integer>());
            p1.get().put("1", 1);
            p1.get().put("2", 2);

            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opStringIntDict(p1.get(),
                    (Ice.Optional<java.util.Map<String, Integer>> ret,
                    Ice.Optional<java.util.Map<String, Integer>> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get().equals(p2.get()) && ret.get().equals(p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }

            {
                final CallbackBase cb = new CallbackBase();
                initial.begin_opStringIntDict(p1,
                    (Ice.Optional<java.util.Map<String, Integer>> ret,
                    Ice.Optional<java.util.Map<String, Integer>> p2) ->
                        cb.called(ret.isSet() && p2.isSet() && ret.get().equals(p2.get()) && ret.get().equals(p1.get())),
                    (Ice.Exception ex) -> cb.called(false));
                cb.check();
            }
        }

        out.println("ok");

        return initial;
    }
}
