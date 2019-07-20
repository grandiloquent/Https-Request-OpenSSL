#include <iostream>
#include <sstream>
#include <map> // std::map
#include "Request.h"
#include "Utils.h"

std::string GetMusicFile(const char* musicId)
{

    std::stringstream stream;

    const char* guid = "365305415";

    stream << "/v8/fcg-bin/fcg_play_single_song.fcg"
           << "?songmid="
           << musicId
           << "&platform=yqq&format=json";

    Request request;
    auto json = request.Touch("c.y.qq.com",
        stream.str().c_str(),
        NULL,
        true,
        false,
        Request::GET);

    if (json.length() == 0)
    {
        return std::string();
    }

    auto mediaMid = FindObj(json, "\"media_mid\"");

    if (mediaMid.length() == 0)
    {
        return std::string();
    }

    /* ================================
	 * 
     * ================================
     */
    std::map<std::string, std::string> map{
        { "M800", "mp3" },
        { "C600", "m4a" },
        { "M500", "mp3" },
        { "C400", "m4a" },
        { "M200", "mp3" },
        { "M100", "mp3" }
    };

    for (std::map<std::string, std::string>::iterator i = map.begin(), end = map.end(); i != end; ++i)
    {

        stream.str(std::string());

        stream << "/base/fcgi-bin/fcg_musicexpress.fcg?json=3&guid="
               << guid
               << "&format=json";

        auto key = request.Touch("c.y.qq.com",
            stream.str().c_str(),
            NULL,
            true,
            false,
            Request::GET);
        if (key.length() == 0)
        {
            return std::string();
        }
        key = FindObj(key, "\"key\"");
        if (key.length() == 0)
        {
            return std::string();
        }
        std::cout << key << std::endl;
        return std::string();
    }

    return std::string();
}

int main()
{

    GetMusicFile("003IjD3G3xCgJv");
    // Request request;

    // std::string body = request.Touch("cn.bing.com", "/search?q=github%20search%20in%20filename&qs=n&form=QBRE&sp=-1&pq=github%20search%20i%20filename&sc=0-24&sk=&cvid=10BBE06651CD4C31B064731802EFF839", Request::GET);

    // std::cout << body << std::endl;

    return 0;
}