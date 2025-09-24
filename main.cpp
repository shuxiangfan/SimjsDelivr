#include <iostream>
#include<httplib.h>
#include <curl/curl.h>
#include<nlohmann/json.hpp>
#include <regex>


#define DEFAULT_URL "https://registry.npmjs.org"


std::string tarball_name="tarball.tgz";
std::string decompressed_dir_name="decompressed";
std::string registryURL;

struct phrased_response {
    std::string entryfilepath; //index.js path
    std::string phrased_tarballURL; //tarball URL
    std::string requested_filepath; // the requested file path(if has)
    //the path starts with "/"

    bool notfound=false;
    bool filelist=false; //if we need to show file list
    bool specified_file=false;
};


int server();

size_t WriteResponse(char *ptr, size_t size, size_t nmemb, void *userdata);

phrased_response response_phrase(std::string OrigResponse,std::string origurl);

std::string fileviewgen(std::string pkgver);

extern void download(std::string url,std::string filename);

extern int decompress(const char* filename, const char* destination);


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

    svr.Get("/:urlpath", [&](const Request& req, Response &res) {
        std::string path = req.path_params.at("urlpath");
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
        download(response.phrased_tarballURL,tarball_name);
        //now we need to decompress it.
        decompress(tarball_name.c_str(),decompressed_dir_name.c_str());

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

    phrased_response result;
    std::string pkgver;

    using json=nlohmann::json;
    json orig_data =json::parse(OrigResponse);

    std::regex findpkgver("@([^/]+)");
    std::regex filepath("^[^@]+@[^/]+(/.*)$");
    std::smatch match;


    if(orig_data.at("error")=="Not found") {
        result.notfound=true;
    }
    else if (std::regex_match(origurl, match, findpkgver)) {
        pkgver=match[1];

    }
    else {
        pkgver=orig_data["dist-tags"]["latest"];

    }


    if (std::regex_match(origurl,match,filepath)) {
        std::string requested_file_path = match[1];
        if (requested_file_path=="/") {
            result.filelist=true;
            //we should return file list
        }
        else {
            //returm the requested file
            result.requested_filepath=requested_file_path;
            result.specified_file=true;
        }
    }


    json pkgjson=orig_data["versions"][pkgver];

    //find the entry file path
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

    //find the tarball file link
    result.phrased_tarballURL=pkgjson["dist"].at("tarball");

    return result;
}

