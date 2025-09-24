#include <iostream>
#include<httplib.h>
#include <curl/curl.h>
#include<nlohmann/json.hpp>
#include <regex>


#define DEFAULT_URL "https://registry.npmjs.org"


std::string registryURL;

struct phrased_response {
    std::string entryfilepath; //index.js path
    std::string phrased_URL; //tarball URL
    std::string fileview_result;
    bool notfound=false;
    bool filelist=false;
};


int server();

size_t WriteResponse(char *ptr, size_t size, size_t nmemb, void *userdata);

phrased_response response_phrase(std::string OrigResponse,std::string origurl);

std::string fileviewgen(std::string pkgver);

extern void download(std::string url);


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

    svr.Get("/:pkgname", [&](const Request& req, Response &res) {
        std::string path = req.path_params.at("pkgname");
        std::string finalurl=registryURL+"/"+path;

        curl_global_init(CURL_GLOBAL_ALL);
        CURL* handle=curl_easy_init();
        std::string OrigResponse;
        curl_easy_setopt(handle,CURLOPT_URL,finalurl.c_str());
        curl_easy_setopt(handle,CURLOPT_WRITEFUNCTION,WriteResponse);
        curl_easy_setopt(handle,CURLOPT_WRITEDATA,&OrigResponse);

        curl_easy_perform(handle);
        curl_easy_cleanup(handle);

        phrased_response response=response_phrase(OrigResponse,path);;

        if (response.notfound==true) {
            res.status=StatusCode::NotFound_404;
            res.set_content("404 Not Found","text/plain");
        }
        download(response.phrased_URL);
        //Since the tarball have saved to tarball.tgz, now we need to decompress it.
        //TODO:Decompress .tgz
    });

    svr.listen("0.0.0.0", 8080);
    return 0;
}


extern size_t WriteResponse(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    ((std::string*)userdata)->append((char*)ptr, size * nmemb);
    return size * nmemb;
}

phrased_response response_phrase(std::string OrigResponse,std::string origurl) {
    using json=nlohmann::json;
    phrased_response result;
    json orig_data =json::parse(OrigResponse);

    std::regex findpkgver("(?<=@).*");
    std::regex fileview(".*/$");
    std::smatch match;
    std::string pkgver;

    if(orig_data.at("error")=="Not found") {
        result.notfound=true;
    }
    else if (std::regex_search(origurl,match,findpkgver)) {
        pkgver=match.str(0);

    }
    else {
        pkgver=orig_data["dist-tags"]["latest"];

    }


    if (std::regex_search(origurl,match,fileview)) {
        //TODO:gen file view
        //TODO: directly return the file view flag and file view content, do not excute the following
    }


    json pkgjson=orig_data["versions"][pkgver];

    if (pkgjson.contains("jsdelivr")) {
        result.entryfilepath=pkgjson["jsdelivr"];
    }
    else if (pkgjson.contains("exports") && pkgjson["exports"].contains(".")) {
        json exports=pkgjson["exports"];
        if (exports.contains("default")) {
            result.entryfilepath=exports["default"];
        }
        else {
            result.entryfilepath=exports["."];
        }

    }
    else if (pkgjson.contains("main")) {
        result.entryfilepath=pkgjson["main"];
    }
    else {
        result.notfound=true;
    }

    result.phrased_URL=pkgjson["dist"].at("tarball");

    return result;
}

