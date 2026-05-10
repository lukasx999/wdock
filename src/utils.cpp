#include <unistd.h>
#include <fontconfig/fontconfig.h>
#include <sys/inotify.h>
#include <curl/curl.h>
#include <poll.h>

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

bool watch_file_async(const std::filesystem::path& path, std::function<void()> fn, std::function<bool()> stop_fn) {

    // vim will only reliably produce IN_MOVE_SELF events, so we have to catch those
    auto flags = IN_MODIFY | IN_CLOSE_WRITE | IN_MOVE_SELF;

    int fd = inotify_init1(IN_NONBLOCK);
    if (fd == -1)
        return false;

    int wd = inotify_add_watch(fd, path.c_str(), flags);
    if (wd == -1)
        return false;

    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;

    while (!stop_fn()) {
        int num_fds = poll(&pfd, 1, 0);
        assert(num_fds != -1);

        if (num_fds == 1) {
            if (!(pfd.revents & POLLIN)) continue;

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

    }

    if (inotify_rm_watch(fd, wd) == -1)
        return false;

    if (close(fd) == -1)
        return false;

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

consteval void test_parse_color_string() {

    constexpr auto a = parse_color_string("#00000000");
    static_assert(a.has_value());
    static_assert(a->x == 0);
    static_assert(a->y == 0);
    static_assert(a->z == 0);
    static_assert(a->w == 0);

    constexpr auto b = parse_color_string("#ffffffff");
    static_assert(b.has_value());
    static_assert(b->x == 1);
    static_assert(b->y == 1);
    static_assert(b->z == 1);
    static_assert(b->w == 1);

    constexpr auto c = parse_color_string("#ffffff");
    static_assert(c.has_value());
    static_assert(c->x == 1);
    static_assert(c->y == 1);
    static_assert(c->z == 1);
    static_assert(c->w == 1);

    constexpr auto d = parse_color_string("transparent");
    static_assert(d.has_value());
    static_assert(d->x == 0);
    static_assert(d->y == 0);
    static_assert(d->z == 0);
    static_assert(d->w == 0);

    constexpr auto e = parse_color_string("red");
    static_assert(e.has_value());
    static_assert(e->x == 1);
    static_assert(e->y == 0);
    static_assert(e->z == 0);
    static_assert(e->w == 1);

    constexpr auto f = parse_color_string("white");
    static_assert(f.has_value());
    static_assert(f->x == 1);
    static_assert(f->y == 1);
    static_assert(f->z == 1);
    static_assert(f->w == 1);

}

consteval void test_string_switch() {

    static_assert(string_switch<int>("bar")
        .match("foo", 1)
        .match("bar", 2)
        .match("baz", 3)
        .done() == 2);

    static_assert(string_switch<int>("qux")
        .match("foo", 1)
        .match("bar", 2)
        .match("baz", 3)
        .catchall(45)
        .done() == 45);

}