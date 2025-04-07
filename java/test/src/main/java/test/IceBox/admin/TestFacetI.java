// Copyright (c) ZeroC, Inc.

package test.IceBox.admin;

import com.zeroc.Ice.Current;

import test.IceBox.admin.Test.*;

import java.util.Map;
import java.util.function.Consumer;

public class TestFacetI
        implements TestFacet, Consumer<Map<String, String>> {
    public TestFacetI() {}

    @Override
    public synchronized Map<String, String> getChanges(Current current) {
        return _changes;
    }

    @Override
    public synchronized void accept(Map<String, String> changes) {
        _changes = changes;
    }

    private Map<String, String> _changes;
}
