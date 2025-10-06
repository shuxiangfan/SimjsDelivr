#include "main.h"
#include <thread>
#include <curl/curl.h>
#include <regex>
#include <boost/url.hpp>

int main() {

    if (std::getenv("REGISTRY") == nullptr) {
        registryURL = DEFAULT_URL;
    }
    else {
        spdlog::info("main: Using user set REGISTRY={}",std::getenv("REGISTRY"));
        registryURL= std::getenv("REGISTRY");
    }

    std::filesystem::create_directory((std::filesystem::path)"cache");
    std::thread thr(call_cache_clean);
    thr.detach();
    server();

    return 0;
}

int server() {
    using namespace httplib;
    Server svr;

    svr.Get(R"(.*)", [&](const Request& req, Response &res) {
        spdlog::info("-------------cut here-------------");

        parsed_url url;
        std::string urlpath=req.target;
        urlpath=urlpath.erase(0,1);

        boost::core::string_view sv(urlpath);
        boost::urls::decode_view decoded_view(sv);
        std::string decoded_url(decoded_view.begin(), decoded_view.end());
        urlpath=decoded_url;

        spdlog::info("urlpath={}",urlpath);
        std::regex findver(R"((?:[~^><=]=?\s*)?\d+(?:\.\d+)*(?:-[a-zA-Z0-9.-]+)?)");
        std::regex findfilepath(R"(/(.+)$)");
        std::smatch m;
        if (std::regex_search(urlpath,m,findver)) {
            url.pkgver=m[0];
        }
        if (std::regex_search(urlpath,m,findfilepath)) {
            url.filepath=m[1];
        }
        std::regex cut("([@/].*)"); //This removes all contents after @xxxx, leaving only pkgname
        //Maybe we can use path_for_get_response for pkgname!
        std::string path_for_get_response=std::regex_replace(urlpath,cut,"");
        std::string finalurl=registryURL+"/"+path_for_get_response;
        spdlog::info("the request URL={}",finalurl);

        curl_global_init(CURL_GLOBAL_ALL);
        CURL* handle=curl_easy_init();
        std::string OrigResponse;
        curl_easy_setopt(handle,CURLOPT_URL,finalurl.c_str());
        curl_easy_setopt(handle,CURLOPT_WRITEFUNCTION,WriteResponse);
        curl_easy_setopt(handle,CURLOPT_WRITEDATA,&OrigResponse);

        curl_easy_perform(handle);
        curl_easy_cleanup(handle);

        parsed_response response=response_parse(OrigResponse,urlpath,url);
        std::string tarballURL=response.parsed_tarballURL;

        tarball_name="cache/"+pkgname+"@"+pkgver+".tgz";
        decompressed_dir_name="cache/"+pkgname+pkgver+"_decompresed";

        //spdlog::info("The tarballURL={}",response.parsed_tarballURL);
        //spdlog::info("The entryfilepath={}",response.entryfilepath);
        //spdlog::info("The notfound flag={}",response.notfound);


        if (response.notfound==true) {
            res.status=StatusCode::NotFound_404;
            res.set_content("404 Not Found","text/plain");
        }
        else {
            if (!is_in_cache()) {
                download(tarballURL,tarball_name);
                }
            else {
                cache_timestamp_upd();
            }
            //now we need to decompress it.
            decompress(tarball_name.c_str(),decompressed_dir_name.c_str());
            //cache/decompressed_dir_name/package/(actual package content)

            if (response.specified_file==false && response.filelist==false) {
                std::string index_file_path=decompressed_dir_name+"/package/"+response.entryfilepath;
                //res.set_file_content(index_file_path,get_content_type(index_file_path));
                send_and_delete_file(res,index_file_path,get_content_type(index_file_path));
            }
            else if (response.specified_file==true && response.filelist==false) {
                std::string file_path=decompressed_dir_name+"/package/"+response.requested_filepath;
                //res.set_file_content(file_path,get_content_type(file_path));
                send_and_delete_file(res,file_path,get_content_type(file_path));
            }
            else if (response.specified_file==false && response.filelist==true) {
                std::string new_root=decompressed_dir_name+"/package/";
                std::string listview = filelist(new_root,pkgname);
                res.set_content(listview,"text/html");
                std::filesystem::remove_all(decompressed_dir_name);
            }
        }
    });
    svr.listen("0.0.0.0", LISTEN_PORT);
    return 0;
}
