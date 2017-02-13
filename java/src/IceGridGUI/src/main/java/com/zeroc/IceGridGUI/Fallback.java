// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceGridGUI;

import javax.swing.JOptionPane;

public class Fallback extends javax.swing.JApplet
{
    @Override
    public void start()
    {
        try
        {
            java.net.URL jar  = Fallback.class.getProtectionDomain().getCodeSource().getLocation();

            java.util.List<String> command = new java.util.ArrayList<>();
            command.add("java");
            command.add("-cp");
            command.add(jar.toURI().getPath());
            command.add("com.zeroc.IceGridGUI.Main");

            String[] args = MainProxy.args();
            for(String arg : args)
            {
                command.add(arg);
            }

            ProcessBuilder pb = new ProcessBuilder(command);

            final Process p = pb.start();
            if(p != null)
            {
                Runtime.getRuntime().addShutdownHook(new Thread()
                {
                    @Override
                    public void run()
                    {
                        while(true)
                        {
                            try
                            {
                                p.waitFor();
                                break;
                            }
                            catch(InterruptedException ex)
                            {
                            }
                        }
                    }
                });
            }

            //
            // Exit from the JApplet after we have lauch IceGridGUI
            //
            System.exit(0);
        }
        catch(Exception ex)
        {
            ex.printStackTrace();
            JOptionPane.showMessageDialog(null,
                                          "Exception trying to start IceGrid GUI from Fallback class",
                                          "IceGrid GUI Error",
                                          JOptionPane.ERROR_MESSAGE);
            //
            // Exit from the JApplet after we have lauch IceGridGUI
            //
            System.exit(1);
        }
    }
}
