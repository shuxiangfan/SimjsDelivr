#include "main.h"
#include<httplib.h>
#include <curl/curl.h>
#include <regex>

#define DEFAULT_URL "https://registry.npmjs.org"

std::string tarball_name="tarball.tgz";
std::string decompressed_dir_name="decompressed";
std::string registryURL;

int main() {
    if (std::getenv("REGISTRY") == nullptr) {
        registryURL = DEFAULT_URL;
    }
    else {
        registryURL= std::getenv("REGISTRY");
    }
    server();
    return 0;
}

int server() {
    using namespace httplib;
    Server svr;

    svr.Get(R"(.*)", [&](const Request& req, Response &res) {
        std::string path = req.target;

        if (!path.empty() && path.front() == '/') {
            path.erase(0, 1);
        }

        spdlog::info("\n\nThe requested URL path={}",path);

        std::regex pattern("@.*");
        std::string finalurl=registryURL+"/"+path;
        finalurl=std::regex_replace(finalurl,pattern,"");
        spdlog::info("the request URL={}",finalurl);

        curl_global_init(CURL_GLOBAL_ALL);
        CURL* handle=curl_easy_init();
        std::string OrigResponse;
        curl_easy_setopt(handle,CURLOPT_URL,finalurl.c_str());
        curl_easy_setopt(handle,CURLOPT_WRITEFUNCTION,WriteResponse);
        curl_easy_setopt(handle,CURLOPT_WRITEDATA,&OrigResponse);

        curl_easy_perform(handle);
        curl_easy_cleanup(handle);

        parsed_response response=response_parse(OrigResponse,path);
        std::string tarballURL=response.parsed_tarballURL;

        tarball_name=pkgname+"@"+pkgver+".tgz";
        decompressed_dir_name=pkgname+pkgver+"_decompresed";

        spdlog::info("The tarballURL={}",response.parsed_tarballURL);
        spdlog::info("The entryfilepath={}",response.entryfilepath);
        spdlog::info("The notfound flag={}",response.notfound);


        if (response.notfound==true) {
            res.status=StatusCode::NotFound_404;
            res.set_content("404 Not Found","text/plain");
        }
        else {
            download(tarballURL,tarball_name);
            //now we need to decompress it.
            decompress(tarball_name.c_str(),decompressed_dir_name.c_str());
            //decompressed_dir_name/package/(actual package content)
            if (response.specified_file==false && response.filelist==false) {
                std::string index_file_path=decompressed_dir_name+"/package/"+response.entryfilepath;
                res.set_file_content(index_file_path,get_content_type(index_file_path));
            }
            else if (response.specified_file==true && response.filelist==false) {
                std::string file_path=decompressed_dir_name+"/package"+response.requested_filepath;
                res.set_file_content(file_path,get_content_type(file_path));
            }
            else if (response.specified_file==false && response.filelist==true) {
                std::string new_root=decompressed_dir_name+"/package/";
                std::string listview = filelist(new_root,pkgname);
                res.set_content(listview,"text/html");
            }
        }


    });

    svr.listen("0.0.0.0", 8080);
    return 0;
}
