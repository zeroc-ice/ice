// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI;

import javax.swing.JOptionPane;

public class Fallback extends javax.swing.JApplet
{
    public void start()
    {
        try
        {
            java.net.URL jar  = Fallback.class.getProtectionDomain().getCodeSource().getLocation();
            
            java.util.List<String> command = new java.util.ArrayList<String>();
            command.add("java");
            command.add("IceGridGUI.Main");
            String[] args = MainProxy.args();
            for(String arg : args)
            {
                command.add(arg);
            }
            ProcessBuilder pb = new ProcessBuilder(command);
            
            java.util.Map<String, String> env = pb.environment();
            
            String classPath = env.get("CLASSPATH");
            classPath = classPath == null ? "" : classPath;
            
            if(classPath.indexOf("IceGridGUI.jar") == -1)
            {
                if(classPath == "")
                {
                    classPath = jar.getPath();
                }
                else
                {
                    classPath = System.getProperty("path.separator") + jar.getPath();
                }
                env.put("CLASSPATH", classPath);
            }
            
            final Process p = pb.start();
            if(p != null)
            {
                Runtime.getRuntime().addShutdownHook(new Thread()
                {
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
        catch(java.io.IOException ex)
        {
            ex.printStackTrace();
            JOptionPane.showMessageDialog(null, 
                                          "IOException trying to start IceGrid Admin from Fallback class",
                                          "IceGrid Admin Error", 
                                          JOptionPane.ERROR_MESSAGE);
            //
            // Exit from the JApplet after we have lauch IceGridGUI
            //
            System.exit(1);
        }
    }
}