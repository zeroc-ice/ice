// Copyright (c) ZeroC, Inc.

package test.Ice.optional;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.Value;

import test.Ice.optional.Test.A;
import test.Ice.optional.Test.DerivedException;
import test.Ice.optional.Test.FixedStruct;
import test.Ice.optional.Test.G;
import test.Ice.optional.Test.Initial;
import test.Ice.optional.Test.Initial.OpMDict1MarshaledResult;
import test.Ice.optional.Test.Initial.OpMDict2MarshaledResult;
import test.Ice.optional.Test.Initial.OpMSeq1MarshaledResult;
import test.Ice.optional.Test.Initial.OpMSeq2MarshaledResult;
import test.Ice.optional.Test.Initial.OpMStruct1MarshaledResult;
import test.Ice.optional.Test.Initial.OpMStruct2MarshaledResult;
import test.Ice.optional.Test.MyEnum;
import test.Ice.optional.Test.MyInterfacePrx;
import test.Ice.optional.Test.OneOptional;
import test.Ice.optional.Test.OptionalException;
import test.Ice.optional.Test.RequiredException;
import test.Ice.optional.Test.SmallStruct;
import test.Ice.optional.Test.VarStruct;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.OptionalDouble;
import java.util.OptionalInt;
import java.util.OptionalLong;

public final class InitialI implements Initial {
    @Override
    public void shutdown(Current current) {
        current.adapter.getCommunicator().shutdown();
    }

    @Override
    public Value pingPong(Value obj, Current current) {
        return obj;
    }

    @Override
    public void opOptionalException(OptionalInt a, Optional<String> b, Current current)
        throws OptionalException {
        OptionalException ex = new OptionalException();
        if (a.isPresent()) {
            ex.setA(a.getAsInt());
        } else {
            ex.clearA(); // The member "a" has a default value.
        }
        if (b.isPresent()) {
            ex.setB(b.get());
        }
        throw ex;
    }

    @Override
    public void opDerivedException(OptionalInt a, Optional<String> b, Current current)
        throws OptionalException {
        DerivedException ex = new DerivedException();
        if (a.isPresent()) {
            ex.setA(a.getAsInt());
        } else {
            ex.clearA(); // The member "a" has a default value.
        }
        if (b.isPresent()) {
            ex.setB(b.get());
            ex.setSs(b.get());
        } else {
            ex.clearSs(); // The member "ss" has a default value.
        }
        ex.d1 = "d1";
        ex.d2 = "d2";
        throw ex;
    }

    @Override
    public void opRequiredException(OptionalInt a, Optional<String> b, Current current)
        throws OptionalException {
        RequiredException ex = new RequiredException();
        if (a.isPresent()) {
            ex.setA(a.getAsInt());
        } else {
            ex.clearA(); // The member "a" has a default value.
        }
        if (b.isPresent()) {
            ex.setB(b.get());
            ex.ss = b.get();
        }
        throw ex;
    }

    @Override
    public Initial.OpByteResult opByte(Optional<Byte> p1, Current current) {
        return new Initial.OpByteResult(p1, p1);
    }

    @Override
    public Initial.OpBoolResult opBool(Optional<Boolean> p1, Current current) {
        return new Initial.OpBoolResult(p1, p1);
    }

    @Override
    public Initial.OpShortResult opShort(Optional<Short> p1, Current current) {
        return new Initial.OpShortResult(p1, p1);
    }

    @Override
    public Initial.OpIntResult opInt(OptionalInt p1, Current current) {
        return new Initial.OpIntResult(p1, p1);
    }

    @Override
    public Initial.OpLongResult opLong(OptionalLong p1, Current current) {
        return new Initial.OpLongResult(p1, p1);
    }

    @Override
    public Initial.OpFloatResult opFloat(Optional<Float> p1, Current current) {
        return new Initial.OpFloatResult(p1, p1);
    }

    @Override
    public Initial.OpDoubleResult opDouble(OptionalDouble p1, Current current) {
        return new Initial.OpDoubleResult(p1, p1);
    }

    @Override
    public Initial.OpStringResult opString(Optional<String> p1, Current current) {
        return new Initial.OpStringResult(p1, p1);
    }

    @Override
    public Initial.OpMyEnumResult opMyEnum(Optional<MyEnum> p1, Current current) {
        return new Initial.OpMyEnumResult(p1, p1);
    }

    @Override
    public Initial.OpSmallStructResult opSmallStruct(Optional<SmallStruct> p1, Current current) {
        return new Initial.OpSmallStructResult(p1, p1);
    }

    @Override
    public Initial.OpFixedStructResult opFixedStruct(Optional<FixedStruct> p1, Current current) {
        return new Initial.OpFixedStructResult(p1, p1);
    }

    @Override
    public Initial.OpVarStructResult opVarStruct(Optional<VarStruct> p1, Current current) {
        return new Initial.OpVarStructResult(p1, p1);
    }

    @Override
    public Initial.OpMyInterfaceProxyResult opMyInterfaceProxy(
            Optional<MyInterfacePrx> p1, Current current) {
        return new Initial.OpMyInterfaceProxyResult(p1, p1);
    }

    @Override
    public Initial.OpOneOptionalResult opOneOptional(OneOptional p1, Current current) {
        return new Initial.OpOneOptionalResult(p1, p1);
    }

    @Override
    public Initial.OpByteSeqResult opByteSeq(Optional<byte[]> p1, Current current) {
        return new Initial.OpByteSeqResult(p1, p1);
    }

    @Override
    public Initial.OpBoolSeqResult opBoolSeq(Optional<boolean[]> p1, Current current) {
        return new Initial.OpBoolSeqResult(p1, p1);
    }

    @Override
    public Initial.OpShortSeqResult opShortSeq(Optional<short[]> p1, Current current) {
        return new Initial.OpShortSeqResult(p1, p1);
    }

    @Override
    public Initial.OpIntSeqResult opIntSeq(Optional<int[]> p1, Current current) {
        return new Initial.OpIntSeqResult(p1, p1);
    }

    @Override
    public Initial.OpLongSeqResult opLongSeq(Optional<long[]> p1, Current current) {
        return new Initial.OpLongSeqResult(p1, p1);
    }

    @Override
    public Initial.OpFloatSeqResult opFloatSeq(Optional<float[]> p1, Current current) {
        return new Initial.OpFloatSeqResult(p1, p1);
    }

    @Override
    public Initial.OpDoubleSeqResult opDoubleSeq(Optional<double[]> p1, Current current) {
        return new Initial.OpDoubleSeqResult(p1, p1);
    }

    @Override
    public Initial.OpStringSeqResult opStringSeq(Optional<String[]> p1, Current current) {
        return new Initial.OpStringSeqResult(p1, p1);
    }

    @Override
    public Initial.OpSmallStructSeqResult opSmallStructSeq(
            Optional<SmallStruct[]> p1, Current current) {
        return new Initial.OpSmallStructSeqResult(p1, p1);
    }

    @Override
    public Initial.OpSmallStructListResult opSmallStructList(
            Optional<List<SmallStruct>> p1, Current current) {
        return new Initial.OpSmallStructListResult(p1, p1);
    }

    @Override
    public Initial.OpFixedStructSeqResult opFixedStructSeq(
            Optional<FixedStruct[]> p1, Current current) {
        return new Initial.OpFixedStructSeqResult(p1, p1);
    }

    @Override
    public Initial.OpFixedStructListResult opFixedStructList(
            Optional<List<FixedStruct>> p1, Current current) {
        return new Initial.OpFixedStructListResult(p1, p1);
    }

    @Override
    public Initial.OpVarStructSeqResult opVarStructSeq(Optional<VarStruct[]> p1, Current current) {
        return new Initial.OpVarStructSeqResult(p1, p1);
    }

    @Override
    public Initial.OpSerializableResult opSerializable(
            Optional<SerializableClass> p1, Current current) {
        return new Initial.OpSerializableResult(p1, p1);
    }

    @Override
    public Initial.OpIntIntDictResult opIntIntDict(
            Optional<Map<Integer, Integer>> p1, Current current) {
        return new Initial.OpIntIntDictResult(p1, p1);
    }

    @Override
    public Initial.OpStringIntDictResult opStringIntDict(
            Optional<Map<String, Integer>> p1, Current current) {
        return new Initial.OpStringIntDictResult(p1, p1);
    }

    @Override
    public void opClassAndUnknownOptional(A p, Current current) {}

    @Override
    public G opG(G g, Current current) {
        return g;
    }

    @Override
    public void opVoid(Current current) {}

    @Override
    public OpMStruct1MarshaledResult opMStruct1(Current current) {
        return new OpMStruct1MarshaledResult(new SmallStruct(), current);
    }

    @Override
    public OpMStruct2MarshaledResult opMStruct2(Optional<SmallStruct> p1, Current current) {
        return new OpMStruct2MarshaledResult(p1, p1, current);
    }

    @Override
    public OpMSeq1MarshaledResult opMSeq1(Current current) {
        return new OpMSeq1MarshaledResult(new String[0], current);
    }

    @Override
    public OpMSeq2MarshaledResult opMSeq2(Optional<String[]> p1, Current current) {
        return new OpMSeq2MarshaledResult(p1, p1, current);
    }

    @Override
    public OpMDict1MarshaledResult opMDict1(Current current) {
        return new OpMDict1MarshaledResult(Optional.of(new HashMap<>()), current);
    }

    @Override
    public OpMDict2MarshaledResult opMDict2(
            Optional<Map<String, Integer>> p1, Current current) {
        return new OpMDict2MarshaledResult(p1, p1, current);
    }

    @Override
    public boolean supportsJavaSerializable(Current current) {
        return true;
    }
}
