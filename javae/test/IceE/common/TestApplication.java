// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

//
// Wraps a test driver class to run it within a MIDP environment.
//
abstract public class TestApplication
    extends javax.microedition.midlet.MIDlet
{
    protected String
    getHost()
    {
	try
	{
	    _hostnameConnection = 
		(javax.microedition.io.ServerSocketConnection)javax.microedition.io.Connector.open("socket://");
	    return _hostnameConnection.getLocalAddress();
	}
	catch(Exception ex)
	{
            return null;
	}
    }

    class SetupThread implements Runnable
    {
        public SetupThread(TestApplication app)
        {
            _app = app;
        }

        public
        void run()
        {
            _app.setup();
        }

        TestApplication _app;
    }


    //
    // Needs to be overridden by wrapper class.
    //
    abstract public void setup();

    abstract protected ConfigurationForm
    initConfigurationForm(javax.microedition.midlet.MIDlet parent, Ice.Properties properties);

    abstract protected void
    updateProperties(Ice.Properties properties);

    protected void
    startApp()
	throws javax.microedition.midlet.MIDletStateChangeException
    {
	try
        {
            //
            // Read properties from embedded config file. 
            //
            Ice.Properties properties = Ice.Util.createProperties();
            java.io.InputStream is = getClass().getResourceAsStream("config");
            if(is != null)
            {
                properties.load(is);
            }

            if(_display == null)
            {
                _display = javax.microedition.lcdui.Display.getDisplay(this);
                _configForm = initConfigurationForm(this, properties); 
                _display.setCurrent(_configForm);
            }

            //
            // Some initialization has to occur in a worker thread. 
            //
            Thread t = new Thread(new SetupThread(this));
            t.start();
        }
	catch(Exception ex)
	{
	    javax.microedition.lcdui.Alert a = 
		new javax.microedition.lcdui.Alert("startApp alert", ex.getMessage(),
			null, javax.microedition.lcdui.AlertType.ERROR);
	    a.setTimeout(javax.microedition.lcdui.Alert.FOREVER);
	    javax.microedition.lcdui.Display.getDisplay(this).setCurrent(a);
	    throw new javax.microedition.midlet.MIDletStateChangeException(ex.getMessage());
	}
    }

    protected void
    pauseApp()
    {
	//
	// The test wrapper will not pause properly because it does not have direct access to the wrapped test
	// driver's communicators.
	//
    }

    protected void
    destroyApp(boolean f)
    {
        //
        // Nothing to do here really. Communicators, etc. should all be
        // shutdown in the forms that have references to them.
        //
        notifyDestroyed();
    }

    public void 
    shutdown()
    {
        destroyApp(true);
    }

    abstract public void
    runTest(Ice.Communicator communicator, java.io.PrintStream ps);
    
    //
    // While it's not necessary in all cases, we will want to hold onto
    // our connection in case the KVM aggressively cleans things up and
    // we lose our assigned IP address.
    //
    protected javax.microedition.io.ServerSocketConnection _hostnameConnection;  
    
    protected javax.microedition.lcdui.Display _display;
    protected ConfigurationForm _configForm;
}
