#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
// #include "archiver.c"
#include "unarchiver.c"

error parseCommand(int argc, char* argv[], Config* archiveCfg);
error run(Config* archiveConfig);

int main(int argc, char *argv[]) {
    Config archiveCfg;
    error err = parseCommand(argc, argv, &archiveCfg);
    if (err != EXIT_SUCCESS){
        return err;
    }
    // TODO: config validation here // validateConfig(cfg);
    printf("Loaded config:\nCommand: %d\nInputPath: %s\nOutputPath: %s\nName: %s\n\n",
        archiveCfg.command,
        archiveCfg.inputPath,
        archiveCfg.outputPath,
        archiveCfg.name);
    return run(&archiveCfg);
}

error parseCommand(int argc, char* argv[], Config* archiveCfg){
    int opt;
    char *inputPath = NULL, *outputPath = NULL;
    // TODO: This is ugly, need to create validation func here
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
    // TODO: --name flag here for archive`s name specifying
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
                // TODO: archive name validation here // validateFileName(optarg);
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
    // Обработка остальных аргументов
    for (int index = optind; index < argc; index++) {
        printf("Non-option argument: %s\n", argv[index]);
    }
    return EXIT_SUCCESS;
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
