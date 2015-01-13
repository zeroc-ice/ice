// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
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
import android.app.DialogFragment;
import android.content.Intent;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.inputmethod.EditorInfo;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;

import com.zeroc.chat.service.ChatRoomListener;

public class ChatActivity extends SessionActivity
{
    private static final int USERS_ID = Menu.FIRST;
    private static final int LOGOUT_ID = Menu.FIRST + 1;
    private static final int MAX_MESSAGE_SIZE = 1024;
    public static final String MESSAGE_TOO_LONG_TAG = "mtl";

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
            showDialogFatal();
        }

        public void inactivity()
        {
            showDialogFatal();
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

    public static class MessageTooLongDialogFragment extends DialogFragment
    {
        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState)
        {
            AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
            builder.setTitle("Error")
                   .setMessage("Message length exceeded, maximum length is " + MAX_MESSAGE_SIZE + " characters.");
            return builder.create();
        }
    }

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
        _text.setOnEditorActionListener(new TextView.OnEditorActionListener()
        {
            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event)
            {
                if(actionId == EditorInfo.IME_ACTION_DONE)
                {
                    sendText();
                    return true;
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

    private void sendText()
    {
        String t = _text.getText().toString().trim();
        if(t.length() == 0)
        {
            return;
        }
        if(t.length() > MAX_MESSAGE_SIZE)
        {
            DialogFragment dialog = new MessageTooLongDialogFragment();
            dialog.show(getFragmentManager(), MESSAGE_TOO_LONG_TAG);
            return;
        }
        _text.setText("");

        _service.send(t);
    }
}
