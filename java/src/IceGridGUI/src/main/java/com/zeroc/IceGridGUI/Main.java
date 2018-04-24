// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceGridGUI;

import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.SwingUtilities;
import javax.swing.UIManager;
import javax.swing.WindowConstants;

import java.util.prefs.Preferences;

public class Main extends JFrame
{
    public static void main(final String[] args)
    {
        //
        // Initialize L&F in main thread before Swing is used
        //
        try
        {
            if(System.getProperty("os.name").startsWith("Mac OS"))
            {
                System.setProperty("apple.laf.useScreenMenuBar", "true");
                System.setProperty("com.apple.mrj.application.apple.menu.about.name", "IceGrid GUI");

                // To catch Command+Q
                System.setProperty("apple.eawt.quitStrategy", "CLOSE_ALL_WINDOWS");

                UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
            }
            else if(System.getProperty("os.name").startsWith("Windows"))
            {
                UIManager.setLookAndFeel("com.jgoodies.looks.windows.WindowsLookAndFeel");
            }
            //
            // Setting PlasticLookAndFeel trigger a crash when creating a JFileChooser.
            //
            //else  // JGoodies L&F
            //{
            //    UIManager.setLookAndFeel("com.jgoodies.looks.plastic.PlasticLookAndFeel");
            //}
        }
        catch(Exception e)
        {
            System.err.println(e.toString());
            JOptionPane.showMessageDialog(null,
                                          e.toString(),
                                          "Initialization failed",
                                          JOptionPane.ERROR_MESSAGE);
            System.exit(1);
        }

        SwingUtilities.invokeLater(() ->
                                   {
                                       try
                                       {
                                           //
                                           // Create and set up the window.
                                           //
                                           new Main(args);
                                       }
                                       catch(com.zeroc.Ice.LocalException e)
                                       {
                                           JOptionPane.showMessageDialog(null,
                                                                         e.toString(),
                                                                         "Initialization failed",
                                                                         JOptionPane.ERROR_MESSAGE);
                                           System.exit(1);
                                       }
                                   });
    }

    Main(String[] args)
    {
        super("IceGrid GUI");
        javax.swing.ImageIcon icon = Utils.getIcon("/icons/16x16/grid.png");
        if(icon != null)
        {
            setIconImage(icon.getImage());
        }

        setDefaultCloseOperation(WindowConstants.DO_NOTHING_ON_CLOSE);
        addWindowListener(new WindowAdapter()
            {
                @Override
                public void windowClosing(WindowEvent e)
                {
                    if(_coordinator != null)
                    {
                        if(_coordinator.needsSaving())
                        {
                            if(JOptionPane.showOptionDialog(
                                   Main.this,
                                   "The application has unsaved changes, if you exit all unsaved changes " +
                                   "will be lost.\nExit and discard changes?",
                                   "Save application", JOptionPane.YES_NO_OPTION,
                                   JOptionPane.YES_NO_OPTION, null, null, null) == JOptionPane.YES_OPTION)
                            {
                                _coordinator.exit(0);
                            }
                        }
                        else
                        {
                            _coordinator.exit(0);
                        }
                    }
                }
            });

        _coordinator = new Coordinator(this, args, Preferences.userRoot().node("IceGridGUI"));

        _coordinator.showMainFrame();
    }

    private Coordinator _coordinator;
}
