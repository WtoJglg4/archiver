#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

typedef int error;

typedef struct {
    int command; // 0 - zip, 1 - unzip
    char* inputPath;
    char* outputPath;
    char* name;
} Config;

#define MAX_PATH 4096

error writeFileContents(FILE *archive, const char *filepath);
error archiveDirectory(FILE *archive, const char *directory);

error RunArchiver(Config cfg){
    printf("\n%s %s\n", cfg.outputPath, cfg.name);
    strcat(cfg.outputPath, cfg.name);
    printf("\n%s\n", cfg.outputPath);
    FILE *archive = fopen(cfg.outputPath, "wb");
    if (!archive) {
        fprintf(stderr,"failed to create archive file, path: %s", cfg.outputPath);
        return EXIT_FAILURE;
    }
    return archiveDirectory(archive, cfg.inputPath);
}

error archiveDirectory(FILE *archive, const char *directory){
    DIR *dir = opendir(directory);
    if (!dir) {
        fprintf(stderr,"failed to open directory: %s", directory);
        return EXIT_FAILURE;
    }
    // printf("Dir %s len %ld\n", directory, dir->);
    struct dirent *entry;
    error err;
    int entitiesCounter = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue; // Skip current and parent directories
        }
        entitiesCounter++;
        char path[MAX_PATH];
        snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name); // Write path to entry
        struct stat statbuf;
        if (stat(path, &statbuf) == -1) {
            fprintf(stderr,"failed to stat file %s", path);
            // continue;
            return EXIT_FAILURE;
        }
        if (S_ISDIR(statbuf.st_mode)) {
            // If it directory, than archive it recursively
            err = archiveDirectory(archive, path);
            if (err != EXIT_SUCCESS){
                return err;
            }
        } else if (S_ISREG(statbuf.st_mode)) {
            // If it a file, than write it to archive 
            err = writeFileContents(archive, path);
            if (err != EXIT_SUCCESS){
                return err;
            }
        }
    }
    // If directory is empty
    if (entitiesCounter == 0) {
        fprintf(archive, "\n%s\n%d\n", directory, -1);
    }
    closedir(dir);
    return EXIT_SUCCESS;
}

error writeFileContents(FILE *archive, const char *filepath) {
    FILE *file = fopen(filepath, "rb");
    if (!file) {
        fprintf(stderr,"failed to open file %s", filepath);
        return EXIT_FAILURE;
    }
    // Get file`s size
    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    fseek(file, 0, SEEK_SET);
    // Write file`s name and size to archive
    fprintf(archive, "%s\n%ld\n", filepath, filesize);
    // Read file`s content and write it to archive
    char *buffer = malloc(filesize);
    if (!buffer) {
        fprintf(stderr,"failed to allocate memory for file %s", filepath);
        fclose(file);
        return EXIT_FAILURE;
    }
    fread(buffer, 1, filesize, file);
    fwrite(buffer, 1, filesize, archive);
    free(buffer);
    fclose(file);
    return EXIT_SUCCESS;
}