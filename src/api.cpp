#include "api.h"

using nlohmann::json;

#ifdef WIN32
static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
static std::wstring to_wstring(const std::string& string) {
    return converter.from_bytes(string);
}
static std::string to_string(const std::wstring& wstring) {
    return converter.to_bytes(wstring);
}
#endif

API::API(bool* empty) : empty_trigger(empty) {
    quit = false;

    // Clone thread
    clone_thread = std::thread([this](){        
        while (true) {
            // Wait for entries or quit
            if (clone_queue.empty()) *empty_trigger = true;
            while (clone_queue.empty() && !quit) {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            } if (quit) {
                break;
            } *empty_trigger = false;

            // Get username
            kidCurl reqHandler;
            std::string uuid, skin_url, buffer;
            Profile profile = clone_queue.dequeue();
        
            // UUID
            if (profile.type == API::Profile::Type::username) {
                auto uuid_req = reqHandler.Send(kidCurl::Type::GET, API_USERNAME + profile.data);
                if (uuid_req && uuid_req->status_code == 200) {
                    json info = json::parse(uuid_req->body, nullptr, false);
                    if (!info.is_discarded()) {
                        if (info["id"].is_string()) {
                            uuid = info["id"];
                        }
                    }
                }
            } else {
                uuid = profile.data;
            }

            // Skin info
            if (!uuid.empty()) {
                auto skin_info_request = reqHandler.Send(kidCurl::Type::GET, API_PROFILE + uuid);
                if (skin_info_request && skin_info_request->status_code == 200) {
                    json skin_info = json::parse(skin_info_request->body, nullptr, false);
                    if (!skin_info.is_discarded()) {
                        if (skin_info["properties"][0]["value"].is_string()) {
                            const std::string& skin_data_base64 = skin_info["properties"][0]["value"];
                            std::string skin_data_raw;
                            skin_data_raw.resize(BASE64_DECODE_OUT_SIZE(skin_data_base64.length()));

                            base64_decode(skin_data_base64.c_str(), skin_data_base64.length(), (unsigned char*)skin_data_raw.c_str());
                            skin_info = json::parse(skin_data_raw, nullptr, false);
                            if (!skin_info.is_discarded()) {
                                skin_info["textures"]["SKIN"]["url"].is_string() ? skin_url = skin_info["textures"]["SKIN"]["url"] : "";
                            }
                        }
                    }
                }
            }

            // Skin
            if (!skin_url.empty()) {
                auto skin_request = reqHandler.Send(kidCurl::Type::GET, skin_url);
                if (skin_request && skin_request->status_code == 200) {
                    buffer = skin_request->body;
                }
            }

            // Save
            if (!buffer.empty()) {
                save_queue.enqueue({ profile.data, buffer });
            }
        } 
    });

    // Save thread
    save_thread = std::thread([this](){
        NFD_Init();
        while (true) {
            // Wait for entries or quit
            while (save_queue.empty() && !quit) {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            } if (quit) {
                break;
            }

            // Get buffer
            SkinBuffer skin = save_queue.dequeue();

            // Save
            nfdu8char_t* out_path;
            nfdu8filteritem_t filter_item[1] = { {"PNGs", "png"} };
            nfdresult_t result = NFD_SaveDialogU8(&out_path, filter_item, 1, nullptr, std::string(skin.save_name + ".png").c_str());
            if (result == NFD_OKAY) {
                #ifdef WIN32
                std::fstream f(to_wstring(out_path), std::ios::binary | std::ios::out);
                #else
                std::fstream f(out_path, std::ios::binary | std::ios::out);
                #endif
                if (f.is_open()) {
                    f << skin.binary;
                } f.close();
                NFD_FreePathU8(out_path);
            }
        }
        NFD_Quit();
    });
}

API::~API() {
    quit = true;
    clone_thread.join();
    save_thread.join();
}

void API::skin(const Profile& profile) {
    clone_queue.enqueue(profile);
}