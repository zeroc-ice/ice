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
	    final String[] args = { "--Ice.Trace.Network=3", "--Ice.Trace.Protocol" };
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
