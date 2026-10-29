#include <stdbool.h>
#include <string.h>
#include <stdint.h>


// This function is responsible for handling NMEA sentences.
// Returns TRUE if there are no errors in the sentence, otherwise returns FALSE.
bool handleSentence(char* sentence){

    bool flag = true;
    bool isValid = false;
    char* fields = sentence;
    char* words[16];
    words[0] = fields;
    uint8_t wordCount = 1;

    // Break down the sentence into various words based on whether there is a "," or '*'
    for(uint8_t i = 0; fields[i] != '\0'; i++){
        if(fields[i] == ',' || fields[i] == '*'){
            fields[i] = '\0';
            words[wordCount] = &fields[i+1];
            wordCount++;
        }
    }

    // Less than 16 as that is the maximum number of fields in a NMEA sentence.
    for(uint8_t j = 0; j < 16; j++){
        switch(j){
            case 0:
                if(fields[j] == "$--GGA"){
                    isValid = true;
                }else{
                    return false;
                }
                break;
            case 1:
                if(!isDigit(fields[j])){
                    return false;
                }else{
                    uint8_t hour = strtod(fields[j], NULL);

                }
        }
    }

    return isValid;
}



//Test environment
//This main function is used to test and validate the handleSentence function.
int main(){

}