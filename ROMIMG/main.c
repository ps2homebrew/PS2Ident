/*
	main.c	- Main program file.
*/

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "romimg.h"

static void DisplayROMImgDetails(const ROMIMG *ROMImg)
{
    unsigned int i, TotalSize;
    struct FileEntry *file;
    char filename[11];

    if (ROMImg->date != 0)
        printf("ROM datestamp:\t%04x/%02x/%02x\n", ((unsigned short int *)&ROMImg->date)[1], ((unsigned char *)&ROMImg->date)[1], ((unsigned char *)&ROMImg->date)[0]);
    if (ROMImg->comment != NULL)
        printf("ROM comment:\t%s\n", ROMImg->comment);

    printf("File list:\n"
           "Name      \tSize\n"
           "-----------------------------\n");
    for (i = 0, file = ROMImg->files, TotalSize = 0; i < ROMImg->NumFiles; TotalSize += file->RomDir.size, i++, file++)
    {
        strncpy(filename, file->RomDir.name, sizeof(filename) - 1);
        filename[sizeof(filename) - 1] = '\0';
        printf("%-10s\t%u\n", filename, file->RomDir.size);
    }

    printf("\nTotal size: %d bytes.\n", TotalSize);
}

static void DisplaySyntaxHelp(void)
{
    printf("Syntax error. Syntax:\n"
           "\tCreate ROM image\t\t\t- ROMIMG /c <ROM image> <files>\n"
           "\tList files in ROM image\t\t\t- ROMIMG /l <ROM image>\n"
           "\tAdd file(s) to ROM image\t\t- ROMIMG /a <ROM image> <file(s)>\n"
           "\tDelete file(s) from ROM image\t\t- ROMIMG /d <ROM image> <file(s)>\n"
           "\tExtract all files from ROM image\t- ROMIMG /x <ROM image>\n"
           "\tExtract file from ROM image\t\t- ROMIMG /x <ROM image> <file>\n");
}

static void DisplayAddDeleteOperationResult(int result, const char *InvolvedFile)
{
    switch (result)
    {
        case 0: //No error.
            printf("done!\n");
            break;
        case ENOENT:
            printf("file not found.\n");
            break;
        case EIO:
            printf("Error writing to file: %s\n", InvolvedFile);
            break;
        case EEXIST:
            printf("File already exists.\n");
            break;
        default:
            printf("failed! code: %d\n", result);
    }
}

int main(unsigned int argc, char **argv)
{
    int result;
    FILE *OutputFile;
    ROMIMG ROMImg;
    unsigned int i, FilesAffected;
    char filename[11];
    struct FileEntry *file;

    printf("PlayStation 2 ROM image generator v1.12\n"
           "---------------------------------------\n\n");

    if (argc < 2)
    {
        DisplaySyntaxHelp();
        return EINVAL;
    }

    if (argc >= 4 && strcmp(argv[1], "/c") == 0)
    {
        if ((result = CreateBlankROMImg(argv[2], &ROMImg)) == 0)
        {
            for (FilesAffected = 0, i = 0; i < argc - 3; i++)
            {
                printf("Adding file %s...", argv[3 + i]);
                if ((result = AddFile(&ROMImg, argv[3 + i])) == 0)
                    FilesAffected++;
                printf(result == 0 ? "done!\n" : "failed!\n");
            }

            if (FilesAffected > 0)
            {
                printf("Writing image...");
                printf("%s", (result = WriteROMImg(argv[2], &ROMImg)) == 0 ? "done!\n" : "failed!\n");
            }
            UnloadROMImg(&ROMImg);
        }
        else
            printf("(Internal fault) Can't create blank image file: %d. Please report.\n", result);
    }
    else if (argc >= 4 && strcmp(argv[1], "/a") == 0)
    {
        if ((result = LoadROMImg(&ROMImg, argv[2])) == 0)
        {
            for (i = 0, FilesAffected = 0; i < argc - 3; i++)
            {
                printf("Adding file %s...", argv[3 + i]);
                if ((result = AddFile(&ROMImg, argv[3 + i])) == 0)
                    FilesAffected++;
                DisplayAddDeleteOperationResult(result, argv[3 + i]);
            }

            if (FilesAffected > 0)
            {
                printf("Writing image...");
                printf("%s", (result = WriteROMImg(argv[2], &ROMImg)) == 0 ? "done!\n" : "failed!\n");
            }
            UnloadROMImg(&ROMImg);
        }
        else
            printf("Can't load image file: %s\n", argv[2]);
    }
    else if (argc >= 4 && strcmp(argv[1], "/d") == 0)
    {
        if ((result = LoadROMImg(&ROMImg, argv[2])) == 0)
        {
            for (i = 0, FilesAffected = 0; i < argc - 3; i++)
            {
                printf("Removing file %s...", argv[3 + i]);
                if ((result = DeleteFile(&ROMImg, argv[3 + i])) == 0)
                    FilesAffected++;
                DisplayAddDeleteOperationResult(result, argv[3 + i]);
            }

            if (FilesAffected > 0)
            {
                printf("Writing image...");
                printf("%s", (result = WriteROMImg(argv[2], &ROMImg)) == 0 ? "done!\n" : "failed!\n");
            }
            UnloadROMImg(&ROMImg);
        }
        else
            printf("Can't load image file: %s\n", argv[2]);
    }
    else if (argc == 3 && strcmp(argv[1], "/l") == 0)
    {
        if ((result = LoadROMImg(&ROMImg, argv[2])) == 0)
        {
            DisplayROMImgDetails(&ROMImg);
            UnloadROMImg(&ROMImg);
        }
        else
            printf("Can't load image file: %s\n", argv[2]);
    }
    else if ((argc == 3 || argc == 4) && strcmp(argv[1], "/x") == 0)
    {
        if ((result = LoadROMImg(&ROMImg, argv[2])) == 0)
        {
            if (argc == 3)
            {
                printf("File list:\n"
                       "Name      \tSize\n"
                       "-----------------------------\n");
                for (i = 0, file = ROMImg.files; i < ROMImg.NumFiles; i++, file++)
                {
                    strncpy(filename, file->RomDir.name, sizeof(filename) - 1);
                    filename[sizeof(filename) - 1] = '\0';
                    printf("%-10s\t%u\n", filename, file->RomDir.size);

                    if (file->RomDir.size > 0)
                    {
                        if ((OutputFile = fopen(filename, "wb")) != NULL)
                        {
                            if (fwrite(file->FileData, 1, file->RomDir.size, OutputFile) != file->RomDir.size)
                            {
                                printf("Error writing to file %s\n", filename);
                            }
                            fclose(OutputFile);
                        }
                        else
                        {
                            printf("Can't create file: %s\n", filename);
                        }
                    }
                }
            }
            else
            {
                printf("Extracting file %s...", argv[3]);
                DisplayAddDeleteOperationResult(result = ExtractFile(&ROMImg, argv[3], argv[3]), argv[3]);
            }

            UnloadROMImg(&ROMImg);
        }
        else
            printf("Can't load image file: %s\n", argv[2]);
    }
    else
    {
        printf("Unrecognized command or incorrect syntax: %s\n", argv[1]);
        DisplaySyntaxHelp();
        result = EINVAL;
    }

    return result;
}
