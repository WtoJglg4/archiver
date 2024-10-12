#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include "archiver.c"

error extractArchive(FILE *archive, const char *outputPath);
error createDirectoryIfNotExists(const char *directory, long filesize);
error extractFile(FILE *archive, const char *filepath, long filesize);
bool isArchive(const char *path);
void removeFileExtension(char *filename);

error RunUnarchiver(Config cfg){
    FILE *archive = fopen(cfg.inputPath, "rb");
    if (!archive) {
        fprintf(stderr, "failed to open archive file: %s\n", cfg.inputPath);
        return EXIT_FAILURE;
    }
    return extractArchive(archive, cfg.outputPath);
}

error extractArchive(FILE *archive, const char *outputPath) {
    char filepath[MAX_PATH];
    long filesize;
    // Reading archive`s content line-by-line
    while (fscanf(archive, "%s\n%ld\n", filepath, &filesize) == 2) {
        char fullPath[MAX_PATH];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", outputPath, filepath);
        // Create directory for each path, even its an empty directory
        error err = createDirectoryIfNotExists(fullPath, filesize);
        if (err != EXIT_SUCCESS) {
            return err;
        }
        if (filesize >= 0) {
            // This is file, re-create it
            err = extractFile(archive, fullPath, filesize);
            if (err != EXIT_SUCCESS) {
                return err;
            }
        }
    }
    return EXIT_SUCCESS;
}

error createDirectoryIfNotExists(const char *path, long filesize) {
    struct stat statbuf;
    char tempPath[MAX_PATH];
    strcpy(tempPath, path);
    // Iterate by each path`s part, creating directories if does not exists
    for (char *p = tempPath + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';  // Temporary setting line`s end
            if (stat(tempPath, &statbuf) != 0) {
                error err = mkdir(tempPath, 0755);
                if (err != EXIT_SUCCESS){
                    fprintf(stderr, "failed to create directory: %s\n", tempPath);
                    return EXIT_FAILURE;
                }
            } else if (!S_ISDIR(statbuf.st_mode)) {
                fprintf(stderr, "path exists but is not a directory: %s\n", tempPath);
                return EXIT_FAILURE;
            }
            *p = '/';  // Recovering the line
        }
    }
    if ((stat(tempPath, &statbuf) != 0) && filesize == -1){
        if (mkdir(tempPath, 0755) != 0 && errno != EEXIST) {
            fprintf(stderr, "failed to create directory: %s\n", tempPath);
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

error extractFile(FILE *archive, const char *filepath, long filesize) {
    FILE *file = fopen(filepath, "wb");
    if (!file) {
        fprintf(stderr, "failed to create file: %s\n", filepath);
        return EXIT_FAILURE;
    }
    // Reading file contents
    char *buffer = malloc(filesize);
    if (!buffer) {
        fprintf(stderr, "failed to allocate memory for file: %s\n", filepath);
        fclose(file);
        return EXIT_FAILURE;
    }
    fread(buffer, 1, filesize, archive);
    fwrite(buffer, 1, filesize, file);
    free(buffer);
    fclose(file);
    if (isArchive(filepath)) {
        Config cfg;
        // Memory allocation for nestedArchivePath
        char *nestedArchivePath = malloc(strlen(filepath) + 1);
        if (nestedArchivePath == NULL) {
            fprintf(stderr, "failed to allocate memory for nested archive path\n");
            return EXIT_FAILURE;
        }
        strcpy(nestedArchivePath, filepath);
        // Memory allocation for cfg.inputPath
        cfg.inputPath = malloc(strlen(filepath) + 1);
        if (cfg.inputPath == NULL) {
            fprintf(stderr, "failed to allocate memory for input path\n");
            free(nestedArchivePath);
            return EXIT_FAILURE;
        }
        strcpy(cfg.inputPath, filepath);
        cfg.command = 1;
        removeFileExtension(nestedArchivePath);
        // Memory allocation for cfg.outputPath
        cfg.outputPath = malloc(strlen(nestedArchivePath) + 1);
        if (cfg.outputPath == NULL) {
            fprintf(stderr, "failed to allocate memory for output path\n");
            free(nestedArchivePath);
            free(cfg.inputPath);
            return EXIT_FAILURE;
        }
        strcpy(cfg.outputPath, nestedArchivePath);
        // Unarchive nested archive
        error err = RunUnarchiver(cfg);
        if (err != EXIT_SUCCESS) {
            fprintf(stderr, "failed to unzip nested archive to: %s\n", filepath);
            free(nestedArchivePath);
            free(cfg.inputPath);
            free(cfg.outputPath);
            return err;
        }
        // Freeing up memory
        free(nestedArchivePath);
        free(cfg.inputPath);
        free(cfg.outputPath);
        // Remove nested-archive file
        if (remove(filepath) != 0){
            fprintf(stderr, "failed to remove nested archive file: %s\n", filepath);
            return err;
        }
    }
    return EXIT_SUCCESS;
}

bool isArchive(const char *path) {
    // Find last '.' enterance
    const char *dot = strrchr(path, '.');
    // If '.' not found or file name is invalid
    if (!dot || dot == path) {
        return false;
    }
    return strcmp(dot, ".g") == 0;
}

void removeFileExtension(char *filename) {
    char *dot = strrchr(filename, '.');
    if (dot && dot != filename) {
        *dot = '\0';
    }
}
