// Copyright (c) ZeroC, Inc.

package test.Ice.exceptions;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.DispatchException;
import com.zeroc.Ice.TimeoutException;
import com.zeroc.Ice.UnknownUserException;
import com.zeroc.Ice.UserException;

import test.Ice.exceptions.Test.A;
import test.Ice.exceptions.Test.B;
import test.Ice.exceptions.Test.C;
import test.Ice.exceptions.Test.D;
import test.Ice.exceptions.Test.Thrower;

public final class ThrowerI implements Thrower {
    public ThrowerI() {}

    @Override
    public void shutdown(Current current) {
        current.adapter.getCommunicator().shutdown();
    }

    @Override
    public boolean supportsUndeclaredExceptions(Current current) {
        return false;
    }

    @Override
    public boolean supportsAssertException(Current current) {
        return true;
    }

    @Override
    public void throwAasA(int a, Current current) throws A {
        A ex = new A();
        ex.aMem = a;
        throw ex;
    }

    @Override
    public void throwAorDasAorD(int a, Current current) throws A, D {
        if (a > 0) {
            A ex = new A();
            ex.aMem = a;
            throw ex;
        } else {
            D ex = new D();
            ex.dMem = a;
            throw ex;
        }
    }

    @Override
    public void throwBasA(int a, int b, Current current) throws A {
        throwBasB(a, b, current);
    }

    @Override
    public void throwBasB(int a, int b, Current current) throws B {
        B ex = new B();
        ex.aMem = a;
        ex.bMem = b;
        throw ex;
    }

    @Override
    public void throwCasA(int a, int b, int c, Current current) throws A {
        throwCasC(a, b, c, current);
    }

    @Override
    public void throwCasB(int a, int b, int c, Current current) throws B {
        throwCasC(a, b, c, current);
    }

    @Override
    public void throwCasC(int a, int b, int c, Current current) throws C {
        C ex = new C();
        ex.aMem = a;
        ex.bMem = b;
        ex.cMem = c;
        throw ex;
    }

    @Override
    public void throwUndeclaredA(int a, Current current) {
        // Not possible in Java.
        throw new UnknownUserException("dummy");
    }

    @Override
    public void throwUndeclaredB(int a, int b, Current current) {
        // Not possible in Java.
        throw new UnknownUserException("dummy");
    }

    @Override
    public void throwUndeclaredC(int a, int b, int c, Current current)
            throws UserException {
        throw new C(a, b, c);
    }

    @Override
    public void throwLocalException(Current current) {
        throw new TimeoutException();
    }

    @Override
    public void throwLocalExceptionIdempotent(Current current) {
        throw new TimeoutException();
    }

    @Override
    public void throwDispatchException(byte replyStatus, Current current) {
        // We convert the signed byte into a positive int.
        throw new DispatchException(replyStatus & 0xFF);
    }

    @Override
    public void throwNonIceException(Current current) {
        throw new RuntimeException();
    }

    @Override
    public void throwAssertException(Current current) {
        throw new AssertionError();
    }

    @Override
    public byte[] throwMemoryLimitException(byte[] seq, Current current) {
        return new byte[1024 * 20]; // 20KB is over the configured 10KB message size max.
    }

    @Override
    public void throwAfterResponse(Current current) {
        //
        // Only relevant for AMD.
        //
    }

    @Override
    public void throwAfterException(Current current) throws A {
        //
        // Only relevant for AMD.
        //
        throw new A();
    }
}
