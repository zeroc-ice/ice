//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
#include "ChatView.xaml.h"
#include "LoginView.xaml.h"

#include <Glacier2/Glacier2.h>
#include <Chat.h>

namespace chat
{
    struct LoginData
    {
        std::string hostname;
        std::string username;
        std::string password;
    };
    
    class Coordinator : virtual public Glacier2::SessionCallback,
                        virtual public Demo::ChatCallback
    {
    public:

        Coordinator(Windows::UI::Core::CoreDispatcher^);

        void signIn(LoginData);
        LoginData loginData();
        
        //
        //  Session callback
        //
        virtual void createdCommunicator(const Glacier2::SessionHelperPtr& session);
        virtual void connected(const Glacier2::SessionHelperPtr&);
        virtual void disconnected(const Glacier2::SessionHelperPtr&);
        virtual void connectFailed(const Glacier2::SessionHelperPtr&, const Ice::Exception&);
    
        //
        // Chat callback
        //
        virtual void message(const std::string& data, const Ice::Current&);

        void setCallbackSuccess();
        void setCallbackError(const Ice::Exception&);

        void say(const std::string&);
        void sayCallbackSuccess();
        void sayCallbackError(const Ice::Exception&);

        void destroy();

    private:

        Demo::ChatSessionPrx _chat;
        Glacier2::SessionHelperPtr _session;
        Windows::UI::Core::CoreDispatcher^ _dispatcher;
        LoginData _loginData;
    };
    typedef IceUtil::Handle<Coordinator> CoordinatorPtr;

    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public ref class MainPage sealed
    {
    public:

        MainPage();

        static MainPage^ instance();

        void setConnected(bool);

    protected:

        virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;

    private:
        
        virtual void setError(const std::string&);

        CoordinatorPtr coordinator()
        {
            return _coordinator;
        }    
        CoordinatorPtr _coordinator;

        static MainPage^ _instance;

        friend ref class LoginView;
        friend ref class ChatView;
        friend class Coordinator;

        LoginView^ _loginView;
        ChatView^ _chatView;
        void signoutClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    };
}
