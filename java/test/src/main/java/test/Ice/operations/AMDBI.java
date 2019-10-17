//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.operations;

import java.util.*;
import java.util.concurrent.CompletionStage;
import java.util.concurrent.CompletableFuture;

import com.zeroc.Ice.Current;

import test.Ice.operations.AMD.M.*;

public final class AMDBI implements BDisp
{

    @Override
    synchronized public CompletionStage<Void> opIntfAsync(Current current)
    {
        return CompletableFuture.completedFuture((Void)null);
    }

    @Override
    synchronized public CompletionStage<Void> opBAsync(Current current)
    {
        return CompletableFuture.completedFuture((Void)null);
    }
}
