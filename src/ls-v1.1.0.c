/*
 ============================================================================
 Name        : ls-v1.1.0.c
 Author      : Abdullah Ejaz
 Description : Feature-2: Add long listing format (-l)
 ============================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

void print_simple_listing(const char *path);
void print_long_listing(const char *path);

int main(int argc, char *argv[]) {
    int long_listing = 0;
    char *path = ".";

    // Detect -l option
    if (argc > 1) {
        if (strcmp(argv[1], "-l") == 0) {
            long_listing = 1;
            if (argc > 2) path = argv[2];
        } else {
            path = argv[1];
        }
    }

    if (long_listing)
        print_long_listing(path);
    else
        print_simple_listing(path);

    return 0;
}

void print_simple_listing(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.')
            continue;
        printf("%s  ", entry->d_name);
    }
    printf("\n");
    closedir(dir);
}

void print_long_listing(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    struct stat st;
    char fullpath[1024];

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.')
            continue;

        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        if (lstat(fullpath, &st) == -1) {
            fprintf(stderr, "Error accessing %s: %s\n", fullpath, strerror(errno));
            continue;
        }

        // Type & permissions
        printf( (S_ISDIR(st.st_mode)) ? "d" : "-");
        printf( (st.st_mode & S_IRUSR) ? "r" : "-");
        printf( (st.st_mode & S_IWUSR) ? "w" : "-");
        printf( (st.st_mode & S_IXUSR) ? "x" : "-");
        printf( (st.st_mode & S_IRGRP) ? "r" : "-");
        printf( (st.st_mode & S_IWGRP) ? "w" : "-");
        printf( (st.st_mode & S_IXGRP) ? "x" : "-");
        printf( (st.st_mode & S_IROTH) ? "r" : "-");
        printf( (st.st_mode & S_IWOTH) ? "w" : "-");
        printf( (st.st_mode & S_IXOTH) ? "x" : "-");

        // Link count
        printf(" %2ld", (long)st.st_nlink);

        // Owner and group
        struct passwd *pw = getpwuid(st.st_uid);
        struct group  *gr = getgrgid(st.st_gid);
        printf(" %-8s %-8s", 
               pw ? pw->pw_name : "unknown", 
               gr ? gr->gr_name : "unknown");

        // Size
        printf(" %8ld", (long)st.st_size);

        // Modification time
        char timebuf[64];
        struct tm *tm_info = localtime(&st.st_mtime);
        strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", tm_info);
        printf(" %s", timebuf);

        // Name
        printf(" %s\n", entry->d_name);
    }

    closedir(dir);
}

