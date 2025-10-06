#ifndef SIMJSDELIVER_MAIN_H
#define SIMJSDELIVER_MAIN_H

#include <spdlog/spdlog.h>
#include <httplib.h>
#include <filesystem>

#define DEFAULT_URL "https://registry.npmjs.org"
#define LISTEN_PORT 8080
#define CACHE_EXPIRE_TIME_IN_SECONDS 1209600 //cache expires in two weeks
#define CACHE_CHECK_DURAION_IN_SECONDS 7200L //check cache library every two hours

struct parsed_response {
    std::string entryfilepath; //index.js path
    std::string parsed_tarballURL; //tarball URL
    std::string requested_filepath; // the requested file path(if has)
    //the path starts with "/"

    bool notfound=false;
    bool filelist=false; //if we need to show file list
    bool specified_file=false;
};

struct parsed_url {
    std::string pkgname;
    std::string pkgver;
    std::string filepath;
};

inline std::string tarball_name;
inline std::string decompressed_dir_name;
inline std::string registryURL;
inline std::string pkgname;
inline std::string pkgver;

extern size_t WriteResponse(char *ptr, size_t size, size_t nmemb, void *userdata);
parsed_response response_parse(const std::string& OrigResponse,std::string target,parsed_url url);
extern void download(const std::string& url,const std::string& filename);
extern int decompress(const char* filename, const char* destination);
extern std::string get_content_type(std::string file_path);
extern std::string filelist(const std::string& dirpath,const std::string& pkg_name);
static int server();
extern bool is_in_cache();
extern void cache_timestamp_upd();
extern void check_cache();
extern void call_cache_clean();
extern void send_and_delete_file(httplib::Response& res, const std::string& filepath, const std::string& content_type);

#endif //SIMJSDELIVER_MAIN_H

