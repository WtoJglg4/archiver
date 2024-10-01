#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "unarchiver.c"

error parseCommand(int argc, char* argv[], Config* archiveCfg);
error run(Config* archiveConfig);
error validateConfig(Config* archiveConfig);

int main(int argc, char *argv[]) {
    Config cfg;
    error err = parseCommand(argc, argv, &cfg);
    if (err != EXIT_SUCCESS){
        return err;
    }
    err = validateConfig(&cfg);
    printf("%s %s %s\n", cfg.inputPath, cfg.outputPath, cfg.name);
    if (err != EXIT_SUCCESS){
        return err;
    }
    return run(&cfg);
}
error run(Config* archiveConfig){
    switch (archiveConfig->command){
    case 0: // zip
        return RunArchiver(*archiveConfig);
    case 1: // unzip
        return RunUnarchiver(*archiveConfig);
    }
    return EXIT_SUCCESS;
}

error validateConfig(Config* archiveConfig){
    // if (strcmp(archiveConfig->inputPath, ".") == 0){
    //     archiveConfig->inputPath = "";
    // }
    // if (strcmp(archiveConfig->outputPath, ".") == 0){
    //     archiveConfig->outputPath = "";
    // }
    if (archiveConfig->command != 1 && archiveConfig->command != 0 || strcmp(archiveConfig->name, ".")==0){
        fprintf(stderr, "invalid command code\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

error parseCommand(int argc, char* argv[], Config* archiveCfg){
    int opt;
    char *inputPath = NULL, *outputPath = NULL;
    // Checking that specified command is valid
    if ((argc < 2) || (strcmp(argv[1], "zip") != 0 && strcmp(argv[1], "unzip") != 0)){
        fprintf(stderr, "Usage: %s <command> [-i input path] [-o output path]\n", argv[0]);
        fprintf(stderr, "Commands: zip, unzip\n");
        return EXIT_FAILURE;
    }
    archiveCfg->command = 0; // Storing the command
    if (strcmp(argv[1], "zip") != 0){
        archiveCfg->command = 1;
    }
    argv++; // Remove command from arguments
    argc--;
    archiveCfg->name = "archive.g";
    // Optional arguments processing
    while ((opt = getopt(argc, argv, "i:o:n:")) != -1) {
        switch (opt) {
            case 'i':
                archiveCfg->inputPath = optarg;
                printf("Input path set to: %s\n", optarg);
                break;
            case 'o':
                archiveCfg->outputPath = optarg;
                printf("Output path set to: %s\n", optarg);
                break;
            case 'n':
                archiveCfg->name = optarg;
                strcat(archiveCfg->name, ".g");
                printf("Archive name set to: %s.g\n", optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s <command> [-i input path] [-o output path]\n", argv[0]);
                fprintf(stderr, "Commands: zip, unzip\n");
                return EXIT_FAILURE;
        }
    }
    // Non-optional args processing
    for (int index = optind; index < argc; index++) {
        printf("Non-option argument: %s\n", argv[index]);
    }
    return EXIT_SUCCESS;
}
