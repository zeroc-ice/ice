// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

import javax.microedition.midlet.*;
import javax.microedition.lcdui.*;

public class ChatMIDlet extends MIDlet
{
    public
    ChatMIDlet()
    {
    }

    public void
    destroy()
    {
	if(_communicator != null)
	{
	    try
	    {
		_communicator.destroy();
	    }
	    catch(Exception ex)
	    {
		// Ignore.
	    }
	}
	notifyDestroyed();
    }

    protected void
    destroyApp(boolean unconditional)
    {
	destroy();
    }

    protected void
    pauseApp()
    {
    }

    protected void
    startApp()
    {
	try
	{
	    final String[] args = new String[0];
	    _communicator = Ice.Util.initialize(args);
	}
	catch(Exception ex)
	{
	    Alert alert = new Alert("Ice Error", ex.getMessage(), null, AlertType.ERROR);
	    alert.setTimeout(Alert.FOREVER);
	    Display.getDisplay(this).setCurrent(alert);
	    notifyDestroyed();
	    return;
	}

	_loginForm = new LoginForm(this, _communicator);
	Display.getDisplay(this).setCurrent(_loginForm);
    }

    private Form _loginForm;
    private Ice.Communicator _communicator;
}
