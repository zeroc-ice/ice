// Copyright (c) ZeroC, Inc.

package test.Ice.exceptions;

import com.zeroc.Ice.Logger;
import com.zeroc.Ice.Util;

public final class DummyLogger implements Logger {
    @Override
    public void print(String message) {
        _logger.print(message);
    }

    @Override
    public void trace(String category, String message) {
        _logger.trace(category, message);
    }

    @Override
    public void warning(String message) {
        if (!message.contains("test.Ice.exceptions.ThrowerI.throwAssertException")
            && !message.contains(
            "test.Ice.exceptions.AMDThrowerI.throwAssertException_async")) {
            _logger.warning(message);
        }
    }

    @Override
    public void error(String message) {
        if (!message.contains("test.Ice.exceptions.ThrowerI.throwAssertException")
            && !message.contains(
            "test.Ice.exceptions.AMDThrowerI.throwAssertException_async")) {
            _logger.error(message);
        }
    }

    @Override
    public String getPrefix() {
        return "";
    }

    @Override
    public Logger cloneWithPrefix(String prefix) {
        return new DummyLogger();
    }

    private final Logger _logger = Util.getProcessLogger();
}
