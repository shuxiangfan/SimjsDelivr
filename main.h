#ifndef SIMJSDELIVER_MAIN_H
#define SIMJSDELIVER_MAIN_H

#include <spdlog/spdlog.h>

struct parsed_response {
    std::string entryfilepath; //index.js path
    std::string parsed_tarballURL; //tarball URL
    std::string requested_filepath; // the requested file path(if has)
    //the path starts with "/"

    bool notfound=false;
    bool filelist=false; //if we need to show file list
    bool specified_file=false;
};

inline std::string pkgname;
inline std::string pkgver;

extern size_t WriteResponse(char *ptr, size_t size, size_t nmemb, void *userdata);
parsed_response response_parse(const std::string& OrigResponse,std::string origurl);
extern void download(const std::string& url,const std::string& filename);
extern int decompress(const char* filename, const char* destination);
extern std::string get_content_type(std::string file_path);
extern std::string filelist(const std::string& dirpath,const std::string& pkg_name);
static int server();

#endif //SIMJSDELIVER_MAIN_H

