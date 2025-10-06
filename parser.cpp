#include "main.h"
#include<nlohmann/json.hpp>
#include "semver.hpp"
#include <algorithm>
#include <vector>

static std::string find_version(const parsed_url &url,nlohmann::json json);

parsed_response response_parse(const std::string& OrigResponse,std::string target,parsed_url url) {

    parsed_response result;
    //spdlog::info("Entered parse url function!\n In parse: origurl={}",target);
    spdlog::info("In parse: Registry response (first 200 chars): {}", OrigResponse.substr(0, 200));


    using json=nlohmann::json;
    json orig_data;
    try {
        orig_data =json::parse(OrigResponse);
        //spdlog::info("In parse: JSON parsed!");
    }catch (const std::exception &e) {
        //spdlog::error("JSON parse failed: {}", e.what());
        result.notfound = true;
        return result;
    }

    if (orig_data.contains("name")) {
        url.pkgname=orig_data["name"];
        pkgname=orig_data["name"];
        //spdlog::info("In parse: pkgname={}",url.pkgname);
    }

    if(orig_data.contains("error") && orig_data["error"] == "Not found") {
        result.notfound=true;
        //spdlog::info("In parse: notfound flag=true");
    }

    else if (!url.pkgver.empty()) {
        url.pkgver=find_version(url,orig_data);
        }
    else {
            spdlog::info("In parse: try to find latest");
            if (orig_data.contains("dist-tags") && orig_data["dist-tags"].contains("latest")) {
                url.pkgver = orig_data["dist-tags"]["latest"].get<std::string>();
            }
            else {
                spdlog::error("dist-tags.latest not found in registry response");
                result.notfound = true;
                return result;
                }
        }
        pkgver=url.pkgver;
        spdlog::info("In parse: pkgver={}",url.pkgver);

    if (target.back()=='/') {
        //spdlog::info("In parse: We should return a file list!");
        result.filelist=true;
        //we should return file list
    }
    else if (!url.filepath.empty()) {
        //returm the requested file
        result.requested_filepath=url.filepath;
        spdlog::info("In parse: requested file path={}",url.filepath);
        result.specified_file=true;
    }

    json pkgjson;
    if (orig_data.contains("versions") && orig_data["versions"].contains(url.pkgver)) {
        pkgjson = orig_data["versions"][url.pkgver];
        //spdlog::info("In parse: The pkgjson is hit");
    } else {
        //spdlog::warn("versions[{}] not present in response", url.pkgver);
        result.notfound = true;
        return result;
    }
    //spdlog::info("In parse: Now outside the version find");

    //find the entry file path
    if (pkgjson.contains("jsdelivr")) {
       // spdlog::info("In parse: jsdelivr case");
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
std::string find_version(const parsed_url &url,nlohmann::json json) {
    if (!semver::valid(url.pkgver)) {
        spdlog::error("Version {} is not valid!",url.pkgver);
        return "Version is not valid!";
    }
    if (!json.contains("versions")) {
        spdlog::error("json is not valid!");
        return "json is not valid!";
    }
    nlohmann::json json_versions =json["versions"];
    std::vector<std::string> all_version_str;

    for (auto& [key, _] : json_versions.items()) {
        if (semver::valid(key)) {
            all_version_str.push_back(key);
        }
        else{
            //spdlog::warn("Skipping invalid semver: {}", key);
        }
    }
    if (all_version_str.empty()) {
        spdlog::error("No valid version found.");
        return "No valid version found.";
    }

    semver::range_set range;
    semver::parse(url.pkgver,range);
    semver::version version;
    std::vector<std::string> matched;

    for (auto& v : all_version_str) {
        semver::parse(v,version);
        if (range.contains(version)) {
            matched.push_back(version.to_string());
        }
    }
    if (matched.empty()) {
        spdlog::error("No matched version found.");
        return "No matched version found.";
    }
    auto latest_matched = std::ranges::max_element(matched,
                                                   [](const std::string& a, const std::string& b) {
                                                       semver::version Ver1;
                                                       semver::version Ver2;
                                                       semver::parse(a,Ver1);
                                                       semver::parse(b,Ver2);
                                                       return Ver1 < Ver2;
                                                   }
    );
    return *latest_matched;
}
