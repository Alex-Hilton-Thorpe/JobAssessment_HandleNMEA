#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

// This function is responsible for handling NMEA sentences.
// Returns TRUE if there are no errors in the sentence, otherwise returns FALSE.
bool handleSentence(char *sentence)
{

    char *words[16];
    words[0] = &sentence[0];
    uint8_t wordCount = 1;

    if (strlen(sentence) > 82)
    {
        return false;
    }

    // Break down the sentence into various words based on whether there is a "," or '*'
    for (int i = 0; sentence[i] != '\0'; i++)
    {

        if (sentence[i] == ',' || sentence[i] == '*')
        {
            sentence[i] = '\0';
            words[wordCount] = &sentence[i + 1];
            wordCount++;
            if (wordCount >= 16)
            {
                return false;
            }
        }
    }

    // Checks that the sentence has the right number of fields.
    if (wordCount != 16)
    {
        return false;
    }

    // Sentence should be exactly 16 fields long.
    for (uint8_t j = 0; j < 16; j++)
    {
        switch (j)
        {
        case 0: // Sentence type field
            if (!(words[j][0] == '$' && strstr(words[j], "GGA") != NULL))
            {
                return false;
            }
            break;
        case 1: // Time fields
            if ((strspn(words[j], "0123456789")) != 6 || (words[j][6] != '\0' && words[j][6] != '.') || (words[j][6] == '.' && !(strlen(words[j]) > 7 && strspn(words[j] + 7, "0123456789") == strlen(words[j] + 7))))
            {
                return false;
            }
            else
            {
                // Get the time fields
                uint8_t hour = (words[j][0] - '0') * 10 + (words[j][1] - '0');
                uint8_t minute = (words[j][2] - '0') * 10 + (words[j][3] - '0');
                uint8_t second = (words[j][4] - '0') * 10 + (words[j][5] - '0');
                if (hour > 23 || minute > 59 || second > 59)
                {
                    return false;
                }
            }
            break;
        case 2: // Latitude field
            if (words[j][0] == '\0')
            {
                if (words[j + 1][0] == '\0')
                {
                    break;
                }
                else
                {
                    return false;
                }
            }
            else if (((strspn(words[j], "0123456789")) != 4) || (words[j][4] != '\0' && words[j][4] != '.') || (words[j][4] == '.' && !(strlen(words[j]) > 5 && strspn(words[j] + 5, "0123456789") == strlen(words[j] + 5))))
            {
                return false;
            }
            else
            {
                uint8_t degrees = (words[j][0] - '0') * 10 + (words[j][1] - '0');
                uint8_t minutes = (words[j][2] - '0') * 10 + (words[j][3] - '0');
                if (degrees > 90 || minutes > 59 || !(strcmp(words[j + 1], "N") == 0 || strcmp(words[j + 1], "S") == 0) || (degrees == 90 && minutes != 0))
                {
                    return false;
                }
            }
            break;
        case 4: // Longitude field
            if (words[j][0] == '\0')
            {
                if (words[j + 1][0] == '\0')
                {
                    break;
                }
                else
                {
                    return false;
                }
            }
            else if (((strspn(words[j], "0123456789")) != 5) || (words[j][5] != '\0' && words[j][5] != '.') || (words[j][5] == '.' && !(strlen(words[j]) > 6 && strspn(words[j] + 6, "0123456789") == strlen(words[j] + 6))))
            {
                return false;
            }
            else
            {
                uint16_t degrees = (words[j][0] - '0') * 100 + (words[j][1] - '0') * 10 + (words[j][2] - '0');
                uint8_t minutes = (words[j][3] - '0') * 10 + (words[j][4] - '0');
                if (degrees > 180 || minutes > 59 || !(strcmp(words[j + 1], "E") == 0 || strcmp(words[j + 1], "W") == 0) || (degrees == 180 && minutes != 0))
                {
                    return false;
                }
            }
            break;
        case 6: // GPS quality indicator field
            if (strspn(words[j], "012345678") != 1 || strlen(words[j]) != 1)
            {
                return false;
            }
            break;
        case 7: // Number of satellites field
            // Assumption: The number of satellites is a two-digit number (00, 01, 02, ..., 12), This field can not be NULL.
            if (strspn(words[j], "0123456789") != 2 || strlen(words[j]) != 2)
            {
                return false;
            }
            else
            {
                uint8_t numSatellites = (words[j][0] - '0') * 10 + (words[j][1] - '0');
                if (numSatellites > 12)
                {
                    return false;
                }
            }
            break;
        case 8:
        { // Horizontal dilution of precision field
            uint8_t n = strspn(words[j], "0123456789");
            if (n == 0 || (words[j][n] != '\0' && words[j][n] != '.') || (words[j][n] == '.' && !(strlen(words[j]) > n + 1 && strspn(words[j] + n + 1, "0123456789") == strlen(words[j] + n + 1))))
            {
                return false;
            }
            break;
        }
        case 9:
        { // Altitude field
            uint8_t n = 0;
            char *num = words[j];

            if (words[j][0] == '\0')
            {
                break;
            }
            else if (words[j][0] == '-')
            {
                num = words[j] + 1;
            }

            n = strspn(num, "0123456789");

            if (n == 0 || (num[n] != '\0' && num[n] != '.') || (num[n] == '.' && !(strlen(num) > n + 1 && strspn(num + n + 1, "0123456789") == strlen(num + n + 1))))
            {
                return false;
            }
            break;
        }
        case 10: // Altitude unit field
            if (strcmp(words[j], "M") != 0 || strlen(words[j]) != 1)
            {
                return false;
            }
            break;
        case 11: // Geoidal separation field
            uint8_t n = 0;
            char *num = words[j];

            if (words[j][0] == '\0')
            {
                break;
            }
            else if (words[j][0] == '-')
            {
                num = words[j] + 1;
            }

            n = strspn(num, "0123456789");

            if (n == 0 || (num[n] != '\0' && num[n] != '.') || (num[n] == '.' && !(strlen(num) > n + 1 && strspn(num + n + 1, "0123456789") == strlen(num + n + 1))))
            {
                return false;
            }
            break;
        }

        return true;
    }
}

// Test environment
// This main function is used to test and validate the handleSentence function.
int main()
{
}