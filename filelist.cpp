#include <filesystem>

std::string filelist(const std::string& dirpath,const std::string& pkg_name) {
    std::string html;
    std::string filepath;
    std::string Filename;
    html+="<!DOCTYPE HTML>\n";
    html+="<html>\n";
    html+="<head>\n";
    html+="<meta charset=\"utf-8\">\n";
    html+="<title>";html+=pkg_name;html+="</title>\n";
    html+="</head>\n";
    html+="<body>\n";
    html+="<h1>";html+=pkg_name;html+="</h1>\n";
    html+="<hr>\n";
    html+="<ul>\n";
    for (const auto& entry : std::filesystem::directory_iterator(dirpath)) {
        filepath=entry.path();
        Filename=std::filesystem::relative(filepath,dirpath);
        html+="<li><a href=\""+Filename.substr(0,Filename.size())+"\">"+Filename.substr(0,Filename.size())+"</a></li>\n";
    }
    html+="</ul>\n";
    html+="<hr>\n";
    html+="</body>\n";
    html+="</html>\n\n";

    return html;
}