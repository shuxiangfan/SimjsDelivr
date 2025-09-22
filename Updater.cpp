#include "Updater.h"
#include <iostream>
#include <string>

#define DEFAULT_METADATA_URL "https://registry.npmjs.org"

int metadata_update() {
    std::string META_URL;
    if (std::getenv("REGISTRY") == nullptr) {
        META_URL = DEFAULT_METADATA_URL;
    }
    else {
        META_URL= std::getenv("REGISTRY");
    }
    return 0;
}
