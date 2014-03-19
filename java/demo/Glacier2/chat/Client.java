// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.Component;
import java.awt.event.ActionEvent;
import java.awt.event.ComponentEvent;
import java.awt.event.ComponentListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

import javax.swing.BoxLayout;
import javax.swing.JDialog;
import javax.swing.AbstractAction;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPasswordField;
import javax.swing.JPopupMenu;
import javax.swing.JScrollBar;
import javax.swing.JScrollPane;
import javax.swing.JSeparator;
import javax.swing.JSplitPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.SwingUtilities;
import javax.swing.text.AttributeSet;
import javax.swing.text.BadLocationException;
import javax.swing.text.DefaultEditorKit;
import javax.swing.text.Document;
import javax.swing.text.Element;

import Glacier2.SessionFactoryHelper;
import Glacier2.SessionHelper;
import Glacier2.SessionCallback;
import Glacier2.SessionNotExistException;
import Ice.Current;
import Ice.LocalException;
import Ice.StringSeqHolder;
import Ice.Util;

@SuppressWarnings("serial")
public class Client extends JFrame
{
    public static void
    main(final String[] args)
    {
        SwingUtilities.invokeLater(new Runnable()
        {
            public void
            run()
            {
                try
                {
                    //
                    // Create and set up the window.
                    //
                    new Client(args);
                }
                catch(Ice.LocalException e)
                {
                    JOptionPane.showMessageDialog(null,
                            e.toString(),
                            "Initialization failed",
                            JOptionPane.ERROR_MESSAGE);
                }
            }
        });
    }

    Client(String[] args)
    {
        // Build the JTextArea that shows the chat conversation.
        _output = new JTextArea("");
        _output.setLineWrap(true);
        _output.setEditable(false);

        final JPopupMenu textMenu = new JPopupMenu();
        textMenu.add(new DefaultEditorKit.CopyAction());
        textMenu.pack();

        _output.addMouseListener(new MouseAdapter()
        {
            public void
            mousePressed(MouseEvent e)
            {
                if(e.isPopupTrigger())
                {
                    textMenu.show(_output, e.getX(), e.getY());
                }
            }
        });

        // Build the JTextArea where the user writes input messages.
        _input = new JTextArea("");
        _input.setLineWrap(true);
        _input.setEditable(true);
        _input.addKeyListener(new KeyListener()
        {
            public void
            keyTyped(KeyEvent e)
            {
                if(e.getKeyChar() == KeyEvent.VK_ENTER)
                {
                    Document doc = _input.getDocument();
                    try
                    {
                        String msg = doc.getText(0, doc.getLength()).trim();
                        if(msg.length() > 0)
                        {
                            _chat.begin_say(msg, new Demo.Callback_ChatSession_say()
                            {
                                @Override
                                public void
                                response()
                                {
                                }

                                @Override
                                public void
                                exception(final LocalException ex)
                                {
                                    appendMessage("<system-message> - " + ex);
                                }
                            });
                        }
                    }
                    catch(BadLocationException e1)
                    {
                    }

                    _input.setText("");
                }
            }

            public void
            keyPressed(KeyEvent e)
            {
            }

            public void
            keyReleased(KeyEvent e)
            {
            }
        });

        _outputScroll = new JScrollPane(_output);
        _outputScroll.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
        _outputScroll.setBorder(null);

        _outputScroll.setMinimumSize(new Dimension(100, 100));
        _outputScroll.setPreferredSize(new Dimension(100, 100));

        JSplitPane verticalSplit = new JSplitPane(JSplitPane.VERTICAL_SPLIT);
        verticalSplit.setTopComponent(_outputScroll);

        JScrollPane conversationInputScroll = new JScrollPane(_input);
        conversationInputScroll.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
        conversationInputScroll.setBorder(null);
        
        conversationInputScroll.setMinimumSize(new Dimension(100, 100));
        conversationInputScroll.setPreferredSize(new Dimension(100, 100));
        verticalSplit.setBottomComponent(conversationInputScroll);

        verticalSplit.setResizeWeight(0.9);

        _output.addComponentListener(new ComponentListener()
        {
            public void
            componentResized(ComponentEvent e)
            {
                JScrollBar vertivalScrollbar = _outputScroll.getVerticalScrollBar();
                vertivalScrollbar.setValue(vertivalScrollbar.getMaximum());
            }
            public void
            componentHidden(ComponentEvent e)
            {
            }

            public void
            componentMoved(ComponentEvent e)
            {
            }

            public void
            componentShown(ComponentEvent e)
            {
            }
        });

        add(verticalSplit, BorderLayout.CENTER);

        JPanel statusPanel = new JPanel();
        JSeparator statusPanelSeparator = new JSeparator();
        _status = new JLabel();
        _status.setText("Not connected");

        statusPanel.add(statusPanelSeparator, BorderLayout.NORTH);
        statusPanel.add(_status, BorderLayout.SOUTH);
        
        add(statusPanel, BorderLayout.SOUTH);

        JMenuBar menuBar = new JMenuBar();
        JMenu connectMenu = new JMenu("Session");

        _login = new AbstractAction("Login")
        {
            public void
            actionPerformed(ActionEvent e) 
            {
                login();
            }
        };

        _logout = new AbstractAction("Logout")
        {
            public void
            actionPerformed(ActionEvent e) 
            {
                setEnabled(false);
                _status.setText("Logging out");
                destroySession();
                _chat = null;
            }
        };
        _logout.setEnabled(false);

        _exit = new AbstractAction("Exit")
        {
            public void
            actionPerformed(ActionEvent e) 
            {
                exit();
            }
        };

        connectMenu.add(_login);
        connectMenu.add(_logout);
        if(!System.getProperty("os.name").startsWith("Mac OS"))
        {
            connectMenu.add(_exit);
        }

        menuBar.add(connectMenu);

        setJMenuBar(menuBar);

        setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);

        addWindowListener(new WindowAdapter()
        {
            public void
            windowClosing(WindowEvent e)
            {
                exit();
            }
        });

        pack();
        setSize(640, 480);
        locateOnScreen(this);
        setVisible(true);

        // Create the labels and text fields.
        JLabel hostLabel = new JLabel("Host:   ", JLabel.RIGHT);
        _hostField = new JTextField("", 12);
        _hostField.setText("127.0.0.1");
        JLabel userNameLabel = new JLabel("Username:   ", JLabel.RIGHT);
        _userNameField = new JTextField("", 12);
        _userNameField.setText("test");
        JLabel passwordLabel = new JLabel("Password:   ", JLabel.RIGHT);
        _passwordField = new JPasswordField("", 12);
        _connectionPanel = new JPanel(false);
        _connectionPanel.setLayout(new BoxLayout(_connectionPanel, BoxLayout.X_AXIS));

        JPanel labelPanel = new JPanel(false);
        labelPanel.setLayout(new GridLayout(0, 1));
        labelPanel.add(hostLabel);
        labelPanel.add(userNameLabel);
        labelPanel.add(passwordLabel);
        JPanel fieldPanel = new JPanel(false);
        fieldPanel.setLayout(new GridLayout(0, 1));
        fieldPanel.add(_hostField);
        fieldPanel.add(_userNameField);
        fieldPanel.add(_passwordField);         
        _connectionPanel.add(labelPanel);
        _connectionPanel.add(fieldPanel);

        _input.setEnabled(false);

        Ice.InitializationData initData = new Ice.InitializationData();

        // Load the configuration file.
        initData.properties = Ice.Util.createProperties();
        initData.properties.load("config.client");
        StringSeqHolder argHolder = new StringSeqHolder(args);
        initData.properties = Util.createProperties(argHolder, initData.properties);

        // Setup a dispatcher to dispath Ice and Glacier2 helper callbacks to the GUI thread.
        initData.dispatcher = new Ice.Dispatcher()
        {
            public void
            dispatch(Runnable runnable, Ice.Connection connection)
            {
                SwingUtilities.invokeLater(runnable);
            }
        };

        _factory = new SessionFactoryHelper(initData, new SessionCallback()
        {
            public void
            connected(SessionHelper session)
                throws SessionNotExistException
            {
                // If the session has been reassigned avoid the spurious callback.
                if(session != _session)
                {
                    return;
                }

                // The chat callback servant. We use an anonymous
                // inner class since the implementation is very
                // simple.
                Demo._ChatCallbackDisp servant = new Demo._ChatCallbackDisp()
                {
                    public void
                    message(final String data, Current current)
                    {                            
                        appendMessage(data);
                    }
                };

                Demo.ChatCallbackPrx callback = Demo.ChatCallbackPrxHelper.uncheckedCast(_session.addWithUUID(servant));

                _chat = Demo.ChatSessionPrxHelper.uncheckedCast(_session.session());
                _chat.begin_setCallback(callback, new Demo.Callback_ChatSession_setCallback()
                {
                    @Override
                    public void
                    response()
                    {
                        assert _loginDialog != null;
                        _loginDialog.dispose();
                        
                        _login.setEnabled(false);
                        _logout.setEnabled(true);
                        
                        _input.setEnabled(true);
                        
                        _status.setText("Connected with " + _hostField.getText());
                    }

                    @Override
                    public void
                    exception(LocalException ex)
                    {
                        destroySession();
                    }
                });
            }

            public void
            disconnected(SessionHelper session)
            {
                // If the session has been reassigned avoid the spurious callback.
                if(session != _session)
                {
                    return;
                }

                if(_loginDialog != null)
                {
                    _loginDialog.dispose();
                }

                _session = null;
                _chat = null;

                _login.setEnabled(true);
                _logout.setEnabled(false);

                _input.setEnabled(false);
                _status.setText("Not connected");
            }

            public void
            connectFailed(SessionHelper session, Throwable ex)
            {
                // If the session has been reassigned avoid the
                // spurious callback.
                if(session != _session)
                {
                    return;
                }

                if(_loginDialog != null)
                {
                    _loginDialog.dispose();
                }
                _status.setText(ex.getClass().getName());
            }

            public void
            createdCommunicator(SessionHelper session)
            {
            }
        });
        _factory.setRouterIdentity(new Ice.Identity("router", "DemoGlacier2"));

        login();
    }

    protected void
    login()
    {
        String[] options = {"Login", "Cancel" };
        // Show Login Dialog.
        int option = JOptionPane.showOptionDialog(this, _connectionPanel, "Login", JOptionPane.OK_CANCEL_OPTION,
                                                  JOptionPane.INFORMATION_MESSAGE, null, options, options[0]);

        if(option == 0)
        {
            _factory.setRouterHost(_hostField.getText());
            // Connect to Glacier2 using SessionFactoryHelper
            _session = _factory.connect(_userNameField.getText(), _passwordField.getText());
            String[] cancel = { "Cancel" };

            // Show Connecting Dialog
            JOptionPane pane = new JOptionPane("Please wait while connecting...", JOptionPane.INFORMATION_MESSAGE,
                                               JOptionPane.DEFAULT_OPTION, null, cancel, cancel[0]);
            _loginDialog = pane.createDialog(this, "Connecting");
            _loginDialog.setVisible(true);

            // User pressed cancel.
            if(pane.getValue() != JOptionPane.UNINITIALIZED_VALUE)
            {
                // Destroy session
                destroySession();
            }
        }
    }

    private void
    destroySession()
    {
        if(_session != null)
        {
            _session.destroy();
            //The session will be set to null on disconnected.
        }
    }

    private void
    exit()
    {
        destroySession();
        dispose();
        Runtime.getRuntime().exit(0);
    }

    public void
    appendMessage(String message)
    {
        Document doc = (Document) _output.getDocument();
        Element e = doc.getDefaultRootElement();
        AttributeSet attr = e.getAttributes().copyAttributes();
        try
        {
            doc.insertString(doc.getLength(), message + "\n", attr);
        }
        catch(BadLocationException ex)
        {
        }
        _output.setCaretPosition(doc.getLength());
    }

    private static void
    locateOnScreen(Component component)
    {
        Dimension paneSize = component.getSize();
        Dimension screenSize = component.getToolkit().getScreenSize();
        component.setLocation((screenSize.width - paneSize.width) / 2, (screenSize.height - paneSize.height) / 2);
    }
    
    private JLabel _status;
    private JTextArea _output;
    private JTextArea _input;
    private JScrollPane _outputScroll;
 
     // Login/Logout actions.
    private AbstractAction _login;
    private AbstractAction _logout;
    private AbstractAction _exit;

    // Login dialog
    private JDialog _loginDialog;
    private JTextField _userNameField;
    private JTextField _passwordField;
    private JTextField _hostField;
    private JPanel _connectionPanel;

    // The session factory and current session.
    private SessionFactoryHelper _factory;
    private SessionHelper _session;
    private Demo.ChatSessionPrx _chat;
}
