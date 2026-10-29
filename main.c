#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

typedef struct
{
    const char *sentence;
    bool expected;
    const char *description;
} TestCase;

#define PASS_CASE(s, d) {s, true, d}
#define FAIL_CASE(s, d) {s, false, d}

// This function is responsible for handling NMEA sentences.
// Returns TRUE if there are no errors in the sentence, otherwise returns FALSE.
bool handleSentence(char *sentence)
{

    char *words[16];
    words[0] = &sentence[0];
    uint8_t wordCount = 1;
    bool latitudeCheck = false, altitudeCheck = false, geoidalCheck = false;

    // Break down the sentence into various words based on whether there is a "," or '*'
    for (int i = 0; sentence[i] != '\0'; i++)
    {

        if (sentence[i] == ',' || sentence[i] == '*')
        {
            if (wordCount > 16)
            {
                return false;
            }
            sentence[i] = '\0';
            words[wordCount] = &sentence[i + 1];
            wordCount++;
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
            if (words[j][0] != '$' || strlen(words[j]) != 6 || (strncmp(words[j] + 3, "GGA", 3) != 0))
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
                if (degrees > 90 || minutes > 59 || !(strcmp(words[j + 1], "N") == 0 || strcmp(words[j + 1], "S") == 0) || (degrees == 90 && (minutes != 0 || (words[j][4] == '.' && strspn(words[j] + 5, "0") != strlen(words[j] + 5)))))
                {
                    return false;
                }
            }

            // Check to ensure if latitude field is present, then longitude field must also be present. If latitude field is not present, then longitude field can be present or not.
            if (words[j][0] != '\0')
            {
                latitudeCheck = true;
            }
            break;
        case 4: // Longitude field
            if (words[j][0] == '\0')
            {
                if (words[j + 1][0] == '\0' && latitudeCheck == false)
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
                if (degrees > 180 || minutes > 59 || !(strcmp(words[j + 1], "E") == 0 || strcmp(words[j + 1], "W") == 0) || (degrees == 180 && (minutes != 0 || (words[j][5] == '.' && strspn(words[j] + 6, "0") != strlen(words[j] + 6)))))
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
            // Assumption: The number of satellites is a two-digit number (00, 01, 02, ..., 12), This field can be NULL.

            if (words[j][0] == '\0')
            {
                break;
            }

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
            if (words[j][0] == '\0')
            {
                break;
            }
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
            altitudeCheck = true;
            break;
        }
        case 10: // Altitude unit field
            // Assumption: This field can be null if the altitude field is null.
            if ((words[j][0] == '\0' && !altitudeCheck))
            {
                break;
            }
            if (strcmp(words[j], "M") != 0)
            {
                return false;
            }
            break;
        case 11: // Geoidal separation field
        {
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

            geoidalCheck = true;
            break;
        }
        case 12: // Geoidal separation unit field
            // Assumption: This field can be null if the geoidal separation field is null.
            if ((words[j][0] == '\0' && !geoidalCheck))
            {
                break;
            }
            if (strcmp(words[j], "M") != 0)
            {
                return false;
            }
            break;
        case 13: // Age of differential GPS data field
        {
            if (words[j][0] == '\0')
            {
                break;
            }
            uint8_t n = strspn(words[j], "0123456789");
            if (n == 0 || (words[j][n] != '\0' && words[j][n] != '.') || (words[j][n] == '.' && !(strlen(words[j]) > n + 1 && strspn(words[j] + n + 1, "0123456789") == strlen(words[j] + n + 1))))
            {
                return false;
            }
            break;
        }
        case 14: // Differential reference station ID field
        {
            // Assumption: This field can be null.
            if (words[j][0] == '\0')
            {
                break;
            }

            if (strspn(words[j], "0123456789") != 4 || strlen(words[j]) != 4)
            {
                return false;
            }

            uint16_t stationID = (words[j][0] - '0') * 1000 + (words[j][1] - '0') * 100 + (words[j][2] - '0') * 10 + (words[j][3] - '0');
            if (stationID > 1023)
            {
                return false;
            }

            break;
        }
        default:
            break;
        }
    }
    return true;
}

// Test environment
// This main function is used to test and validate the handleSentence function.
int main(void)
{
     TestCase tests[] = {
        // ---- Should be TRUE ----
        PASS_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47", "Base"),
        PASS_CASE("$GNGGA,123519.00,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47", "GN talker"),
        PASS_CASE("$GPGGA,123519.00,,,,,0,00,,,M,,M,,*47", "No fix"),
        PASS_CASE("$GPGGA,123519.00,,,,,0,00,,,,,,,*47", "No fix, empty units"),
        PASS_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,2,08,0.9,545.4,M,46.9,M,3.0,0123*47", "Differential"),
        PASS_CASE("$GPGGA,000000,0000.000,N,00000.000,E,1,00,0,0,M,0,M,0,0000*47", "Minimum values"),
        PASS_CASE("$GPGGA,235959.999,9000.000,S,18000.000,W,8,12,99.9,9999.9,M,-999.9,M,99.9,1023*47", "Maximum values"),
        PASS_CASE("$GPGGA,123519,4807,N,01131,E,1,08,1,545,M,46,M,,*47", "No fractions"),
        PASS_CASE("$GPGGA,123519.5,4807.0,S,01131.0,W,1,08,0.9,-12.5,M,-46.9,M,,*47", "Negative values, S/W"),
        PASS_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,1,08,0.9,,M,,M,,*47", "Empty altitude/geoid"),
        PASS_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47\r\n", "CR LF ending"),
        PASS_CASE("$GPGGA,123519.00,,,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47", "Latitude empty, longitude present"),

        // ---- FALSE: framing / type ----
        FAIL_CASE("$GPGSA,123519.00,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47", "Wrong type"),
        FAIL_CASE("$GPGGAX,123519.00,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47", "Type too long"),
        FAIL_CASE("GPGGA,123519.00,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47", "Missing dollar"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,", "Missing asterisk"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,,*47", "17 words"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,*47", "15 words"),
        FAIL_CASE("", "Empty sentence"),
        FAIL_CASE("$GPGGA", "Header only"),

        // ---- FALSE: time ----
        FAIL_CASE("$GPGGA,240000.00,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47", "Hour 24"),
        FAIL_CASE("$GPGGA,126000.00,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47", "Minute 60"),
        FAIL_CASE("$GPGGA,123561.00,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47", "Second 61"),
        FAIL_CASE("$GPGGA,12351.00,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47", "Five time digits"),
        FAIL_CASE("$GPGGA,123519.,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47", "Time bare dot"),
        FAIL_CASE("$GPGGA,12a519.00,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47", "Time letter"),
        FAIL_CASE("$GPGGA,,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47", "Empty time"),
        FAIL_CASE("$GPGGA,123519.0.0,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47", "Time two dots"),

        // ---- FALSE: latitude / longitude ----
        FAIL_CASE("$GPGGA,123519.00,9100.000,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47", "Latitude 91"),
        FAIL_CASE("$GPGGA,123519.00,9030.000,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47", "Latitude 90 degrees 30"),
        FAIL_CASE("$GPGGA,123519.00,9000.500,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47", "Latitude beyond 90"),
        FAIL_CASE("$GPGGA,123519.00,4860.000,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47", "Latitude minutes 60"),
        FAIL_CASE("$GPGGA,123519.00,480.7,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47", "Latitude three digits"),
        FAIL_CASE("$GPGGA,123519.00,4807.,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47", "Latitude bare dot"),
        FAIL_CASE("$GPGGA,123519.00,-4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47", "Negative latitude"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47", "Missing N/S"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,X,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47", "Invalid N/S"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,n,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47", "Lowercase latitude direction"),
        FAIL_CASE("$GPGGA,123519.00,,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47", "Latitude empty, direction present"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,18100.000,E,1,08,0.9,545.4,M,46.9,M,,*47", "Longitude 181"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,18030.000,E,1,08,0.9,545.4,M,46.9,M,,*47", "Longitude 180 degrees 30"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,18000.500,E,1,08,0.9,545.4,M,46.9,M,,*47", "Longitude beyond 180"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01160.000,E,1,08,0.9,545.4,M,46.9,M,,*47", "Longitude minutes 60"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,1131.000,E,1,08,0.9,545.4,M,46.9,M,,*47", "Longitude four digits"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.,E,1,08,0.9,545.4,M,46.9,M,,*47", "Longitude bare dot"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,,,1,08,0.9,545.4,M,46.9,M,,*47", "Latitude present, longitude missing"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,,1,08,0.9,545.4,M,46.9,M,,*47", "Missing E/W"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,N,1,08,0.9,545.4,M,46.9,M,,*47", "Wrong E/W"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,E,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47", "Latitude uses E"),

        // ---- FALSE: quality / satellites / HDOP ----
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,9,08,0.9,545.4,M,46.9,M,,*47", "Quality 9"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,,08,0.9,545.4,M,46.9,M,,*47", "Empty quality"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,12,08,0.9,545.4,M,46.9,M,,*47", "Two-digit quality"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,A,08,0.9,545.4,M,46.9,M,,*47", "Quality letter"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,1,13,0.9,545.4,M,46.9,M,,*47", "13 satellites"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,1,8,0.9,545.4,M,46.9,M,,*47", "One-digit satellites"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,1,008,0.9,545.4,M,46.9,M,,*47", "Three-digit satellites"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,1,0a,0.9,545.4,M,46.9,M,,*47", "Satellite letter"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,1,08,.5,545.4,M,46.9,M,,*47", "HDOP no leading digit"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,1,08,5.,545.4,M,46.9,M,,*47", "HDOP bare dot"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,1,08,1.2.3,545.4,M,46.9,M,,*47", "HDOP two dots"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,1,08,-0.9,545.4,M,46.9,M,,*47", "Negative HDOP"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,1,08,abc,545.4,M,46.9,M,,*47", "HDOP letters"),

        // ---- FALSE: altitude / geoid / units ----
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,1,08,0.9,-,M,46.9,M,,*47", "Altitude minus only"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,1,08,0.9,--5,M,46.9,M,,*47", "Altitude double minus"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,1,08,0.9,+5,M,46.9,M,,*47", "Altitude plus sign"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,1,08,0.9,5.,M,46.9,M,,*47", "Altitude bare dot"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,1,08,0.9,5a,M,46.9,M,,*47", "Altitude letter"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,1,08,0.9,545.4,,46.9,M,,*47", "Altitude unit missing"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,1,08,0.9,545.4,m,46.9,M,,*47", "Lowercase altitude unit"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,1,08,0.9,545.4,F,46.9,M,,*47", "Wrong altitude unit"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,1,08,0.9,545.4,MM,46.9,M,,*47", "Altitude unit too long"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,1,08,0.9,,F,46.9,M,,*47", "Empty altitude, bad unit"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,1,08,0.9,545.4,M,-,M,,*47", "Geoid minus only"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,1,08,0.9,545.4,M,4a,M,,*47", "Geoid letter"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,,,*47", "Geoid unit missing"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,X,,*47", "Wrong geoid unit"),

        // ---- FALSE: age / station ID ----
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,2,08,0.9,545.4,M,46.9,M,-3,0123*47", "Negative age"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,2,08,0.9,545.4,M,46.9,M,3.,0123*47", "Age bare dot"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,2,08,0.9,545.4,M,46.9,M,3a,0123*47", "Age letter"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,2,08,0.9,545.4,M,46.9,M,3.0,1024*47", "Station ID 1024"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,2,08,0.9,545.4,M,46.9,M,3.0,9999*47", "Station ID 9999"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,2,08,0.9,545.4,M,46.9,M,3.0,123*47", "Station ID three digits"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,2,08,0.9,545.4,M,46.9,M,3.0,12345*47", "Station ID five digits"),
        FAIL_CASE("$GPGGA,123519.00,4807.038,N,01131.000,E,2,08,0.9,545.4,M,46.9,M,3.0,12a4*47", "Station ID letter")
    };

    int total = (int)(sizeof(tests) / sizeof(tests[0]));
    int passed = 0;

    for (int i = 0; i < total; i++)
    {
        // handleSentence modifies its input, so copy each test first.
        char sentence[256];
        snprintf(sentence, sizeof(sentence), "%s", tests[i].sentence);

        bool actual = handleSentence(sentence);

        printf("%s | %-32s | Expected: %c | Got: %c\n",
               actual == tests[i].expected ? "PASS" : "FAIL",
               tests[i].description,
               tests[i].expected ? 'T' : 'F',
               actual ? 'T' : 'F');

        if (actual == tests[i].expected)
        {
            passed++;
        }
    }

    printf("\nTests passed: %d/%d\n", passed, total);
    printf("Tests failed: %d/%d\n", total - passed, total);

    return passed == total ? 0 : 1;
}