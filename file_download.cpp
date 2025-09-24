#include <iostream>
#include <fstream>
#include <curl/curl.h>
#include <stdlib.h>

void downloader(std::string url,std::string filename) {
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