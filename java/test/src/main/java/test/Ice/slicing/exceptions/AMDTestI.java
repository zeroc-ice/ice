// Copyright (c) ZeroC, Inc.

package test.Ice.slicing.exceptions;

import com.zeroc.Ice.Current;

import test.Ice.slicing.exceptions.serverAMD.Test.Base;
import test.Ice.slicing.exceptions.serverAMD.Test.KnownDerived;
import test.Ice.slicing.exceptions.serverAMD.Test.KnownIntermediate;
import test.Ice.slicing.exceptions.serverAMD.Test.KnownMostDerived;
import test.Ice.slicing.exceptions.serverAMD.Test.TestIntf;
import test.Ice.slicing.exceptions.serverAMD.Test.UnknownDerived;
import test.Ice.slicing.exceptions.serverAMD.Test.UnknownIntermediate;
import test.Ice.slicing.exceptions.serverAMD.Test.UnknownMostDerived1;
import test.Ice.slicing.exceptions.serverAMD.Test.UnknownMostDerived2;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;

public final class AMDTestI implements TestIntf {
    @Override
    public CompletionStage<Void> shutdownAsync(Current current) {
        current.adapter.getCommunicator().shutdown();
        return CompletableFuture.completedFuture((Void) null);
    }

    @Override
    public CompletionStage<Void> baseAsBaseAsync(Current current) throws Base {
        Base b = new Base();
        b.b = "Base.b";
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(b);
        return f;
    }

    @Override
    public CompletionStage<Void> unknownDerivedAsBaseAsync(Current current)
        throws Base {
        UnknownDerived d = new UnknownDerived();
        d.b = "UnknownDerived.b";
        d.ud = "UnknownDerived.ud";
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(d);
        return f;
    }

    @Override
    public CompletionStage<Void> knownDerivedAsBaseAsync(Current current)
        throws Base {
        KnownDerived d = new KnownDerived();
        d.b = "KnownDerived.b";
        d.kd = "KnownDerived.kd";
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(d);
        return f;
    }

    @Override
    public CompletionStage<Void> knownDerivedAsKnownDerivedAsync(Current current)
        throws KnownDerived {
        KnownDerived d = new KnownDerived();
        d.b = "KnownDerived.b";
        d.kd = "KnownDerived.kd";
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(d);
        return f;
    }

    @Override
    public CompletionStage<Void> unknownIntermediateAsBaseAsync(Current current)
        throws Base {
        UnknownIntermediate ui = new UnknownIntermediate();
        ui.b = "UnknownIntermediate.b";
        ui.ui = "UnknownIntermediate.ui";
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(ui);
        return f;
    }

    @Override
    public CompletionStage<Void> knownIntermediateAsBaseAsync(Current current)
        throws Base {
        KnownIntermediate ki = new KnownIntermediate();
        ki.b = "KnownIntermediate.b";
        ki.ki = "KnownIntermediate.ki";
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(ki);
        return f;
    }

    @Override
    public CompletionStage<Void> knownMostDerivedAsBaseAsync(Current current)
        throws Base {
        KnownMostDerived kmd = new KnownMostDerived();
        kmd.b = "KnownMostDerived.b";
        kmd.ki = "KnownMostDerived.ki";
        kmd.kmd = "KnownMostDerived.kmd";
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(kmd);
        return f;
    }

    @Override
    public CompletionStage<Void> knownIntermediateAsKnownIntermediateAsync(
            Current current) throws KnownIntermediate {
        KnownIntermediate ki = new KnownIntermediate();
        ki.b = "KnownIntermediate.b";
        ki.ki = "KnownIntermediate.ki";
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(ki);
        return f;
    }

    @Override
    public CompletionStage<Void> knownMostDerivedAsKnownIntermediateAsync(
            Current current) throws KnownIntermediate {
        KnownMostDerived kmd = new KnownMostDerived();
        kmd.b = "KnownMostDerived.b";
        kmd.ki = "KnownMostDerived.ki";
        kmd.kmd = "KnownMostDerived.kmd";
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(kmd);
        return f;
    }

    @Override
    public CompletionStage<Void> knownMostDerivedAsKnownMostDerivedAsync(
            Current current) throws KnownMostDerived {
        KnownMostDerived kmd = new KnownMostDerived();
        kmd.b = "KnownMostDerived.b";
        kmd.ki = "KnownMostDerived.ki";
        kmd.kmd = "KnownMostDerived.kmd";
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(kmd);
        return f;
    }

    @Override
    public CompletionStage<Void> unknownMostDerived1AsBaseAsync(Current current)
        throws Base {
        UnknownMostDerived1 umd1 = new UnknownMostDerived1();
        umd1.b = "UnknownMostDerived1.b";
        umd1.ki = "UnknownMostDerived1.ki";
        umd1.umd1 = "UnknownMostDerived1.umd1";
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(umd1);
        return f;
    }

    @Override
    public CompletionStage<Void> unknownMostDerived1AsKnownIntermediateAsync(
            Current current) throws KnownIntermediate {
        UnknownMostDerived1 umd1 = new UnknownMostDerived1();
        umd1.b = "UnknownMostDerived1.b";
        umd1.ki = "UnknownMostDerived1.ki";
        umd1.umd1 = "UnknownMostDerived1.umd1";
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(umd1);
        return f;
    }

    @Override
    public CompletionStage<Void> unknownMostDerived2AsBaseAsync(Current current)
        throws Base {
        UnknownMostDerived2 umd2 = new UnknownMostDerived2();
        umd2.b = "UnknownMostDerived2.b";
        umd2.ui = "UnknownMostDerived2.ui";
        umd2.umd2 = "UnknownMostDerived2.umd2";
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(umd2);
        return f;
    }
}
