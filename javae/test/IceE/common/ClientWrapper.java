// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class ClientWrapper 
    extends TestApplication 
    implements Runnable
{
    protected void
    startApp()
	throws javax.microedition.midlet.MIDletStateChangeException
    {
	super.startApp();
	_screen.addCommand(CMD_STARTCLIENT);
    }

    public void
    commandAction(javax.microedition.lcdui.Command cmd, javax.microedition.lcdui.Displayable source)
    {
	super.commandAction(cmd, source);
	if(source == _screen)
	{
	    if(cmd == CMD_STARTCLIENT)
	    {
		new Thread(this).start();
	    }
	}
    }
    
    public void
    run()
    {
	try
	{
	    if(_communicator != null)
	    {
		Client.run(new String[0], _communicator, _out);
	    }
	}
	catch(Exception ex)
	{
	    _out.println(ex.toString());
	    ex.printStackTrace();
	}
    }

    private javax.microedition.lcdui.Command CMD_STARTCLIENT =
        new javax.microedition.lcdui.Command("Run", javax.microedition.lcdui.Command.ITEM, CMD_PRIORITY);
}
