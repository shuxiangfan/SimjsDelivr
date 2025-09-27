#ifndef SIMJSDELIVER_MAIN_H
#define SIMJSDELIVER_MAIN_H

#include <spdlog/spdlog.h>

#define DEFAULT_URL "https://registry.npmjs.org"
#define CACHE_EXPIRE_TIME_IN_SECONDS 5
#define CACHE_CHECK_DURAION_IN_SECONDS 10L

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
inline std::string tarball_name;
inline std::string decompressed_dir_name;
inline std::string registryURL;

extern size_t WriteResponse(char *ptr, size_t size, size_t nmemb, void *userdata);
extern parsed_response response_parse(const std::string& OrigResponse,std::string origurl);
extern void download(const std::string& url,const std::string& filename);
extern int decompress(const char* filename, const char* destination);
extern std::string get_content_type(std::string file_path);
extern std::string filelist(const std::string& dirpath,const std::string& pkg_name);
static int server();
extern bool is_in_cache();
extern void cache_timestamp_upd();
extern void check_cache();
extern void call_cache_clean();

#endif //SIMJSDELIVER_MAIN_H

