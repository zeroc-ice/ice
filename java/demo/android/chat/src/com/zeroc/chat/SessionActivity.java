// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.chat;

import com.zeroc.chat.service.ChatRoomListener;
import com.zeroc.chat.service.ChatService;
import com.zeroc.chat.service.NoSessionException;
import com.zeroc.chat.service.Service;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.ComponentName;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;

// Base of any activities that use the chat session.
public abstract class SessionActivity extends Activity
{
    protected static final int DIALOG_FATAL = 1;
    protected Service _service;

    abstract ChatRoomListener getChatRoomListener();
    abstract boolean replayEvents();

    final private ServiceConnection _connection = new ServiceConnection()
    {
        public void onServiceConnected(ComponentName name, IBinder service)
        {
            // This is called when the connection with the service has been
            // established, giving us the service object we can use to
            // interact with the service. Because we have bound to a explicit
            // service that we know is running in our own process, we can
            // cast its IBinder to a concrete class and directly access it.
            _service = ((com.zeroc.chat.service.ChatService.LocalBinder)service).getService();
            try
            {
                String hostname = _service.addChatRoomListener(getChatRoomListener(), replayEvents());
                setTitle(hostname);
            }
            catch(NoSessionException e)
            {
                finish();
            }
        }

        public void onServiceDisconnected(ComponentName name)
        {
        }
    };

    @Override
    public void onResume()
    {
        super.onResume();
        bindService(new Intent(SessionActivity.this, ChatService.class), _connection, BIND_AUTO_CREATE);
    }

    @Override
    public void onStop()
    {
        super.onStop();
        unbindService(_connection);
        if(_service != null)
        {
            _service.removeChatRoomListener(getChatRoomListener());
            _service = null;
        }
    }

    @Override
    protected Dialog onCreateDialog(final int id)
    {
        switch(id)
        {
        case DIALOG_FATAL:
        {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setTitle("Error");
            builder.setMessage(""); // Details provided in onPrepareDialog
            builder.setCancelable(false);
            builder.setPositiveButton("Ok", new DialogInterface.OnClickListener()
            {
                public void onClick(DialogInterface dialog, int whichButton)
                {
                    _service.logout();
                    finish();
                }
            });
            return builder.create();
        }

        }

        return null;
    }

    @Override
    protected void onPrepareDialog(int id, Dialog dialog)
    {
        super.onPrepareDialog(id, dialog);
        if(id == DIALOG_FATAL)
        {
            AlertDialog alert = (AlertDialog)dialog;
            alert.setMessage(_service.getSessionError());
        }
    }
}
