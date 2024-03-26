//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceGridGUI;

import java.awt.event.ActionEvent;
import javax.swing.AbstractAction;
import javax.swing.Action;

import com.zeroc.IceGridGUI.LiveDeployment.*;

//
// Holds all actions for the "Live Deployment" view
//
public class LiveActions
{
    class SendSignal extends AbstractAction
    {
        SendSignal(String name)
        {
            super(name);
            _name = name;
            putValue(Action.SHORT_DESCRIPTION, "Send " + _name);
        }

        @Override
        public void actionPerformed(ActionEvent e)
        {
            _target.signal(_name);
        }
        private String _name;
    }

    public Action get(int index)
    {
        return _array[index];
    }

    public boolean[] setTarget(TreeNode target)
    {
        _target = target;

        boolean[] availableActions;
        if(_target == null)
        {
            availableActions = new boolean[TreeNode.ACTION_COUNT];
        }
        else
        {
            availableActions = _target.getAvailableActions();
        }

        for(int i = 0; i < _array.length; ++i)
        {
            _array[i].setEnabled(availableActions[i]);
        }
        return availableActions;
    }

    LiveActions()
    {
        _array[TreeNode.START] = new AbstractAction("Start", Utils.getIcon("/icons/16x16/start.png"))
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    _target.start();
                }
            };
        _array[TreeNode.START].putValue(Action.SHORT_DESCRIPTION, "Start this server or service");

        _array[TreeNode.STOP] = new AbstractAction("Stop", Utils.getIcon("/icons/16x16/stop.png"))
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    _target.stop();
                }
            };
        _array[TreeNode.STOP].putValue(Action.SHORT_DESCRIPTION, "Stop this server or service");

        _array[TreeNode.ENABLE] = new AbstractAction("Enable", Utils.getIcon("/icons/16x16/enable.png"))
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    _target.enable();
                }
            };
        _array[TreeNode.ENABLE].putValue(Action.SHORT_DESCRIPTION, "Enable this server");

        _array[TreeNode.DISABLE] = new AbstractAction("Disable", Utils.getIcon("/icons/16x16/disable.png"))
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    _target.disable();
                }
            };
        _array[TreeNode.DISABLE].putValue(Action.SHORT_DESCRIPTION, "Disable this server");

        _array[TreeNode.SIGHUP] = new SendSignal("SIGHUP");
        _array[TreeNode.SIGINT] = new SendSignal("SIGINT");
        _array[TreeNode.SIGQUIT] = new SendSignal("SIGQUIT");
        _array[TreeNode.SIGKILL] = new SendSignal("SIGKILL");
        _array[TreeNode.SIGUSR1] = new SendSignal("SIGUSR1");
        _array[TreeNode.SIGUSR2] = new SendSignal("SIGUSR2");
        _array[TreeNode.SIGTERM] = new SendSignal("SIGTERM");

        _array[TreeNode.WRITE_MESSAGE] = new AbstractAction("Write Message")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    _target.writeMessage();
                }
            };
        _array[TreeNode.WRITE_MESSAGE].putValue(Action.SHORT_DESCRIPTION, "Write message to stdout or stderr");

        _array[TreeNode.RETRIEVE_ICE_LOG] = new AbstractAction("Retrieve Ice log")
        {
            @Override
            public void actionPerformed(ActionEvent e)
            {
                _target.retrieveIceLog();
            }
        };
        _array[TreeNode.RETRIEVE_ICE_LOG].putValue(Action.SHORT_DESCRIPTION, "Attach RemoteLogger to Ice logger");

        _array[TreeNode.RETRIEVE_STDOUT] = new AbstractAction("Retrieve stdout")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    _target.retrieveOutput(true);
                }
            };
        _array[TreeNode.RETRIEVE_STDOUT].putValue(Action.SHORT_DESCRIPTION, "Retrieve stdout if redirected to file");

        _array[TreeNode.RETRIEVE_STDERR] = new AbstractAction("Retrieve stderr")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    _target.retrieveOutput(false);
                }
            };
        _array[TreeNode.RETRIEVE_STDERR].putValue(Action.SHORT_DESCRIPTION, "Retrieve stderr if redirected to file");

        _array[TreeNode.RETRIEVE_LOG_FILE] = new AbstractAction("Retrieve log file")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    _target.retrieveLogFile();
                }
            };
        _array[TreeNode.RETRIEVE_LOG_FILE].putValue(Action.SHORT_DESCRIPTION, "Retrieve log file");

        _array[TreeNode.SHUTDOWN_NODE] = new AbstractAction("Shutdown")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    _target.shutdownNode();
                }
            };

        _array[TreeNode.SHUTDOWN_REGISTRY] = new AbstractAction("Shutdown")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    _target.shutdownRegistry();
                }
            };

        _array[TreeNode.ADD_OBJECT] = new AbstractAction("Add Well-known Object")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    _target.addObject();
                }
            };

        _array[TreeNode.OPEN_DEFINITION] = new AbstractAction("Open Definition")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    _target.openDefinition();
                }
            };

        _array[TreeNode.ENABLE_METRICS_VIEW] = new AbstractAction("Enable Metrics View")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    _target.enableMetricsView(true);
                }
            };

        _array[TreeNode.DISABLE_METRICS_VIEW] = new AbstractAction("Disable Metrics View")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    _target.enableMetricsView(false);
                }
            };

        _array[TreeNode.START_ALL_SERVERS] =  new AbstractAction("Start All Servers")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    _target.startAllServers();
                }
            };

        _array[TreeNode.STOP_ALL_SERVERS] =  new AbstractAction("Stop All Servers")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    _target.stopAllServers();
                }
            };
    }

    private TreeNode _target;
    private Action[] _array = new Action[TreeNode.ACTION_COUNT];
}
