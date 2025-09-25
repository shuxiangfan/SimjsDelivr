//THE USE OF UNORDERED MAP IS PROVIDED BY OPENAI CHATGPT
#include <fstream>
#include <iostream>
#include <filesystem>
#include <unordered_map>
#include <algorithm>

namespace fs=std::filesystem;
using std::string;
string file_type;

string get_content_type(std::string file_path) {
    static const std::unordered_map<std::string, std::string> mime_map = {
        {".css", "text/css"},
        {".csv", "text/csv"},
        {".txt", "text/plain"},
        {".vtt", "text/vtt"},
        {".html", "text/html"},
        {".htm", "text/html"},
        {".apng", "image/apng"},
        {".avif", "image/avif"},
        {".bmp", "image/bmp"},
        {".gif", "image/gif"},
        {".png", "image/png"},
        {".svg", "image/svg+xml"},
        {".webp", "image/webp"},
        {".ico", "image/x-icon"},
        {".tif", "image/tiff"},
        {".tiff", "image/tiff"},
        {".jpeg", "image/jpeg"},
        {".jpg", "image/jpeg"},
        {".mp4", "video/mp4"},
        {".mpeg", "video/mpeg"},
        {".webm", "video/webm"},
        {".mp3", "audio/mp3"},
        {".mpga", "audio/mpeg"},
        {".weba", "audio/webm"},
        {".wav", "audio/wave"},
        {".otf", "font/otf"},
        {".ttf", "font/ttf"},
        {".woff", "font/woff"},
        {".woff2", "font/woff2"},
        {".7z", "application/x-7z-compressed"},
        {".atom", "application/atom+xml"},
        {".pdf", "application/pdf"},
        {".mjs", "text/javascript"},
        {".js", "text/javascript"},
        {".json", "application/json"},
        {".rss", "application/rss+xml"},
        {".tar", "application/x-tar"},
        {".xhtml", "application/xhtml+xml"},
        {".xht", "application/xhtml+xml"},
        {".xslt", "application/xslt+xml"},
        {".xml", "application/xml"},
        {".gz", "application/gzip"},
        {".zip", "application/zip"},
        {".wasm", "application/wasm"}
    };
    string text_extensions[] = {
        ".css", ".csv", ".txt", ".vtt",
        ".html", ".htm", ".mjs", ".js",
        ".json", ".rss", ".xhtml", ".xht",
        ".xslt", ".xml"
    };

    string ext=fs::path(file_path).extension();

    auto type=mime_map.find(ext);

    if (type!=mime_map.end()) {
        file_type= type->second;
    }
    else {
        file_type= "application/octet-stream";
    }

    if (std::find(text_extensions,text_extensions+14,ext)) {
        file_type += "; charset=utf-8";
    }

    return file_type;
}