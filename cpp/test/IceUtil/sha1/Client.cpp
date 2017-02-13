// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/SHA1.h>
#include <TestCommon.h>

using namespace std;

namespace
{

struct SHA1Item
{
    const char* data;
    const char* digest;
};

SHA1Item items[] =
{
    {"hello world", "2aae6c35c94fcfb415dbe95f408b9ce91ee846ed"},
    {"2aae6c35c94fcfb415dbe95f408b9ce91ee846ed", "d6b0d82cea4269b51572b8fab43adcee9fc3cf9a"},
    {"Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
     "Cras vel ante adipiscing, aliquet ipsum sit amet, suscipit ante. "
     "Donec tempor purus in suscipit tempus. "
     "Vestibulum vulputate ipsum in quam bibendum laoreet. "
     "Etiam tristique viverra metus, at condimentum eros scelerisque et. "
     "Proin in diam sit amet nisi facilisis dictum. "
     "Donec placerat libero a mauris tristique, "
     "id pulvinar magna pharetra. Vivamus posuere tincidunt elit, "
     "eget lacinia nisl feugiat in. Aenean neque mi, "
     "vehicula a vestibulum nec, condimentum vel ipsum. "
     "Duis sit amet quam sit amet risus sollicitudin euismod. "
     "Nullam elit neque, suscipit ac convallis non, convallis et urna. "
     "Sed semper elementum erat, ut egestas lacus interdum sit amet. "
     "Sed ipsum nisi, ultricies congue augue id, euismod posuere sapien. "
     "Fusce volutpat nisl in orci laoreet, vitae lacinia urna venenatis.", 
     "84fac2d6a78ab4acbeae1408ab4de49c4bd145bb"},
    {"Pellentesque accumsan mauris neque, nec sollicitudin metus ornare eu. "
     "Sed dignissim nisi neque, sit amet dapibus purus faucibus laoreet. "
     "Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. "
     "Donec cursus risus sed arcu varius posuere. Donec pretium felis mi, eu consequat arcu imperdiet vitae. "
     "Etiam fringilla velit in nisl semper, nec mattis turpis blandit. "
     "Etiam magna sem, laoreet eu urna vitae, laoreet dictum tortor. In et iaculis enim, nec pharetra ipsum. "
     "Sed porttitor suscipit ligula mattis sollicitudin. Cras imperdiet iaculis dictum. "
     "Ut vel nisl condimentum, placerat ante a, suscipit turpis",
     "0ecd4503f02f3febeca1460fb81e8f7fc0d59abe"},
    {"Curabitur in dolor justo. Morbi vitae consectetur odio. Aliquam erat volutpat. "
     "Fusce consectetur magna nec metus ornare pretium. Ut tincidunt mi eu bibendum pharetra. "
     "Morbi sit amet elit augue. Praesent et enim condimentum, ullamcorper est tempor, auctor leo. "
     "Pellentesque eu convallis urna, ac auctor orci. Cras sapien lectus, convallis congue nunc nec, "
     "porta molestie est. Aliquam consectetur elementum posuere. Duis et mollis lorem.",
     "b4d38ece814fdfe9ff067d08c6389c6512530bb4"}
};

int itemsSize = sizeof(items)/sizeof(SHA1Item);

string toHex(const string& data)
{
    ostringstream os;
    for(size_t i = 0; i < data.size(); ++i)
    {
        os.width(2);
        os.fill('0');
        unsigned char c = data[i];
        os << hex << (int)c;
    }
    return os.str();
}

}

int main(int argc, char* argv[])
{
    cout << "Testing sha1 hash computation... ";
    for(int i = 0; i < itemsSize; ++i)
    {
        const SHA1Item* item = &items[i];
        vector<unsigned char> buffer;
        IceInternal::sha1(reinterpret_cast<const unsigned char*>(item->data), strlen(item->data), buffer);
        test(buffer.size() == 20);
        string digest = toHex(string(reinterpret_cast<const char*>(&buffer[0]), 20));
        test(item->digest == digest);
    }
    
    for(int i = 0; i < itemsSize; ++i)
    {
        const SHA1Item* item = &items[i];
        IceInternal::SHA1 hasher;
        //
        // Test adding the data in chunks
        //
        const unsigned char* begin = reinterpret_cast<const unsigned char*>(&item->data[0]);
        const unsigned char* end = begin + strlen(item->data);
        while(begin != end)
        {
            size_t packetSize = min(static_cast<size_t>(5), static_cast<size_t>(end - begin));
            hasher.update(begin, packetSize);
            begin += packetSize;
        }
        vector<unsigned char> buffer;
        hasher.finalize(buffer);
        test(buffer.size() == 20);
        string digest = toHex(string(reinterpret_cast<const char*>(&buffer[0]), 20));
        test(item->digest == digest);
    }
    cout << "ok" << endl;
    return EXIT_SUCCESS;
}
