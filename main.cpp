#include <iostream>
#include<httplib.h>
#include <curl/curl.h>
#include<nlohmann/json.hpp>
#include <regex>
#include <spdlog/spdlog.h>//TODO: using log library to debug


#define DEFAULT_URL "https://registry.npmjs.org"


std::string tarball_name="tarball.tgz";
std::string decompressed_dir_name="decompressed";
std::string registryURL;
std::string pkgname;
std::string pkgver;

struct parsed_response {
    std::string entryfilepath; //index.js path
    std::string parsed_tarballURL; //tarball URL
    std::string requested_filepath; // the requested file path(if has)
    //the path starts with "/"

    bool notfound=false;
    bool filelist=false; //if we need to show file list
    bool specified_file=false;
};


int server();

size_t WriteResponse(char *ptr, size_t size, size_t nmemb, void *userdata);

parsed_response response_parse(const std::string& OrigResponse,std::string origurl);

extern void download(const std::string& url,const std::string& filename);

extern int decompress(const char* filename, const char* destination);

extern std::string get_content_type(std::string file_path);

extern std::string filelist(std::string dirpath);


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
                std::string root=decompressed_dir_name+"/package/";
                //TODO:Generate file list view like nginx
            }
        }


    });

    svr.listen("0.0.0.0", 8080);
    return 0;
}


extern size_t WriteResponse(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    static_cast<std::string *>(userdata)->append((char*)ptr, size * nmemb);
    return size * nmemb;
}

parsed_response response_parse(const std::string& OrigResponse,std::string origurl) {

    parsed_response result;
    spdlog::info("Entered parse url function!\n In parse: origurl={}",origurl);
    spdlog::info("In parse: Registry response (first 200 chars): {}", OrigResponse.substr(0, 200));


    using json=nlohmann::json;
    json orig_data;
    try {
        orig_data =json::parse(OrigResponse);
        spdlog::info("In parse: JSON parsed!");
    }catch (const std::exception &e) {
        spdlog::error("JSON parse failed: {}", e.what());
        result.notfound = true;
        return result;
    }


    std::regex findpkgver("@([^/]+)");
    std::regex filepath("^[^@]+@[^/]+(/.*)$");
    std::smatch match;


    if(orig_data.contains("error") && orig_data["error"] == "Not found") {
        result.notfound=true;
        spdlog::info("In parse: notfound flag=true");
    }
    else if (std::regex_search(origurl, match, findpkgver)) {
        pkgver=match[1];
        spdlog::info("In parse: pkgver={}",pkgver);


    }
    else {
        spdlog::info("In parse: try to find latest");
        if (orig_data.contains("dist-tags") && orig_data["dist-tags"].contains("latest")) {
            pkgver = orig_data["dist-tags"]["latest"].get<std::string>();
        } else {
            spdlog::error("dist-tags.latest not found in registry response");
            result.notfound = true;
            return result;
        }
        spdlog::info("In parse: pkgver={}",pkgver);

    }


    if (std::regex_match(origurl,match,filepath)) {
        std::string requested_file_path = match[1];
        spdlog::info("In parse: We found :{} after the pkgname@pkgver",requested_file_path);
        if (requested_file_path=="/") {
            spdlog::info("In parse: We should return a file list!");
            result.filelist=true;
            //we should return file list
        }
        else {
            //returm the requested file
            result.requested_filepath=requested_file_path;
            spdlog::info("In parse: requested file path={}",requested_file_path);
            result.specified_file=true;
        }
    }

    json pkgjson;
    if (orig_data.contains("versions") && orig_data["versions"].contains(pkgver)) {
        pkgjson = orig_data["versions"][pkgver];
        spdlog::info("In parse: The pkgjson is hit");
    } else {
        spdlog::error("versions[{}] not present in response", pkgver);
        result.notfound = true;
        return result;
    }
    spdlog::info("In parse: Now outside the version find");

    //find the entry file path
    if (pkgjson.contains("jsdelivr")) {
        spdlog::info("In parse: jsdelivr case");
        result.entryfilepath=pkgjson["jsdelivr"];
        spdlog::info("In parse: The extryfilepath:{}",result.entryfilepath);

    }
    else if (pkgjson.contains("exports") && pkgjson["exports"].contains(".")) {
        json exports=pkgjson["exports"];
        if (exports["."].contains("default")) {
            result.entryfilepath=exports["."]["default"];
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

    if (pkgjson.contains("dist") && pkgjson["dist"].contains("tarball")) {
        //find the tarball file link
        result.parsed_tarballURL=pkgjson["dist"]["tarball"];
    }

   if (orig_data.contains("name")) {
       pkgname=orig_data["name"];
       spdlog::info("In parse: pkgname={}",pkgname);
   }

    return result;
}

