// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.chat;

import android.app.DialogFragment;
import android.os.Bundle;
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
    private static final String FATAL_TAG = "fatal";
    protected Service _service;

    abstract ChatRoomListener getChatRoomListener();
    abstract boolean replayEvents();

    public static class FatalDialogFragment extends DialogFragment
    {
        public static FatalDialogFragment newInstance(String message)
        {
            FatalDialogFragment frag = new FatalDialogFragment();
            Bundle args = new Bundle();
            args.putString("message", message);
            frag.setArguments(args);
            return frag;
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState)
        {
            AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
            builder.setTitle("Error")
                    .setMessage(getArguments().getString("message"))
                    .setCancelable(false)
                    .setPositiveButton("Ok", new DialogInterface.OnClickListener()
                    {
                        public void onClick(DialogInterface dialog, int whichButton)
                        {
                            ((SessionActivity) getActivity()).fatalOk();
                        }
                    });
            return builder.create();
        }
    }

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

    void showDialogFatal()
    {
        DialogFragment dialog = FatalDialogFragment.newInstance(
                (_service != null) ? _service.getSessionError() : "");
        dialog.show(getFragmentManager(), FATAL_TAG);
    }

    private void fatalOk()
    {
        _service.logout();
        finish();
    }
}
