#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

enum PlaylistType
{
    MASTER,
    VARIANT
};

typedef enum
{
    COMMENT,
    URI,
    BLANK,
    TAG
} LineType;

typedef struct
{
    unsigned int index;
    LineType type;
    char *value;
} Line;

typedef struct
{
    Line **lines;
    int lineCount;
} HlsPlaylist;

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
        // This checks for UTF-8 control characters not including LF and CR
        else if ((byte >= 0x00 && byte <= 0x1F && byte != 0x0D && byte != 0x0A) || (byte >= 0x7F && byte <= 0x9F))
        {
            return false;
        }
        else
        {
            return false;
        }
    }
    return true;
}

bool hasUtf8Bom(const unsigned char *data, size_t length)
{
    // The BOM is 3 bytes
    if (length < 3)
    {
        return false;
    }

    // Check if the first 3 bytes are 0xEF, 0xBB, 0xBF
    if (data[0] == 0xEF && data[1] == 0xBB && data[2] == 0xBF)
    {
        return true;
    }

    return false;
}

HlsPlaylist readFileLines(const char *fileName)
{
    FILE *file = fopen(fileName, "rb");
    if (file == NULL)
    {
        printf("Unable to open file\n");
        return (HlsPlaylist){NULL, 0};
    }

    Line **lines = NULL;
    size_t size = 0;
    int lineCount = 0;

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), file))
    {
        lineCount++;

        LineType lineType = BLANK;
        // Blank lines are ignored
        if (strlen(buffer) == 0)
        {
            lineType = BLANK;
            continue;
        }

        if (buffer[strlen(buffer) - 1] != '\n')
        {
            printf("Invalid line ending. Lines must end with a newline character.\n");
            fclose(file);
            return (HlsPlaylist){NULL, 0};
        }

        // Get rid of the newline character
        buffer[strcspn(buffer, "\n")] = '\0';
        size_t lineSize = strlen(buffer);

        if (lineCount == 0 && hasUtf8Bom((unsigned char *)buffer, lineSize))
        {
            printf("Input file contains UTF-8 BOM. This is not supported by the HLS specification. Please remove the BOM from the input file.\n");
            fclose(file);
            return (HlsPlaylist){NULL, 0};
        }

        // Check if the line is valid UTF-8
        if (!isValidUtf8((unsigned char *)buffer, lineSize))
        {
            printf("Invalid UTF-8 sequence found in line: %s\n", buffer);
            continue;
        }

        if (buffer[0] == '#' && (buffer[1] != 'E' && buffer[2] != 'X' && buffer[3] != 'T'))
        {
            lineType = COMMENT;
        }
        else
        {
            lineType = URI;
        }

        Line **resizedLines = realloc(lines, (size + 1) * sizeof(Line *));
        if (resizedLines == NULL)
        {
            printf("Failed to reallocate space for new line\n");
            free(lines);
            fclose(file);
            return (HlsPlaylist){NULL, 0};
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
            return (HlsPlaylist){NULL, 0};
        }

        Line *line = malloc(sizeof(Line));
        line->value = strdup(buffer);
        line->index = lineCount - 1;
        line->type = lineType;
        lines[size] = line;

        size++;
        lineCount++;
    }

    fclose(file);
    return (HlsPlaylist){lines, lineCount};
}

int main(int argc, char const *argv[])
{
    HlsPlaylist playlist = readFileLines("files/test.m3u8");

    if (playlist.lines == NULL)
    {
        fprintf(stderr, "Failed to read file\n");
        return 1;
    }

    printf("Read %d lines from the file:\n", playlist.lineCount);
    for (int i = 0; i < playlist.lineCount; i++)
    {
        printf("%s\n", playlist.lines[i]->value);
    }

    // Free the memory allocated for the lines
    for (int i = 0; i < playlist.lineCount; i++)
    {
        free(playlist.lines[i]->value);
        free(playlist.lines[i]);
    }
    free(playlist.lines);

    return 0;
}
