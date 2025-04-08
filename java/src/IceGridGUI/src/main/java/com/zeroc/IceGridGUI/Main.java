// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI;

import com.zeroc.Ice.LocalException;

import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.prefs.Preferences;

import javax.swing.*;

public class Main extends JFrame {
    public static void main(final String[] args) {
        // Initialize L&F in main thread before Swing is used
        try {
            if (System.getProperty("os.name").startsWith("Mac OS")) {
                System.setProperty("apple.laf.useScreenMenuBar", "true");
                System.setProperty(
                    "com.apple.mrj.application.apple.menu.about.name", "IceGrid GUI");

                // To catch Command+Q
                System.setProperty("apple.eawt.quitStrategy", "CLOSE_ALL_WINDOWS");

                UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
            }
        } catch (Exception e) {
            System.err.println(e.toString());
            JOptionPane.showMessageDialog(
                null, e.toString(), "Initialization failed", JOptionPane.ERROR_MESSAGE);
            System.exit(1);
        }

        SwingUtilities.invokeLater(
            () -> {
                try {
                    // Create and set up the window.
                    new Main(args);
                } catch (LocalException e) {
                    JOptionPane.showMessageDialog(
                        null,
                        e.toString(),
                        "Initialization failed",
                        JOptionPane.ERROR_MESSAGE);
                    System.exit(1);
                }
            });
    }

    Main(String[] args) {
        super("IceGrid GUI");
        ImageIcon icon = Utils.getIcon("/icons/16x16/grid.png");
        if (icon != null) {
            setIconImage(icon.getImage());
        }

        setDefaultCloseOperation(WindowConstants.DO_NOTHING_ON_CLOSE);
        addWindowListener(
            new WindowAdapter() {
                @Override
                public void windowClosing(WindowEvent e) {
                    if (_coordinator != null) {
                        if (_coordinator.needsSaving()) {
                            if (JOptionPane.showOptionDialog(
                                Main.this,
                                "The application has unsaved changes, if you exit"
                                    + " all unsaved changes will be lost.\n"
                                    + "Exit and discard changes?",
                                "Save application",
                                JOptionPane.YES_NO_OPTION,
                                JOptionPane.YES_NO_OPTION,
                                null,
                                null,
                                null)
                                == JOptionPane.YES_OPTION) {
                                _coordinator.exit(0);
                            }
                        } else {
                            _coordinator.exit(0);
                        }
                    }
                }
            });

        _coordinator = new Coordinator(this, args, Preferences.userRoot().node("IceGridGUI"));
        _coordinator.tryMigrateDataDirectory();
        _coordinator.showMainFrame();
    }

    private final Coordinator _coordinator;
}
