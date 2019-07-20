#include <iostream>
#include <sstream>
#include <unordered_map>
#include <map> // std::map
#include "Request.h"
#include "Utils.h"

std::string GetMusicFile(const char* musicId)
{

    std::unordered_map<const char*, const char*> headers{
        { "User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/73.0.3683.56 Safari/537.36" },
    };

    std::stringstream stream;

    const char* guid = "365305411";

    stream << "/v8/fcg-bin/fcg_play_single_song.fcg"
           << "?songmid="
           << musicId
           << "&platform=yqq&format=json";

    Request request;
    auto json = request.Touch("c.y.qq.com",
        stream.str().c_str(),
        headers,
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
    std::unordered_map<std::string, std::string> map{
        // { "M100", "mp3" },
        // { "M200", "mp3" },
        // { "C400", "m4a" },
        { "M500", "mp3" },
        // { "C600", "m4a" },
        // { "M800", "mp3" }

    };

    const char* host = "dl.stream.qqmusic.qq.com";
    headers["Referer"] = "https://y.qq.com/portal/player.html";
    for (std::unordered_map<std::string, std::string>::iterator i = map.begin(), end = map.end(); i != end; ++i)
    {

        stream.str(std::string());

        stream << "/base/fcgi-bin/fcg_musicexpress.fcg?json=3&guid="
               << guid
               << "&format=json";

        auto key = request.Touch("c.y.qq.com",
            stream.str().c_str(),
            headers,
            true,
            false,
            Request::GET);
        if (key.length() == 0)
        {
            return std::string();
        }
        std::cout << key << std::endl;
        key = FindObj(key, "\"key\"");
        if (key.length() == 0)
        {
            return std::string();
        }

        stream.str(std::string());
        stream << i->first
               << mediaMid
               << "."
               << i->second
               << "?vkey="
               << key
               << "&guid="
               << guid
               << "&uid=0&fromtag=30";

        std::cout << "https://" << host << "/" << stream.str() << std::endl;
        bool ok = request.Ok(host, stream.str().c_str(), true);
        std::cout
            << ok << std::endl;
        if (ok)
            return std::string();
    }

    stream.str(std::string());
    stream << "http://ws.stream.qqmusic.qq.com/C100"
           << mediaMid
           << ".m4a?fromtag=0&guid="
           << guid;
    return stream.str();
}

int main()
{

    std::string url = GetMusicFile("003IjD3G3xCgJv");
    std::cout << url << std::endl;
    // Request request;

    // std::string body = request.Touch("cn.bing.com", "/search?q=github%20search%20in%20filename&qs=n&form=QBRE&sp=-1&pq=github%20search%20i%20filename&sc=0-24&sk=&cvid=10BBE06651CD4C31B064731802EFF839", Request::GET);

    // std::cout << body << std::endl;

    return 0;
}