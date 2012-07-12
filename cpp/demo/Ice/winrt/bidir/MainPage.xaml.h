//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <Callback.h>

namespace bidir
{
    ref class MainPage;

    class CallbackReceiverI : public Demo::CallbackReceiver
    {
    public:

        CallbackReceiverI(MainPage^ page) : _page(page)
        {
        }

        virtual void
        callback(Ice::Int, const Ice::Current&);
    
    private:
        
        MainPage^ _page;
    };

    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public ref class MainPage sealed
    {
    public:
        MainPage();

    protected:

        virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;

    private:

        friend class CallbackReceiverI;

        void callback(Ice::Int, const Ice::Current&);
        void print(const std::string&);
        void startClient_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void stopClient_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        Ice::CommunicatorPtr _communicator;
    };
}
