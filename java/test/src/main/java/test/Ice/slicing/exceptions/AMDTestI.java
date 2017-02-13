// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.slicing.exceptions;

import java.util.concurrent.CompletionStage;
import java.util.concurrent.CompletableFuture;

import test.Ice.slicing.exceptions.serverAMD.Test.*;

public final class AMDTestI implements TestIntf
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    @Override
    public CompletionStage<Void> shutdownAsync(com.zeroc.Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
        return CompletableFuture.completedFuture((Void)null);
    }

    @Override
    public CompletionStage<Void> baseAsBaseAsync(com.zeroc.Ice.Current current)
        throws Base
    {
        Base b = new Base();
        b.b = "Base.b";
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(b);
        return f;
    }

    @Override
    public CompletionStage<Void> unknownDerivedAsBaseAsync(com.zeroc.Ice.Current current)
        throws Base
    {
        UnknownDerived d = new UnknownDerived();
        d.b = "UnknownDerived.b";
        d.ud = "UnknownDerived.ud";
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(d);
        return f;
    }

    @Override
    public CompletionStage<Void> knownDerivedAsBaseAsync(com.zeroc.Ice.Current current)
        throws Base
    {
        KnownDerived d = new KnownDerived();
        d.b = "KnownDerived.b";
        d.kd = "KnownDerived.kd";
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(d);
        return f;
    }

    @Override
    public CompletionStage<Void> knownDerivedAsKnownDerivedAsync(com.zeroc.Ice.Current current)
        throws KnownDerived
    {
        KnownDerived d = new KnownDerived();
        d.b = "KnownDerived.b";
        d.kd = "KnownDerived.kd";
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(d);
        return f;
    }

    @Override
    public CompletionStage<Void> unknownIntermediateAsBaseAsync(com.zeroc.Ice.Current current)
        throws Base
    {
        UnknownIntermediate ui = new UnknownIntermediate();
        ui.b = "UnknownIntermediate.b";
        ui.ui = "UnknownIntermediate.ui";
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(ui);
        return f;
    }

    @Override
    public CompletionStage<Void> knownIntermediateAsBaseAsync(com.zeroc.Ice.Current current)
        throws Base
    {
        KnownIntermediate ki = new KnownIntermediate();
        ki.b = "KnownIntermediate.b";
        ki.ki = "KnownIntermediate.ki";
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(ki);
        return f;
    }

    @Override
    public CompletionStage<Void> knownMostDerivedAsBaseAsync(com.zeroc.Ice.Current current)
        throws Base
    {
        KnownMostDerived kmd = new KnownMostDerived();
        kmd.b = "KnownMostDerived.b";
        kmd.ki = "KnownMostDerived.ki";
        kmd.kmd = "KnownMostDerived.kmd";
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(kmd);
        return f;
    }

    @Override
    public CompletionStage<Void> knownIntermediateAsKnownIntermediateAsync(com.zeroc.Ice.Current current)
        throws KnownIntermediate
    {
        KnownIntermediate ki = new KnownIntermediate();
        ki.b = "KnownIntermediate.b";
        ki.ki = "KnownIntermediate.ki";
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(ki);
        return f;
    }

    @Override
    public CompletionStage<Void> knownMostDerivedAsKnownIntermediateAsync(com.zeroc.Ice.Current current)
        throws KnownIntermediate
    {
        KnownMostDerived kmd = new KnownMostDerived();
        kmd.b = "KnownMostDerived.b";
        kmd.ki = "KnownMostDerived.ki";
        kmd.kmd = "KnownMostDerived.kmd";
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(kmd);
        return f;
    }

    @Override
    public CompletionStage<Void>
    knownMostDerivedAsKnownMostDerivedAsync(com.zeroc.Ice.Current current)
        throws KnownMostDerived
    {
        KnownMostDerived kmd = new KnownMostDerived();
        kmd.b = "KnownMostDerived.b";
        kmd.ki = "KnownMostDerived.ki";
        kmd.kmd = "KnownMostDerived.kmd";
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(kmd);
        return f;
    }

    @Override
    public CompletionStage<Void> unknownMostDerived1AsBaseAsync(com.zeroc.Ice.Current current)
        throws Base
    {
        UnknownMostDerived1 umd1 = new UnknownMostDerived1();
        umd1.b = "UnknownMostDerived1.b";
        umd1.ki = "UnknownMostDerived1.ki";
        umd1.umd1 = "UnknownMostDerived1.umd1";
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(umd1);
        return f;
    }

    @Override
    public CompletionStage<Void> unknownMostDerived1AsKnownIntermediateAsync(com.zeroc.Ice.Current current)
        throws KnownIntermediate
    {
        UnknownMostDerived1 umd1 = new UnknownMostDerived1();
        umd1.b = "UnknownMostDerived1.b";
        umd1.ki = "UnknownMostDerived1.ki";
        umd1.umd1 = "UnknownMostDerived1.umd1";
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(umd1);
        return f;
    }

    @Override
    public CompletionStage<Void> unknownMostDerived2AsBaseAsync(com.zeroc.Ice.Current current)
        throws Base
    {
        UnknownMostDerived2 umd2 = new UnknownMostDerived2();
        umd2.b = "UnknownMostDerived2.b";
        umd2.ui = "UnknownMostDerived2.ui";
        umd2.umd2 = "UnknownMostDerived2.umd2";
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(umd2);
        return f;
    }

    @Override
    public CompletionStage<Void> unknownMostDerived2AsBaseCompactAsync(com.zeroc.Ice.Current current)
    {
        UnknownMostDerived2 umd2 = new UnknownMostDerived2();
        umd2.b = "UnknownMostDerived2.b";
        umd2.ui = "UnknownMostDerived2.ui";
        umd2.umd2 = "UnknownMostDerived2.umd2";
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(umd2);
        return f;
    }

    @Override
    public CompletionStage<Void> knownPreservedAsBaseAsync(com.zeroc.Ice.Current current)
    {
        KnownPreservedDerived ex = new KnownPreservedDerived();
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(ex);
        return f;
    }

    @Override
    public CompletionStage<Void> knownPreservedAsKnownPreservedAsync(com.zeroc.Ice.Current current)
    {
        KnownPreservedDerived ex = new KnownPreservedDerived();
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(ex);
        return f;
    }

    @Override
    public CompletionStage<Void> relayKnownPreservedAsBaseAsync(RelayPrx r, com.zeroc.Ice.Current current)
    {
        try
        {
            r.knownPreservedAsBase();
            test(false);
            return null;
        }
        catch(com.zeroc.Ice.UserException ex)
        {
            CompletableFuture<Void> f = new CompletableFuture<>();
            f.completeExceptionally(ex);
            return f;
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            CompletableFuture<Void> f = new CompletableFuture<>();
            f.completeExceptionally(ex);
            return f;
        }
    }

    @Override
    public CompletionStage<Void> relayKnownPreservedAsKnownPreservedAsync(RelayPrx r, com.zeroc.Ice.Current current)
    {
        try
        {
            r.knownPreservedAsKnownPreserved();
            test(false);
            return null;
        }
        catch(com.zeroc.Ice.UserException ex)
        {
            CompletableFuture<Void> f = new CompletableFuture<>();
            f.completeExceptionally(ex);
            return f;
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            CompletableFuture<Void> f = new CompletableFuture<>();
            f.completeExceptionally(ex);
            return f;
        }
    }

    @Override
    public CompletionStage<Void> unknownPreservedAsBaseAsync(com.zeroc.Ice.Current current)
    {
        SPreserved2 ex = new SPreserved2();
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        ex.p1 = new SPreservedClass("bc", "spc");
        ex.p2 = ex.p1;
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(ex);
        return f;
    }

    @Override
    public CompletionStage<Void> unknownPreservedAsKnownPreservedAsync(com.zeroc.Ice.Current current)
    {
        SPreserved2 ex = new SPreserved2();
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        ex.p1 = new SPreservedClass("bc", "spc");
        ex.p2 = ex.p1;
        CompletableFuture<Void> f = new CompletableFuture<>();
        f.completeExceptionally(ex);
        return f;
    }

    @Override
    public CompletionStage<Void> relayUnknownPreservedAsBaseAsync(RelayPrx r, com.zeroc.Ice.Current current)
    {
        try
        {
            r.unknownPreservedAsBase();
            test(false);
            return null;
        }
        catch(com.zeroc.Ice.UserException ex)
        {
            CompletableFuture<Void> f = new CompletableFuture<>();
            f.completeExceptionally(ex);
            return f;
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            CompletableFuture<Void> f = new CompletableFuture<>();
            f.completeExceptionally(ex);
            return f;
        }
    }

    @Override
    public CompletionStage<Void> relayUnknownPreservedAsKnownPreservedAsync(RelayPrx r, com.zeroc.Ice.Current current)
    {
        try
        {
            r.unknownPreservedAsKnownPreserved();
            test(false);
            return null;
        }
        catch(com.zeroc.Ice.UserException ex)
        {
            CompletableFuture<Void> f = new CompletableFuture<>();
            f.completeExceptionally(ex);
            return f;
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            CompletableFuture<Void> f = new CompletableFuture<>();
            f.completeExceptionally(ex);
            return f;
        }
    }
}
