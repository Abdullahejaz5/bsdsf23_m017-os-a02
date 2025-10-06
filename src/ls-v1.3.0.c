/*
 ============================================================================
 Name        : ls-v1.3.0.c
 Author      : Abdullah Ejaz
 Description : Feature-4 - Alphabetical Sorting for all modes
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

int compare_names(const void *a, const void *b) {
    const char **pa = (const char **)a;
    const char **pb = (const char **)b;
    return strcasecmp(*pa, *pb); // case-insensitive alphabetical
}

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
        print_column_listing(path, 3);
    else
        print_simple_listing(path);

    return 0;
}

// ----------------------------------------------------------------------
// Helper: read & sort all filenames
// ----------------------------------------------------------------------
int read_sorted_files(const char *path, char *names[], int max) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return 0;
    }

    struct dirent *entry;
    int count = 0;

    while ((entry = readdir(dir)) != NULL && count < max) {
        if (entry->d_name[0] == '.')
            continue;
        names[count] = strdup(entry->d_name);
        count++;
    }
    closedir(dir);

    qsort(names, count, sizeof(char *), compare_names); // sort A–Z
    return count;
}

// ----------------------------------------------------------------------
// Simple listing (sorted)
// ----------------------------------------------------------------------
void print_simple_listing(const char *path) {
    char *names[MAX_FILES];
    int count = read_sorted_files(path, names, MAX_FILES);

    for (int i = 0; i < count; i++) {
        printf("%s\n", names[i]);
        free(names[i]);
    }
}

// ----------------------------------------------------------------------
// Long listing (sorted)
// ----------------------------------------------------------------------
void print_long_listing(const char *path) {
    char *names[MAX_FILES];
    int count = read_sorted_files(path, names, MAX_FILES);
    struct stat st;
    char fullpath[1024];

    for (int i = 0; i < count; i++) {
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, names[i]);
        if (lstat(fullpath, &st) == -1) {
            fprintf(stderr, "Error accessing %s: %s\n", fullpath, strerror(errno));
            free(names[i]);
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

        // Time
        char timebuf[64];
        struct tm *tm_info = localtime(&st.st_mtime);
        strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", tm_info);
        printf(" %s %s\n", timebuf, names[i]);

        free(names[i]);
    }
}

// ----------------------------------------------------------------------
// Column display (sorted)
// ----------------------------------------------------------------------
void print_column_listing(const char *path, int cols) {
    char *names[MAX_FILES];
    int count = read_sorted_files(path, names, MAX_FILES);
    if (count == 0) return;

    // Find longest name
    size_t maxlen = 0;
    for (int i = 0; i < count; i++) {
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

    for (int i = 0; i < count; i++)
        free(names[i]);
}
