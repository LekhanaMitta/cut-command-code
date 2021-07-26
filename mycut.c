#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>           //for system calls for file
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>             //for string operations
#include<stdbool.h>             //for boolean operations

char *fileName;
char delim = '\t';              //delimiter is set by default as tab

bool fGiven = 0;                                    //checks if fields are given or not
bool validField[1001] = {false};                    //array in which we mark all the given fields as true

int fieldsIdx, delimIdx, highestField = 0;          /*  fieldsIdx - stores index of fields in arg vector
                                                        delimIdx - stores index of delimiter in arg vector
                                                        highestField - stores value of highest field input
                                                            by the user.
                                                    */

/*FUNCTION PROTOTYPES*/
void parseCommandLine(int argc, char* argv[]);      //parses arguments given in command line

void checkFields(char s[]);                         //checks if field inputs are correct
void markFields(char fields[]);                     //marks given field's corresponding indices 
                                                    //as true in validFields[]
void cutFile();                                     //writes output to command line
void Error();                                       //error function
/*FUNCTION PROTOTYPES END*/

/*MAIN FUNCTION*/input:-./mycut -d " " -f 1,2 filename
int main(int argc, char *argv[]){
	
	parseCommandLine(argc, argv);                   //calls parseCommandLine function

    if(fGiven == 0){
        printf("fields are not given");             //if field is not given, print the error
        Error();                                  //and exit
    }

	cutFile();                                      //calls cutFile function
	return 0;
}
/*MAIN FUNCTION END*/
	
void parseCommandLine(int argc, char* argv[]){
	int suma = 0, sumb = 0;
	
	for(int i = 1; ; i++) {                         //infinite loop

        if(argv[i] == NULL) break;                  /*the last element of argument vector
                                                        is NULL. Hence, when we reach the 
                                                        end of the vector, loop will break
                                                    */

        if(strcmp(argv[i], "-d") == 0){             //find -d in the argv (delimiter) 
            delimIdx = i+1;                         //store index of delimiter in argv
            if(strlen(argv[i+1])<= 1) delim = argv[i+1][0];     /*checks if length of delimiter
                                                                is less than or equal to 1. if yes,
                                                                stores it in delim variable
                                                                */
            
            else Error();                                   //if length is >1, gives error
        }

        else if(strcmp(argv[i], "-f") == 0){        //find -f in argv (field)  -f 1,2
            fGiven = true;
            fieldsIdx = i+1;                        //store index of fields string in argv
            markFields(argv[i+1]);                  //calls markFields function
        }

        else if (argv[i][0] != '-') {               //if argument's first char is not a hyphen
			fileName = argv[i];                     //save this as the file name
            int fd1 = open(fileName,O_RDONLY);      //open to check if file exists
		    if (fd1 < 0 && i != delimIdx && i != fieldsIdx){                    //if file doesn't exist and also
                printf("mycut: %s: no such file or directory\n", fileName);     //we are ignoring delim and field
                                                                                //as we know they are not the file
            }
		}
	}
}

void cutFile()
{
	int curField = 1, delimsFound = 0;              //curField - current field we are in, delimsFound - delimiters found
    char line_buf[10000], print[10000]; int idx = 0, idx2 = 0;      //initialize buffer arrays

	    int fd = open(fileName,O_RDONLY);           //open file in fileName
		if (fd < 0) Error();                        //if fd returns <0, file doesn't exist
		
        char c;
        while(read(fd, &c, 1)){                     /*read file descriptor character by character
                                                    until we reach EOF*/
            
            line_buf[idx] = c;                      //store all characters in line_buf array
            idx++;

            if(validField[curField] == true ||( c =='\n' && delimsFound > 0)){      /*if the current field is a valid field or 
                                                                                      or if the character is a new line */
                if(c!=delim){                       //if current character is not the delimiter
                    print[idx2] = c;                //store it in print buffer array
                    idx2++;
                }
            }            

            if(c == delim){                         //if current character is delimiter
                delimsFound++;
                
			    if(validField[curField] == true) {  //if current field is valid
					print[idx2] = delim;            //store it in print buffer array
					idx2++;
				}
                curField++;
            }

            if(c == '\n' || c+1 == EOF){            //if current character is new line or next character is EOF
                
                if(delimsFound == 0 && validField[1] == false){         //if no delims have been found in this line,
                    write(1, line_buf, sizeof(char)*idx);               //print entire line_buf array to command line
                }
                else if(delimsFound + 1 != highestField){               /*if number of delims found is 1 less than the highest
                                                                        field specified by user*/
                    write(1, print, strlen(print)-2);                   //print entire print[] array except last 2 elements
                    char nl = '\n';
                    write(1,&nl,1);                                     //print new line
				}
                else {
                    write(1, print, sizeof(char)*idx2);                 /*if we have found atleast one delimiter and number of
                                                                         delims found is not 1 less than the highest
                                                                        field specified by user*/
				}

                memset(line_buf, 0, sizeof(line_buf));                  //reset line_buf array
				memset(print, 0, sizeof(print));                        //reset print array
                idx = 0;
				idx2 = 0;
                curField = 1;
                delimsFound = 0;
            }
        }
		close(fd);                                                      //close file
}

void markFields(char fields[]){
    checkFields(fields);                                                //check if fields are correct

    int num=0;

    for(int i=0; i<strlen(fields); i++){                                 //loop over the fields string
        if(fields[i] >= '0' && fields[i] <= '9'){                        //if character is a number
            int temp_dig = (int) (fields[i]-'0');
            num = num*10 + temp_dig;                                    //converting char to digit
        }

        if (fields[i]==',' || (i==strlen(fields)-1&&fields[i]!='-')){   //if comma or last character and not hyphen
            validField[num]=true;                                       //mark num index as true in validField
            if(num>highestField) highestField = num;                    //record highest field
            num=0;
        }
        else if(fields[i] == '-')                                       //if hyphen
        {
            i++;                                                        //go to next character
            int num2=0;
            while(fields[i] != ',' && i<strlen(fields))                 //loop till I get a comma
            {
                if(fields[i]=='-')                                      //if we get more than one hyphen
                    Error();                                            //between two commas, give error
                int temp_dig2 = (int) (fields[i]-'0');
                num2 = num2*10 + temp_dig2;                             //converting char to digit
                i++;
            }
            
            if(num==0&&num2==0)                 //if both digits around hyphen are zero
                Error();     
            if(num2==0)                         //if num2 is not given, make it max.
                num2=1000;                      //for example: 3- will print all fields from 3 till end of line
            if(num>num2)                        //for example 5-3 is wrong
                Error();    
            memset(validField+num,true,num2-num+1);             //set all values from index num1 to num2 as true
            num=0;
        }
    }
}
-f 3,-2

void checkFields(char s[]){
    int len = strlen(s);                            //len - length of the string s

    if(len==0) Error();                             //Checking empty string

    if(s[0] == ',' || s[len-1] == ',') Error();     //checking if there is comma at end or start of string
    
    for(int i=0; i<len-1; i++)
        if(s[i]==','&&s[i+1]==',')                  //Checking empty segment (nothing between 2 commas)
            Error();

    for(int i=0; i<len; i++)                        //Checking if there is anything other than comma, hyphen or digits
        if(s[i] != ',' && s[i] != '-' && (s[i] < '0' || s[i] > '9')) Error();

    int num=0;
    for(int i=0; i<len; i++){                       //Checking if user has input 0

        if(s[i] >= '0' && s[i] <= '9'){
            int temp_dig = (int) (s[i]-'0');
            num = num*10 + temp_dig;                //char to digit
        }

        if (s[i]==',' || i==len-1 || s[i]=='-') 
        {
            if( (s[i]==','&&i!=len-1&&s[i+1]=='-') || (s[i]=='-'&&i!=len-1&&s[i+1]==',')   //ex: 3-,|| (s[i]=='-'&&i==len-1)                //last char hyphen|| s[i]=='-' && i==0                    //first char hyphen
            );                                          //all these are accepted

            else if (num==0)                            //if the number is zero, give error
                Error();
            num=0;
        }
    }
}

void Error(){
    printf("There is an error. Something's not right :(\n");
    _exit(0);
}