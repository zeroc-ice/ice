// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package com.zeroc.chat.service;

public interface ChatRoomListener
{
    /**
    *
    * Called when the listener is attached to the chat session.
    * This provides the client with the initial list of users
    * currently in the chat room.
    *
    * @param users The names of users currently in the chat room.
    *
    **/
   void init(java.util.List<String> users);

   /**
    *
    * Called when a new user joins the chat room.
    *
    * @param name The name of the user that joined the chat room.
    *
    * @param timestamp The time at which the user joined the chat room.
    *
    **/
   void join(long timestamp, String name);

   /**
    *
    * Called when a user leaves the chat room.
    *
    * @param name The name of the user that joined the chat room.
    *
    * @param timestamp The time at which the user joined the chat room.
    *
    **/
   void leave(long timestamp, String name);
   /**
    *
    * Called when a user sends a message.
    *
    * @param name The name of the user that send the message.
    *
    * @param message The contents of the message.
    *
    * @param timestamp The time at which the message was sent.
    *
    **/
   void send(long timestamp, String name, String message);

   /**
    *
    * Called upon an error when communicating with the chat room. Once this
    * method is called the chat session has been destroyed.
    *
    */
   void error();

   /**
    *
    * Called if the chat room has been inactive for too long. Once this
    * method is called the chat session has been destroyed.
    */
   void inactivity();
}
