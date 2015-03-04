// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.chat;

import java.util.LinkedList;
import java.util.List;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Intent;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnKeyListener;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ListView;

import com.zeroc.chat.service.ChatRoomListener;

public class ChatActivity extends SessionActivity
{
    private static final int DIALOG_MESSAGE_TOO_LONG = 2;

    private static final int USERS_ID = Menu.FIRST;
    private static final int LOGOUT_ID = Menu.FIRST + 1;
    private static final int MAX_MESSAGE_SIZE = 1024;

    private EditText _text;
    private ArrayAdapter<String> _adapter;
    private LinkedList<String> _strings = new LinkedList<String>();
    final private ChatRoomListener _listener = new ChatRoomListener()
    {
        public void init(final List<String> users)
        {
            _strings.clear();
            _adapter.notifyDataSetChanged();
        }

        public void join(long timestamp, String name)
        {
            add(ChatUtils.formatTimestamp(timestamp) + " - <system-message> - " + name + " joined.");
        }

        public void leave(long timestamp, String name)
        {
            add(ChatUtils.formatTimestamp(timestamp) + " - " + "<system-message> - " + name + " left.");
        }

        public void send(long timestamp, String name, final String message)
        {
            add(ChatUtils.formatTimestamp(timestamp) + " - <" + name + "> " + ChatUtils.unstripHtml(message));
        }

        public void error()
        {
            showDialog(DIALOG_FATAL);
        }

        public void inactivity()
        {
            showDialog(DIALOG_FATAL);
        }

        private void add(final String msg)
        {
            _strings.add(msg);
            if(_strings.size() > 200) // AppSession.MAX_MESSAGES)
            {
                _strings.removeFirst();
            }
            _adapter.notifyDataSetChanged();
        }
    };

    @Override
    ChatRoomListener getChatRoomListener()
    {
        return _listener;
    }

    @Override
    boolean replayEvents()
    {
        return true;
    }

    // Hook the back key to logout the session.
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event)
    {
        if(keyCode == KeyEvent.KEYCODE_BACK)
        {
            _service.logout();
            finish();
            return true;
        }
        return false;
    }

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.chat);

        _text = (EditText)findViewById(R.id.text);

        ListView transcript = (ListView)findViewById(R.id.list);
        _adapter = new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, _strings);
        transcript.setAdapter(_adapter);

        _text.setOnClickListener(new OnClickListener()
        {
            public void onClick(View v)
            {
                sendText();
            }
        });
        _text.setOnKeyListener(new OnKeyListener()
        {
            public boolean onKey(View v, int keyCode, KeyEvent event)
            {
                if(event.getAction() == KeyEvent.ACTION_DOWN)
                {
                    switch (keyCode)
                    {
                    case KeyEvent.KEYCODE_DPAD_CENTER:
                    case KeyEvent.KEYCODE_ENTER:
                        sendText();
                        return true;
                    }
                }
                return false;
            }
        });
        _text.requestFocus();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu)
    {
        boolean result = super.onCreateOptionsMenu(menu);
        menu.add(0, USERS_ID, 0, R.string.menu_users);
        menu.add(0, LOGOUT_ID, 0, R.string.menu_logout);
        return result;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item)
    {
        switch (item.getItemId())
        {
        case USERS_ID:
            startActivity(new Intent(ChatActivity.this, UserViewActivity.class));
            return true;

        case LOGOUT_ID:
            _service.logout();
            finish();
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    @Override
    protected Dialog onCreateDialog(final int id)
    {
        if(id == DIALOG_MESSAGE_TOO_LONG)
        {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setTitle("Error");
            builder.setMessage("Message length exceeded, maximum length is " + MAX_MESSAGE_SIZE + " characters.");
            return builder.create();
        }

        return super.onCreateDialog(id);
    }

    private void sendText()
    {
        String t = _text.getText().toString().trim();
        if(t.length() == 0)
        {
            return;
        }
        if(t.length() > MAX_MESSAGE_SIZE)
        {
            showDialog(DIALOG_MESSAGE_TOO_LONG);
            return;
        }
        _text.setText("");

        _service.send(t);
    }
}
