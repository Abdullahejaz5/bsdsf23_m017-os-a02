/*
 ============================================================================
 Name        : ls-v1.2.0.c
 Author      : Abdullah Ejaz
 Description : Feature-3 - Column Display (down then across)
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
#include <errno.h>

#define MAX_FILES 1024
#define MAX_NAME_LEN 256

void print_simple_listing(const char *path);
void print_long_listing(const char *path);
void print_column_listing(const char *path, int cols);

int main(int argc, char *argv[]) {
    int long_listing = 0;
    int column_display = 0;
    const char *path = ".";

    // ----- argument parsing -----
    if (argc == 2) {
        if (strcmp(argv[1], "-l") == 0)
            long_listing = 1;
        else if (strcmp(argv[1], "-C") == 0)
            column_display = 1;
        else
            path = argv[1];
    } 
    else if (argc == 3) {
        if (strcmp(argv[1], "-l") == 0)
            long_listing = 1, path = argv[2];
        else if (strcmp(argv[1], "-C") == 0)
            column_display = 1, path = argv[2];
    }

    // ----- choose mode -----
    if (long_listing)
        print_long_listing(path);
    else if (column_display)
        print_column_listing(path, 3); // default 3 columns
    else
        print_simple_listing(path);

    return 0;
}

// ----------------------------------------------------------------------
// Simple listing (one file per line)
// ----------------------------------------------------------------------
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
        printf("%s\n", entry->d_name);
    }
    closedir(dir);
}

// ----------------------------------------------------------------------
// Long listing (like ls -l)
// ----------------------------------------------------------------------
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

        // Permissions
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

        // Links, owner, group, size
        printf(" %2ld", (long)st.st_nlink);
        struct passwd *pw = getpwuid(st.st_uid);
        struct group  *gr = getgrgid(st.st_gid);
        printf(" %-8s %-8s", pw ? pw->pw_name : "unknown", gr ? gr->gr_name : "unknown");
        printf(" %8ld", (long)st.st_size);

        // Modification time
        char timebuf[64];
        struct tm *tm_info = localtime(&st.st_mtime);
        strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", tm_info);
        printf(" %s", timebuf);

        printf(" %s\n", entry->d_name);
    }
    closedir(dir);
}

// ----------------------------------------------------------------------
// Column display (down then across)
// ----------------------------------------------------------------------
void print_column_listing(const char *path, int cols) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    char *names[MAX_FILES];
    int count = 0, i;

    // Read filenames
    while ((entry = readdir(dir)) != NULL && count < MAX_FILES) {
        if (entry->d_name[0] == '.')
            continue;
        names[count] = strdup(entry->d_name);
        count++;
    }
    closedir(dir);

    if (count == 0) return;

    // Find longest filename
    size_t maxlen = 0;
    for (i = 0; i < count; i++) {
        size_t len = strlen(names[i]);
        if (len > maxlen) maxlen = len;
    }

    int rows = (count + cols - 1) / cols;
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int idx = c * rows + r;
            if (idx < count)
                printf("%-*s  ", (int)maxlen, names[idx]);
        }
        printf("\n");
    }

    for (i = 0; i < count; i++)
        free(names[i]);
}
