#include <Arduino.h>
#include "mimetable.h"
#include "pgmspace.h"

namespace mime {

// Table of extension->MIME strings stored in PROGMEM, needs to be global due to GCC section typing rules
const Entry mimeTable[maxType] = {
    { ".html", "text/html" },
    { ".htm", "text/html" },
    { ".css", "text/css" },
    { ".txt", "text/plain" },
    { ".js", "application/javascript" },
    { ".json", "application/json" },
    { ".png", "image/png" },
    { ".gif", "image/gif" },
    { ".jpg", "image/jpeg" },
    { ".ico", "image/x-icon" },
    { ".svg", "image/svg+xml" },
    { ".ttf", "application/x-font-ttf" },
    { ".otf", "application/x-font-opentype" },
    { ".woff", "application/font-woff" },
    { ".woff2", "application/font-woff2" },
    { ".eot", "application/vnd.ms-fontobject" },
    { ".sfnt", "application/font-sfnt" },
    { ".xml", "text/xml" },
    { ".pdf", "application/pdf" },
    { ".zip", "application/zip" },
    { ".gz", "application/x-gzip" },
    { ".appcache", "text/cache-manifest" },
    { "", "application/octet-stream" }
};


arduino::String getContentType(const arduino::String& path) {
    for (size_t i = 0; i < maxType; i++) {
        if (path.endsWith(FPSTR(mimeTable[i].endsWith))) {
            return arduino::String(FPSTR(mimeTable[i].mimeType));
        }
    }
    // Fall-through and just return default type
    return arduino::String(FPSTR(mimeTable[none].mimeType));
}
}
