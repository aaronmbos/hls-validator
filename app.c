#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    char **lines;
    int lineCount;
} HlsManifest;

HlsManifest readFileLines(const char *fileName)
{
    FILE *file = fopen(fileName, "r");
    if (file == NULL)
    {
        printf("Unable to open file\n");
        return (HlsManifest){NULL, 0};
    }

    char **lines = NULL;
    size_t size = 0;
    int lineCount = 0;

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), file))
    {
        // Get rid of the newline character
        buffer[strcspn(buffer, "\n")] = '\0';

        char **resizedLines = realloc(lines, (size + 1) * sizeof(char *));
        if (resizedLines == NULL)
        {
            printf("Failed to reallocate space for new line\n");
            free(lines);
            fclose(file);
            return (HlsManifest){NULL, 0};
        }
        lines = resizedLines;

        lines[size] = malloc(strlen(buffer) + 1);
        if (lines[size] == NULL)
        {
            printf("Error allocating memory for line in lines\n");
            for (int i = 0; i < size; i++)
            {
                free(lines[i]);
            }
            free(lines);
            fclose(file);
            return (HlsManifest){NULL, 0};
        }
        strcpy(lines[size], buffer);

        size++;
        lineCount++;
    }

    fclose(file);
    return (HlsManifest){lines, lineCount};
}

int main(int argc, char const *argv[])
{
    HlsManifest manifest = readFileLines("files/test.m3u8");

    if (manifest.lines == NULL)
    {
        fprintf(stderr, "Failed to read file\n");
        return 1;
    }

    printf("Read %d lines from the file:\n", manifest.lineCount);
    for (int i = 0; i < manifest.lineCount; i++)
    {
        printf("%s\n", manifest.lines[i]);
    }

    // Free the memory allocated for the lines
    for (int i = 0; i < manifest.lineCount; i++)
    {
        free(manifest.lines[i]);
    }
    free(manifest.lines);

    return 0;
}
