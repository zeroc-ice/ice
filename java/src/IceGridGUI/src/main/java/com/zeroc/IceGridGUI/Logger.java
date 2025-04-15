// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI;

import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.SwingUtilities;

class Logger implements com.zeroc.Ice.Logger {
    private final com.zeroc.Ice.Logger _defaultLogger;
    private final JFrame _mainFrame;

    Logger(JFrame mainFrame, com.zeroc.Ice.Logger defaultLogger) {
        _mainFrame = mainFrame;
        _defaultLogger = defaultLogger;
    }

    @Override
    public void print(String message) {
        _defaultLogger.print(message);
    }

    @Override
    public void trace(String category, String message) {
        _defaultLogger.trace(category, message);
    }

    @Override
    public void warning(final String message) {
        // Ignore spurious selector wake up warnings in solaris.
        SwingUtilities.invokeLater(
            () -> {
                JOptionPane.showMessageDialog(
                    _mainFrame,
                    message,
                    "Warning - IceGrid GUI Logger",
                    JOptionPane.WARNING_MESSAGE);
            });
    }

    @Override
    public void error(final String message) {
        SwingUtilities.invokeLater(
            () -> {
                JOptionPane.showMessageDialog(
                    _mainFrame,
                    message,
                    "Error - IceGrid GUI Logger",
                    JOptionPane.ERROR_MESSAGE);
            });
    }

    @Override
    public String getPrefix() {
        // Not used for anything.
        return "IceGrid GUI";
    }

    @Override
    public Logger cloneWithPrefix(String prefix) {
        throw new UnsupportedOperationException(
            "cloneWithPrefix is not supported on IceGridGUI.Logger");
    }
}
