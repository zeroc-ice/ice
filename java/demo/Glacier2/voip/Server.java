// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

import Glacier2.CannotCreateSessionException;
import Glacier2.SessionControlPrx;
import Glacier2.SessionPrx;
import Glacier2._PermissionsVerifierDisp;
import Glacier2._SessionManagerDisp;
import Ice.Current;
import Ice.LocalException;
import Ice.NotRegisteredException;
import Ice.ObjectNotExistException;
import Ice.StringHolder;
import Voip.Callback_Control_incomingCall;
import Voip.ControlPrx;
import Voip.SessionPrxHelper;
import Voip._SessionDisp;

public class Server extends Ice.Application
{
    private final ScheduledExecutorService timer = new ScheduledThreadPoolExecutor(1);

    class SessionI extends _SessionDisp
    {
        private ControlPrx _ctrl;
        private long _timestamp = System.currentTimeMillis();

        public void setControl(ControlPrx ctrl, Current current)
        {
            _ctrl = ctrl;
        }

        public void simulateCall(int delay, Current current)
        {
            timer.schedule(new Runnable()
            {
                public void run()
                {
                    if(_ctrl != null)
                    {
                        System.out.println("calling incoming call");
                        _ctrl.begin_incomingCall(new Callback_Control_incomingCall()
                            {
                                public void exception(LocalException ex)
                                {
                                    System.out.println("incoming call failed");
                                    ex.printStackTrace();
                                }

                                public void response()
                                {
                                    System.out.println("incoming call succeeded");
                                }
                            });
                    }
                }
            }, delay, TimeUnit.MILLISECONDS);
        }

        public void destroy(Current current)
        {
            try
            {
                current.adapter.remove(current.id);
            }
            catch(NotRegisteredException ex)
            {
                // Ignore.
            }
        }

        public void refresh(Current current)
        {
            _timestamp = System.currentTimeMillis();
        }
        
        long getTimestamp()
        {
            return _timestamp;
        }
    }

    class PermissionsVerifierI extends _PermissionsVerifierDisp
    {
        public boolean checkPermissions(String userId, String password, StringHolder reason, Current current)
        {
            return true;
        }
    }

    class SessionManagerI extends _SessionManagerDisp
    {
        public SessionPrx create(String userId, SessionControlPrx control,
                Current current) throws CannotCreateSessionException
        {
             // The configured timeout must be greater than 600. This is 601 * 2.
            final long sessionTimeout = 1202;
            final SessionI session = new SessionI();
            final SessionPrx proxy = SessionPrxHelper.uncheckedCast(current.adapter.addWithUUID(session));
            timer.scheduleWithFixedDelay(new Runnable()
                {
                    public void run()
                    {
                        // If the session has already been destroyed the ONE will
                        // fall out of run canceling the task.
                        if (System.currentTimeMillis() - session.getTimestamp() > (sessionTimeout * 1000L * 2))
                        {
                            proxy.destroy();
                            throw new ObjectNotExistException();
                        }
                    }
                }, sessionTimeout, sessionTimeout, TimeUnit.SECONDS);
            
            return proxy;
        }
    };
        
    public int
    run(String[] args)
    {
        if(args.length > 0)
        {
            System.err.println(appName() + ": too many arguments");
            return 1;
        }

        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("VoipServer");
        adapter.add(new PermissionsVerifierI(), communicator().stringToIdentity("VoipVerifier"));
        adapter.add(new SessionManagerI(), communicator().stringToIdentity("VoipSessionManager"));
        adapter.activate();
        communicator().waitForShutdown();
        return 0;
    }

    public static void
    main(String[] args)
    {
        Server app = new Server();
        int status = app.main("Server", args, "config.server");
        System.exit(status);
    }

}
