#ifndef API_H
#define API_H

#include <chrono>
#include <thread>
#include <string>
#include <fstream>
#include <mutex>
#include <atomic>

#ifdef WIN32
#include <locale>
#include <codecvt>
#include <string>
#endif

#include "tsqueue.hpp"

#include <kidCurl.hpp>

#include <nlohmann/json.hpp>

extern "C" {
    #include <base64.h>
}

#include <nfd.hpp>

class API
{
public:
    API(bool* empty_trigger);
    ~API();
    
public:
    struct Profile;
    void skin(const Profile& username);

public:
    struct Profile {
        std::string data;
        enum class Type {
            username = 0,
            uuid
        } type;
    };

private:
    struct SkinBuffer {
        std::string save_name;
        std::string binary;
    };

private:
    std::thread clone_thread;
    std::thread save_thread;

    TSQueue<Profile> clone_queue;
    TSQueue<SkinBuffer> save_queue;

    std::atomic<bool> quit;
    bool* empty_trigger = nullptr;

    std::string API_USERNAME = "https://api.mojang.com/users/profiles/minecraft/";
    std::string API_PROFILE = "https://sessionserver.mojang.com/session/minecraft/profile/";
};

#endif