#include <unistd.h>
#include <fontconfig/fontconfig.h>
#include <sys/inotify.h>
#include <curl/curl.h>

#include "utils.hpp"

bool download_file(const char* url, const std::filesystem::path& path) {

    CURL* curl = curl_easy_init();
    if (curl == nullptr)
        return false;

    FILE* file = fopen(path.c_str(), "wb");
    if (file == nullptr)
        return false;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, nullptr);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

    if (curl_easy_perform(curl) != CURLE_OK)
        return false;

    fclose(file);
    curl_easy_cleanup(curl);

    return true;
}

bool watch_file(const std::filesystem::path& path, std::function<void()> fn) {

    // vim will only reliably produce IN_MOVE_SELF events, so we have to catch those
    auto flags = IN_MODIFY | IN_CLOSE_WRITE | IN_MOVE_SELF;

    int fd = inotify_init();
    if (fd == -1)
        return false;

    int wd = inotify_add_watch(fd, path.c_str(), flags);
    if (wd == -1)
        return false;

    while (true) {
        struct inotify_event event;
        ssize_t bytes_read = read(fd, &event, sizeof event);
        if (bytes_read == 0 || bytes_read == -1)
            return false;

        // vim will actually swap the edited file with a new file, so
        // we have to catch that and add the file back to the watchlist
        if (event.mask & IN_IGNORED) {
            while (true) {
                wd = inotify_add_watch(fd, path.c_str(), flags);
                if (wd != -1) break;
                if (errno != ENOENT)
                    return false;
            }
        }

        fn();
    }

    if (inotify_rm_watch(fd, wd) == -1)
        return false;

    if (close(fd) == -1)
        return false;

    return true;
}

auto parse_font_name(const char* font_name) -> std::optional<std::filesystem::path> {

    FcInit();
    FcConfig* conf = FcInitLoadConfigAndFonts();
    FcPattern* pattern = FcNameParse(reinterpret_cast<const FcChar8*>(font_name));
    if (pattern == nullptr) return {};

    FcConfigSubstitute(conf, pattern, FcMatchPattern);
    FcDefaultSubstitute(pattern);

    FcResult result;
   	FcPattern* font = FcFontMatch(conf, pattern, &result);
    if (font == nullptr) return {};

    FcChar8* file = nullptr;
    if (FcPatternGetString(font, FC_FILE, 0, &file) != FcResultMatch)
        return {};

    std::filesystem::path path = reinterpret_cast<const char*>(file);

    FcPatternDestroy(font);
    FcPatternDestroy(pattern);
    FcConfigDestroy(conf);
    FcFini();
    return path;
}