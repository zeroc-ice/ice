//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <Hello.h>

namespace hello
{
    ref class MainPage;

    class HelloCallback : virtual public IceUtil::Shared
    {
    public:

        HelloCallback(MainPage^);

        void helloSent(bool);

        void helloSuccess();

        void helloFailure(const Ice::Exception&);
        
    private:

        MainPage^ _page;
    };
    typedef IceUtil::Handle<HelloCallback> HelloCallbackPtr;

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

        friend class HelloCallback;

        Demo::HelloPrx proxy();
        bool isBatch();

        void helloSuccess();
        void helloFailure(const Ice::Exception& ex);
        void helloSent(bool);
        void print(const std::string&);

        void hello_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void shutdown_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void flush_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

        Ice::CommunicatorPtr _communicator;
    };
}
