#include <iostream>
#include <fstream>
#include <curl/curl.h>
#include <cstdlib>
#include "main.h"


void download(const std::string& url,const std::string& filename) {
    FILE*fp=fopen(filename.c_str(),"wb");
    if (!fp) {std::cout<<"File open error";exit(EXIT_FAILURE);}  //im lazy to deal with all the s**t. just once

    curl_global_init(CURL_GLOBAL_ALL);
    CURL* handle=curl_easy_init();
    curl_easy_setopt(handle,CURLOPT_URL,url.c_str());
    curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(handle,CURLOPT_WRITEDATA,fp);

    curl_easy_perform(handle);

    fclose(fp);
    curl_easy_cleanup(handle);
}

size_t WriteResponse(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    static_cast<std::string *>(userdata)->append((char*)ptr, size * nmemb);
    return size * nmemb;
}

void send_and_delete_file(httplib::Response& res, const std::string& filepath, const std::string& content_type) { //Again thank you gpt5!
    std::ifstream ifs(filepath, std::ios::binary);
    if (!ifs) {
        res.status=httplib::StatusCode::NotFound_404;
        res.set_content("404 Not Found","text/plain");
        return;
    }

    // Get file size
    ifs.seekg(0, std::ios::end);
    auto size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    // Copy file into buffer (or stream chunks if large)
    std::string buffer(size, '\0');
    ifs.read(buffer.data(), size);

    // Provide the content
    res.set_content_provider(
        size, content_type,
        [buffer = std::move(buffer)](size_t offset, size_t length, const httplib::DataSink &sink) {
            sink.write(buffer.data() + offset, length);
            return true;
        },
        [filepath](bool) {
            // Cleanup callback after response is finished
            std::error_code ec;
            std::filesystem::remove_all(decompressed_dir_name, ec); // modified: always delete the entire directory and use remove_all() to purge the entire dir
            if (ec) {
                spdlog::error("Failed to delete decompressed dir:{}, reason:{}",tarball_name,ec.message());//modified: do not use cout, use spdlog instead
            }
            else {
                spdlog::info("Decompressed tarball dir:{} purged!",decompressed_dir_name);
            }
        }
    );
}
