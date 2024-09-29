#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include "archiver.c"

error extractArchive(FILE *archive, const char *outputPath);
error createDirectoryIfNotExists(const char *directory, long filesize);
error extractFile(FILE *archive, const char *filepath, long filesize);

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
    // Считываем содержимое архива, строка за строкой
    while (fscanf(archive, "%s\n%ld\n", filepath, &filesize) == 2) {
        char fullPath[MAX_PATH];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", outputPath, filepath);
        // Создаем директорию для каждого пути, даже если это не пустая директория
        error err = createDirectoryIfNotExists(fullPath, filesize);
        if (err != EXIT_SUCCESS) {
            return err;
        }
        if (filesize > 0) {
            // Это файл, восстановим его
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
    // Проходим по каждому компоненту пути и создаем директории по мере необходимости
    for (char *p = tempPath + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';  // Временная остановка на уровне пути
            if (stat(tempPath, &statbuf) != 0) {
                printf("make dir1 %s\n", tempPath);
                error err = mkdir(tempPath, 0755);
                if (err != EXIT_SUCCESS){
                    fprintf(stderr, "Failed to create directory: %s\n", tempPath);
                    return EXIT_FAILURE;
                }
            } else if (!S_ISDIR(statbuf.st_mode)) {
                fprintf(stderr, "Path exists but is not a directory: %s\n", tempPath);
                return EXIT_FAILURE;
            }
            *p = '/';  // Восстанавливаем строку
        }
    }
    if ((stat(tempPath, &statbuf) != 0) && filesize == -1){
        printf("make dir2 %s\n", tempPath);
        if (mkdir(tempPath, 0755) != 0 && errno != EEXIST) {
            fprintf(stderr, "Failed to create directory: %s\n", tempPath);
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

error extractFile(FILE *archive, const char *filepath, long filesize) {
    FILE *file = fopen(filepath, "wb");
    if (!file) {
        fprintf(stderr, "Failed to create file: %s\n", filepath);
        return EXIT_FAILURE;
    }
    // Чтение содержимого файла из архива
    char *buffer = malloc(filesize);
    if (!buffer) {
        fprintf(stderr, "Failed to allocate memory for file: %s\n", filepath);
        fclose(file);
        return EXIT_FAILURE;
    }
    fread(buffer, 1, filesize, archive);
    fwrite(buffer, 1, filesize, file);
    free(buffer);
    fclose(file);
    return EXIT_SUCCESS;
}
