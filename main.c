#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "logic.h"

#define STRING_BUFFER_SIZE PATH_MAX * 2

#define STRLEN(s) (sizeof(s)/sizeof(s[0]))

void print_help_and_exit() {
    puts("Usage: %s command options");
    puts("Commands:");
    puts("  vacuum - remove duplicated files in upperdir where copy_up is done but the file is not actually modified");
    puts("  diff   - show the list of actually changed files");
    puts("  merge  - merge all changes from upperdir to lowerdir, and clear upperdir");
    puts("Options:");
    puts("  -l, --lowerdir=LOWERDIR    the lowerdir of OverlayFS (required)");
    puts("  -u, --upperdir=UPPERDIR    the upperdir of OverlayFS (required)");
    puts("  -v, --verbose              when a directory only exists in the newer version, still list every file of the directory");
    puts("Warning:");
    puts("  Only works for regular files, symbolic links and directories. Do not use it on OverlayFS with device files, socket files, etc..");
    puts("  Hard links may be broken (i.e. resulting in duplicated independent files).");
    puts("  This program only works for OverlayFS with only one lower layer.");
    puts("  It is recommended to have the OverlayFS unmounted before running this program.");
    exit(EXIT_SUCCESS);
}

bool starts_with(const char *haystack, const char* needle) {
    return strncmp(needle, haystack, strlen(needle)) == 0;
}

bool is_mounted(const char *lower, const char *upper) {
    FILE* f = fopen("/proc/mounts", "r");
    if (!f) {
        fprintf(stderr, "Cannot read /proc/mounts to test whether OverlayFS is mounted.\n");
        return true;
    }
    char buf[STRING_BUFFER_SIZE];
    while (fgets(buf, STRING_BUFFER_SIZE, f)) {
        if (!starts_with(buf, "overlay")) {
            continue;
        }
        if (strlen(buf) == STRING_BUFFER_SIZE) {
            fprintf(stderr, "OverlayFS line in /proc/mounts too long.\n");
            return true;
        }
        char* m_lower = &(strstr(buf, "lowerdir=")[STRLEN("lowerdir=")]);
        char* m_upper = &(strstr(buf, "upperdir=")[STRLEN("upperdir=")]);
        if (!(strncmp(lower, m_lower, strlen(lower)) && strncmp(upper, m_upper, strlen(upper)))) {
            printf("The OverlayFS involved is still mounted.\n");
            return true;
        }
    }
    return false;
}

bool directory_exists(const char *path) {
    struct stat sb;
    if (stat(path, &sb) != 0) { return false; }
    return (sb.st_mode & S_IFMT) == S_IFDIR;
}

int main(int argc, char *argv[]) {

    char lower[PATH_MAX] = "";
    char upper[PATH_MAX] = "";
    bool verbose = false;

    static struct option long_options[] = {
        { "lowerdir", required_argument, 0, 'l' },
        { "upperdir", required_argument, 0, 'u' },
        { "help",     no_argument      , 0, 'h' },
        { "verbose",  no_argument      , 0, 'v' },
        { 0,          0,                 0,  0  }
    };

    int opt = 0;

    int long_index = 0;
    while ((opt = getopt_long_only(argc, argv, "", long_options, &long_index)) != -1) {
        switch (opt) {
            case 'l' :
                realpath(optarg, lower);
                break;
            case 'u' :
                realpath(optarg, upper);
                break;
            case 'h':
                print_help_and_exit();
                break;
            case 'v':
                verbose = true;
                break;
            default:
                printf("Option %c is not supported. Use \n", opt);
                printf("Try '%s --help' for more information.\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (lower[0] == '\0') {
        fprintf(stderr, "Lower directory not specified.\n");
        exit(EXIT_FAILURE);
    }
    if (!directory_exists(lower)) {
        fprintf(stderr, "Lower directory cannot be opened.\n");
        exit(EXIT_FAILURE);
    }
    if (upper[0] == '\0') {
        fprintf(stderr, "Upper directory not specified.\n");
        exit(EXIT_FAILURE);
    }
    if (!directory_exists(upper)) {
        fprintf(stderr, "Lower directory cannot be opened.\n");
        exit(EXIT_FAILURE);
    }

    if (is_mounted(lower, upper)) {
        printf("It is strongly recommended to unmount OverlayFS first. Still continue (not recommended)?: \n");
        int r = getchar();
        if (r != 'Y' && r != 'y') {
            exit(EXIT_FAILURE);
        }
    }

    if (optind == argc - 1) {
        set_globals(lower, upper, verbose ? 2 : 1);
        if (strcmp(argv[optind], "vacuum") == 0) {

        } else if (strcmp(argv[optind], "diff") == 0) {

        } else if (strcmp(argv[optind], "merge") == 0) {

        }
    } else {
        puts("Please specify one action.");
        printf("Try '%s --help' for more information.\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;

}