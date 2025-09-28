#include "main.h"
#include<nlohmann/json.hpp>
#include <regex>
#include <spdlog/spdlog.h>

parsed_response response_parse(const std::string& OrigResponse,std::string origurl) {

    parsed_response result;
    //spdlog::info("Entered parse url function!\n In parse: origurl={}",origurl);
    //spdlog::info("In parse: Registry response (first 200 chars): {}", OrigResponse.substr(0, 200));


    using json=nlohmann::json;
    json orig_data;
    try {
        orig_data =json::parse(OrigResponse);
        //spdlog::info("In parse: JSON parsed!");
    }catch (const std::exception &e) {
        spdlog::error("JSON parse failed: {}", e.what());
        result.notfound = true;
        return result;
    }

    if (orig_data.contains("name")) {
        pkgname=orig_data["name"];
        //spdlog::info("In parse: pkgname={}",pkgname);
    }



    std::regex findpkgver("@([^/]+)"); // only take out pkgver
    std::regex filepath("^[^@/]+(?:@[^/]+)?(/.*)$"); //Thank you gpt-5! This removes everything after pkgname@pkgver, leaving only /path/to/file
    std::smatch match;


    if(orig_data.contains("error") && orig_data["error"] == "Not found") {
        result.notfound=true;
        //spdlog::info("In parse: notfound flag=true");
    }
    else if (std::regex_search(origurl, match, findpkgver)) {
        pkgver=match[1];
        //spdlog::info("In parse: pkgver={}",pkgver);


    }
    else {
        //spdlog::info("In parse: try to find latest");
        if (orig_data.contains("dist-tags") && orig_data["dist-tags"].contains("latest")) {
            pkgver = orig_data["dist-tags"]["latest"].get<std::string>();
        } else {
            //spdlog::error("dist-tags.latest not found in registry response");
            result.notfound = true;
            return result;
        }
        //spdlog::info("In parse: pkgver={}",pkgver);

    }

    if (std::regex_match(origurl,match,filepath)) {
        std::string requested_file_path = match[1];
        //spdlog::info("In parse: Found :{} after the pkgname@pkgver",requested_file_path);
        if (origurl.back()=='/') {
            //spdlog::info("In parse: We should return a file list!");
            result.filelist=true;
            //we should return file list
        }
        else {
            //returm the requested file
            result.requested_filepath=requested_file_path;
            //spdlog::info("In parse: requested file path={}",requested_file_path);
            result.specified_file=true;
        }
    }

    json pkgjson;
    if (orig_data.contains("versions") && orig_data["versions"].contains(pkgver)) {
        pkgjson = orig_data["versions"][pkgver];
        //spdlog::info("In parse: The pkgjson is hit");
    } else {
        //spdlog::warn("versions[{}] not present in response", pkgver);
        result.notfound = true;
        return result;
    }
    //spdlog::info("In parse: Now outside the version find");

    //find the entry file path
    if (pkgjson.contains("jsdelivr")) {
        //spdlog::info("In parse: jsdelivr case");
        result.entryfilepath=pkgjson["jsdelivr"];
        //spdlog::info("In parse: The extryfilepath:{}",result.entryfilepath);

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


    return result;
}