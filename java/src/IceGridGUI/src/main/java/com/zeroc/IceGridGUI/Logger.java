// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI;

import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.SwingUtilities;

public class Logger extends com.zeroc.Ice.LoggerI
{
    public
    Logger(JFrame mainFrame)
    {
        super("IceGrid GUI", "");

        _mainFrame = mainFrame;
    }

    @Override
    public void
    warning(final String message)
    {
        // Ignore spurious selector wake up warnings in solaris.
        SwingUtilities.invokeLater(() ->
            {
                JOptionPane.showMessageDialog(_mainFrame, message, "Warning - IceGrid GUI Logger",
                                              JOptionPane.WARNING_MESSAGE);
            });
    }

    @Override
    public void
    error(final String message)
    {
        SwingUtilities.invokeLater(() ->
            {
                JOptionPane.showMessageDialog(_mainFrame, message, "Error - IceGrid GUI Logger",
                                              JOptionPane.ERROR_MESSAGE);
            });
    }

    private final JFrame _mainFrame;
}
