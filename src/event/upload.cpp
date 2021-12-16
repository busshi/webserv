#include "FileUploader.hpp"
#include "core.hpp"

using std::map;

void
handleUploadEvents(void)
{
    for (map<int, FileUploader*>::const_iterator cit = uploaders.begin();
         cit != uploaders.end();
         ++cit) {
        FileUploader* uploader = cit->second;

        // blank parse: just tell the parser to continue its work
        uploader->parseFormDataFragment("", 0);
    }
}
