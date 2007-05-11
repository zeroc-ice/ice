// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

import javax.microedition.lcdui.*;

class ConfigurationForm extends Form implements CommandListener, Runnable
{
    public class Shutdown implements Runnable
    {
        Shutdown(TestApplication parent, Ice.Communicator communicator)
        {
            _parent = parent;
            _communicator = communicator;
        }

        public void
        run()
        {
            try
            {
                if(_communicator != null)
                {
                    _communicator.destroy();
                }
                _parent.shutdown();
            }
            catch(Exception ex)
            {
            }
        }

        Ice.Communicator _communicator;
        TestApplication _parent;
    }

    public
    ConfigurationForm(javax.microedition.midlet.MIDlet parent, Ice.Properties properties)
    {
        super("Configure");
        _parent = parent;
        _properties = properties;
        _okCommand = new Command("OK", Command.OK, 0);
        _exitCommand = new Command("Exit", Command.EXIT, 1);

        addCommand(_exitCommand);
        setCommandListener(this);
    }

    public void
    commandAction(Command c, Displayable s)
    {
        if(c.getCommandType() == Command.EXIT)
        {
            Thread t = new Thread(new Shutdown((TestApplication)_parent, _communicator));
            t.start();
        }
        else
        {
            ((TestApplication)_parent).updateProperties(_properties);
            //
            // Once we start the test, we don't need the Ok command
            // anymore.
            //
            Thread t = new Thread(this);
            t.start();
        }
    }

    public void
    setHost(String host)
    {
        _properties.setProperty("Ice.Default.Host", host);
    }

    public void
    enableOk()
    {
        addCommand(_okCommand);
    }

    public void 
    run()
    {
        Screen screen = new List("Standard Output", javax.microedition.lcdui.Choice.EXCLUSIVE);
        _out = new java.io.PrintStream(new Test.StreamListAdapter((javax.microedition.lcdui.List)screen));
        Display.getDisplay(_parent).setCurrent(screen);

        screen.addCommand(_exitCommand);
        screen.setCommandListener(this);

        // 
        // We remain the command listener for the time being. In the
        // future, it might be nice to have separate command listener
        // for when the test is running.
        //

        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = _properties;
        _communicator = Ice.Util.initialize(new String[0], initData);
        ((TestApplication)_parent).runTest(_communicator, _out);
    }

    private javax.microedition.midlet.MIDlet _parent;
    private Ice.Communicator _communicator;
    private Command _okCommand;
    private Command _exitCommand;
    private java.io.PrintStream _out;
    private Ice.Properties _properties;
}
