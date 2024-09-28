#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct {
    char* command;
    char* inputPath;
    char* outputPath;
} Config;

int parseCommand(int argc, char* argv[], Config* archiveCfg){
    int opt;
    char *inputPath = NULL, *outputPath = NULL;
    if (argc < 2) {
        fprintf(stderr, "Usage: %s (zip | unzip) [-i input path] [-o output path]\n", argv[0]);
        return EXIT_FAILURE;
    }
    archiveCfg->command = argv[1]; // Storing the command
    // Checking that specified command is valid
    if (strcmp(archiveCfg->command, "zip") != 0 && strcmp(archiveCfg->command, "unzip") != 0){
        fprintf(stderr, "Usage: %s (zip | unzip) [-i input path] [-o output path]\n", argv[0]);
        return EXIT_FAILURE;
    }
    argv++; // Remove command from arguments
    argc--;
    // Optional arguments processing
    while ((opt = getopt(argc, argv, "i:o:")) != -1) {
        switch (opt) {
            case 'i':
                archiveCfg->inputPath = optarg;
                printf("Input path set to: %s\n", optarg);
                break;
            case 'o':
                archiveCfg->outputPath = optarg;
                printf("Output path set to: %s\n", optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s (zip | unzip) [-i input path] [-o output path]\n", argv[0]);
                return EXIT_FAILURE;
        }
    }
    // Обработка остальных аргументов
    for (int index = optind; index < argc; index++) {
        printf("Non-option argument: %s\n", argv[index]);
    }
    return 0;
}

int main(int argc, char *argv[]) {
    Config archiveCfg;
    int err = parseCommand(argc, argv, &archiveCfg);
    if (err != 0){
        return err;
    }
    printf("Loaded config:\nCommand: %s\nInputPath: %s\nOutputPath: %s\n",
        archiveCfg.command,
        archiveCfg.inputPath,
        archiveCfg.outputPath);
    return 0;
}
