#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct
{
    char **lines;
    int lineCount;
} HlsManifest;

// While the HLS specification requires UTF-8, it seems rather complex to check for UTF-8 validity.
// I think this does pretty well for validating byte sequences. It'll do for now.
bool isValidUtf8(const unsigned char *data, size_t length)
{
    size_t i = 0;
    while (i < length)
    {
        unsigned char byte = data[i];
        if (byte <= 0x7F)
        {
            i++;
        }
        else if ((byte & 0xE0) == 0xC0)
        {
            if (i + 1 >= length || (data[i + 1] & 0xC0) != 0x80)
            {
                return false;
            }
            i += 2;
        }
        else if ((byte & 0xF0) == 0xE0)
        {
            if (i + 2 >= length || (data[i + 1] & 0xC0) != 0x80 || (data[i + 2] & 0xC0) != 0x80)
            {
                return false;
            }
            i += 3;
        }
        else if ((byte & 0xF8) == 0xF0)
        {
            if (i + 3 >= length || (data[i + 1] & 0xC0) != 0x80 || (data[i + 2] & 0xC0) != 0x80 || (data[i + 3] & 0xC0) != 0x80)
            {
                return false;
            }
            i += 4;
        }
        else
        {
            return false;
        }
    }
    return true;
}

HlsManifest readFileLines(const char *fileName)
{
    FILE *file = fopen(fileName, "rb");
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
        size_t lineSize = strlen(buffer);

        // Check if the line is valid UTF-8
        if (!isValidUtf8((unsigned char *)buffer, lineSize))
        {
            printf("Invalid UTF-8 sequence found in line: %s\n", buffer);
            continue;
        }

        char **resizedLines = realloc(lines, (size + 1) * sizeof(char *));
        if (resizedLines == NULL)
        {
            printf("Failed to reallocate space for new line\n");
            free(lines);
            fclose(file);
            return (HlsManifest){NULL, 0};
        }
        lines = resizedLines;

        lines[size] = malloc(lineSize + 1);
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
