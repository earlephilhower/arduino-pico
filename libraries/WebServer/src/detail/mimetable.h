#pragma once

#include <api/String.h>

namespace mime {

enum type {
    html,
    htm,
    css,
    txt,
    js,
    json,
    png,
    gif,
    jpg,
    ico,
    svg,
    ttf,
    otf,
    woff,
    woff2,
    eot,
    sfnt,
    xml,
    pdf,
    zip,
    gz,
    appcache,
    none,
    maxType
};

struct Entry {
    const char *endsWith;
    const char *mimeType;
};

extern const Entry mimeTable[maxType];

arduino::String getContentType(const arduino::String& path);

}
