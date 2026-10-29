#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>


// This function is responsible for handling NMEA sentences.
// Returns TRUE if there are no errors in the sentence, otherwise returns FALSE.
bool handleSentence(char* sentence){

    char* words[16];
    words[0] = &sentence[0];
    uint8_t wordCount = 1;

    // Break down the sentence into various words based on whether there is a "," or '*'
    for(int i = 0; sentence[i] != '\0'; i++){

        if(sentence[i] == ',' || sentence[i] == '*'){
            sentence[i] = '\0';
            words[wordCount] = &sentence[i+1];
            wordCount++;
            if(wordCount >= 16){
                return false;
            }
        }
    }

    // Checks that the sentence has the right number of fields.
    if(wordCount != 16){
        return false;
    }

    // Sentence should be exactly 16 fields long. 
    for(uint8_t j = 0; j < 16; j++){
        switch(j){
            case 0: // Sentence type field
                if(!(words[j][0] == '$' && strstr(words[j], "GGA") != NULL)){
                    return false;
                }
                break;
            case 1:  // Time fields
                if((strspn(words[j], "0123456789")) != 6 || (words[j][6] != '\0' && words[j][6] != '.') || (words[j][6] == '.' && !(strlen(words[j]) > 7 && strspn(words[j] + 7, "0123456789") == strlen(words[j] + 7)))){
                            return false;
                }else{
                    // Get the time fields
                    uint8_t hour = (words[j][0] - '0') * 10 + (words[j][1] - '0');
                    uint8_t minute = (words[j][2] - '0') * 10 + (words[j][3] - '0');
                    uint8_t second = (words[j][4] - '0') * 10 + (words[j][5] - '0');
                    if(hour > 23 || minute > 59  || second > 59){
                        return false;
                    }
                }
                break;
            case 2: // Latitude field
                if(words[j][0] == '\0'){
                    if(words[j+1][0] == '\0'){
                        break;
                    }else{
                        return false;
                    }
                        
                }else if(((strspn(words[j], "0123456789")) != 4) || (words[j][4] != '\0' && words[j][4] != '.') || (words[j][4] == '.' && !(strlen(words[j]) > 5 && strspn(words[j] + 5, "0123456789") == strlen(words[j] + 5)))){
                    return false;
                }else{
                    uint8_t degrees = (words[j][0] - '0') * 10 + (words[j][1] - '0');
                    uint8_t minutes = (words[j][2] - '0') * 10 + (words[j][3] - '0');
                    if(degrees > 90 || minutes > 59 || !(strcmp(words[j+1], "N") == 0 || strcmp(words[j+1], "S") == 0) || (degrees == 90 && minutes != 0)){
                        return false;
                    }
                }
                break;
            case 4: // Longitude field
                if(words[j][0] == '\0'){
                    if(words[j+1][0] == '\0'){
                        break;
                    }else{
                        return false;
                    }
                        
                }else if(((strspn(words[j], "0123456789")) != 5) || (words[j][5] != '\0' && words[j][5] != '.') || (words[j][5] == '.' && !(strlen(words[j]) > 6 && strspn(words[j] + 6, "0123456789") == strlen(words[j] + 6)))){
                    return false;
                }else{
                    uint16_t degrees = (words[j][0] - '0') * 100 + (words[j][1] - '0') * 10 + (words[j][2] - '0');
                    uint8_t minutes = (words[j][3] - '0') * 10 + (words[j][4] - '0');
                    if(degrees > 180 || minutes > 59 || !(strcmp(words[j+1], "E") == 0 || strcmp(words[j+1], "W") == 0) || (degrees == 180 && minutes != 0)){
                        return false;
                    }
                }
                break;
        } 
        
        return true;
    }

}

   




//Test environment
//This main function is used to test and validate the handleSentence function.
int main(){

}