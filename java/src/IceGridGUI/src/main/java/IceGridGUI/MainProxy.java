// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI;

import javax.swing.JOptionPane;

public class MainProxy
{
    public static void main(final String[] args)
    {
        //
        // Save main arguments for later usage.
        //
        _args = java.util.Arrays.copyOf(args, args.length);
        
        String version =  System.getProperty("java.version");
        
        Class<?> cls = IceInternal.Util.findClass("com.javafx.main.Main", null);
        if(cls != null && version.indexOf("1.7") == 0)
        {
            try
            {
                java.lang.reflect.Method main = cls.getMethod("main", new Class[]{String[].class});
                main.invoke(null, new Object[]{args});
                return;
            }
            catch(NoSuchMethodException ex)
            {
                ex.printStackTrace();
                JOptionPane.showMessageDialog(null, 
                                            "Unable to find method `main(String[] args)' in class `com.javafx.main.Main'",
                                            "IceGrid Admin Error", 
                                            JOptionPane.ERROR_MESSAGE);
            }
            catch(IllegalAccessException ex)
            {
                ex.printStackTrace();
                JOptionPane.showMessageDialog(null, 
                                            "IllegalAccessException invoking method `main(String[] args)' in class `com.javafx.main.Main'",
                                            "IceGrid Admin Error", 
                                            JOptionPane.ERROR_MESSAGE);
            }
            catch(java.lang.reflect.InvocationTargetException ex)
            {
            }
        }
        cls = IceInternal.Util.findClass("IceGridGUI.Main", null);
        if(cls == null)
        {
            JOptionPane.showMessageDialog(null, 
                                          "Unable to find class `IceGridGUI.Main'",
                                          "IceGrid Admin Error", 
                                          JOptionPane.ERROR_MESSAGE);
            return;
        }
        try
        {
            java.lang.reflect.Method main = cls.getMethod("main", new Class[]{String[].class});
            main.invoke(null, new Object[]{args});
            return;
        }
        catch(NoSuchMethodException ex)
        {
            ex.printStackTrace();
            JOptionPane.showMessageDialog(null, 
                                        "Unable to find method `main(String[] args)' in class `com.javafx.main.Main'",
                                        "IceGrid Admin Error", 
                                        JOptionPane.ERROR_MESSAGE);
        }
        catch(IllegalAccessException ex)
        {
            ex.printStackTrace();
            JOptionPane.showMessageDialog(null, 
                                        "IllegalAccessException invoking method `main(String[] args)' in class `com.javafx.main.Main'",
                                        "IceGrid Admin Error", 
                                        JOptionPane.ERROR_MESSAGE);
        }
        catch(java.lang.reflect.InvocationTargetException ex)
        {
        }
    }
    
    public static String[] args()
    {
        return _args;
    }
    
    private static String[] _args = new String[]{};
}
