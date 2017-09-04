#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

#define smallBuffer 256
#define medBuffer 512
#define largeBuffer 1024

typedef int bool;
enum {false, true};
//global mutex
pthread_mutex_t adventureMutex = PTHREAD_MUTEX_INITIALIZER;


//function attempts to lock the current mutex thread
//when able, prints formatted time/date info to currentTime.txt
//in the same directory then prints the data printed to that file
//the console
void* printTime(void* argument){
    //lock the time thread
    pthread_mutex_lock(&adventureMutex);

    time_t rawTime;
    struct tm *currentTime;
    char timeBuffer[smallBuffer]; //holds string for time results
    char readTimeBuffer[smallBuffer]; //holds time string read from file
    memset(timeBuffer, '\0', sizeof(timeBuffer));
    memset(readTimeBuffer, '\0', sizeof(readTimeBuffer));

    //gets the current date/time and formats it as specified
    //copies formated data to a string
    time(&rawTime);
    currentTime = localtime(&rawTime);
    strftime(timeBuffer, smallBuffer, "%-l:%M%P, %A, %B %-d, %Y", currentTime);

    //prints the data to a file
    FILE * timeFile;
    timeFile = fopen("currentTime.txt", "w");
    fprintf(timeFile, "%s\n", timeBuffer);
    fclose(timeFile);

    //reads the data back from the file and prints to console
    timeFile = fopen("currentTime.txt", "r");
    fgets(readTimeBuffer, smallBuffer, timeFile);
    printf("%s\n", readTimeBuffer);

    //closes file and unlocks the mutex
    fclose(timeFile);
    pthread_mutex_unlock(&adventureMutex);

	return NULL;
}


//Function to pull the most recent directory
//Original Source: Oregon State University CS344 Lecture 2.4 - Modified by John Moyer
void getLatestDir(char* directoryName){
    int latestDirTime = -1;
    char targetDirPrefix[32] = "moyerjo.rooms.";
    char latestDirName[smallBuffer]; //holds string of last directory
    memset(latestDirName, '\0', sizeof(latestDirName));

    //holds starting directory
    DIR* thisFolder;
    //holds the current directory entity (sub-folder) of the starting folder
    struct dirent *currentSubFolder;
    //holds struct containing stats of the current directory entity
    struct stat subFolderAttributes;

    //Opens current directory
    thisFolder = opendir(".");

    //if the directory could be opened
    if(thisFolder > 0){
        //While there is a new subfolder to read in the folder
        while ((currentSubFolder = readdir(thisFolder)) != NULL){
            //if the subfolder has the correct prefix
            if(strstr(currentSubFolder->d_name, targetDirPrefix) != NULL){
                stat(currentSubFolder->d_name, &subFolderAttributes);

                //if the current subfolder is newer than the newest previously read dir
                if((int)subFolderAttributes.st_mtime > latestDirTime){
                    //update the latest dir time and then copy the directory name
                    latestDirTime = (int)subFolderAttributes.st_mtime;
                    memset(latestDirName, '\0', sizeof(latestDirName));
                    strcpy(latestDirName, currentSubFolder->d_name);
                }
            }
        }
    }
    else{
        printf("Couldn't open directory\n");
    }

    closedir(thisFolder);
    strcpy(directoryName, latestDirName);

}

void findStartEndRooms(char* directoryName, char* startRoomName, char* endRoomName){
    char fullPath[medBuffer]; //holds full path of file/directory
    char fileName[medBuffer];  //holds filename string
    char readBuffer[smallBuffer];
    FILE *fileToCheck;
    memset(fullPath, '\0', sizeof(fullPath));
    memset(fileName, '\0', sizeof(fileName));
    memset(readBuffer, '\0', sizeof(readBuffer));
    strcpy(fullPath, "./");
    strcat(fullPath, directoryName);
    strcat(fullPath, "/");

    DIR *filePath;
    struct dirent *currentFile;

    filePath = opendir(fullPath);
    //if directory opened successfully
    if(filePath > 0){
        while ((currentFile = readdir(filePath)) != NULL){
            //weed out the directory and previous directory from processing
            if(strcmp(".", currentFile->d_name) != 0 && strcmp("..", currentFile->d_name) != 0){
                //generate the full filename path
                strcpy(fileName, directoryName);
                strcat(fileName, "/");
                strcat(fileName, currentFile->d_name);

                //open the file
               fileToCheck = fopen(fileName, "r");

                //read every line
               while(fgets(readBuffer, smallBuffer, fileToCheck)){

                   //check if string contains START_ROOM
                   if(strstr(readBuffer, "START_ROOM")){
                       strcpy(startRoomName, currentFile->d_name);

                   }
                   //else check if string contains END_ROOM
                   else if (strstr(readBuffer, "END_ROOM")){
                       strcpy(endRoomName, currentFile->d_name);
                   }
                   //clear string after use
                   memset(readBuffer, '\0', sizeof(readBuffer));

               }
                //close file and clear string
                fclose(fileToCheck);
                memset(fileName, '\0', sizeof(fileName));
            }

        }
    }
    else{
        printf("Directory in findStartRoom did not open\n");
    }
    //close the directory after all files are checked
    closedir(filePath);



}




void playGame(){


    char directory[smallBuffer]; //holds directory of the last directory created by buildrooms
    char startRoomName[32]; //holds name of the start room
    char endRoomName[32];  //holds the end room name
    FILE * currentRoomFile;
    int connectionCounter = 0; //counts the number of connections for a specific room
    int i, totalSteps = 0;
    bool validConnection;
    char currentFileName[medBuffer]; //holds current filename
    char currentRoomName[medBuffer]; //holds current room name
    char totalPath[largeBuffer]; //holds the total path of all steps
    char connections[6][medBuffer]; //array of strings holding the various connections
    char* token;
    char readBuffer[medBuffer];
    char chosenRoom[medBuffer]; //holds the room chosen by player
    size_t len= 0;
    char* readString;


    //ensure the various strings are clear before use
    memset(currentFileName, '\0', sizeof(currentFileName));
    memset(readBuffer, '\0', sizeof(readBuffer));
    memset(currentRoomName, '\0', sizeof(currentRoomName));
    memset(chosenRoom, '\0', sizeof(chosenRoom));
    memset(totalPath, '\0', sizeof(totalPath));
    for(i = 0; i < 6; i++){
        memset(connections[i], '\0', sizeof(connections[i]));
    }
    memset(directory, '\0', smallBuffer);
    memset(startRoomName, '\0', sizeof(startRoomName));
    memset(startRoomName, '\0', sizeof(endRoomName));

    //gets the latest directory created under paired buildrooms program
    getLatestDir(directory);

    //find the start and end rooms
    findStartEndRooms(directory, startRoomName, endRoomName);

    //locks the current (main) thread
    pthread_mutex_lock(&adventureMutex);

    //create the second thread variables
    pthread_t timeThread;
    int threadResult;

    //set up and open the starting room file
    strcpy(currentFileName, directory);
    strcat(currentFileName, "/");
    strcat(currentFileName, startRoomName);

    //set the current room name to the starting room
    strcpy(currentRoomName, startRoomName);

    currentRoomFile = fopen(currentFileName, "r");

    //do while the end room has not been reached
    do {

        //read all lines of the current file and set up connections
        while (fgets(readBuffer, smallBuffer, currentRoomFile)) {
        //if the string contains connection
        if (strstr(readBuffer, "CONNECTION ") != NULL) {

                //tokenize the string to pull just the room name and add
                //to an array of connections for the room
                token = strtok(readBuffer, " \t");
                token = strtok(NULL, " \t");
                token = strtok(NULL, "\n");


                strcpy(connections[connectionCounter], token);

                //increment the counter for the number of connections
                //and clear the token
                connectionCounter++;
                memset(token, '\0', sizeof(token));
           }
            //clear the read buffer
            memset(readBuffer, '\0', sizeof(readBuffer));
        }

        validConnection = false;

        //while a valid connection has not been entered by the user
        while(validConnection != true){
            //print the current room, list of all connections
            //and prompt user for the next room
            printf("CURRENT LOCATION: %s\n", currentRoomName);
            printf("POSSIBLE CONNECTIONS: ");
            for(i = 0; i < connectionCounter; i++){
                printf("%s", connections[i]);
                //print comma after every connection through the next to last
                if(i < connectionCounter - 1){
                    printf(", ");
                }
                //for the last connection print a period
                else{
                    printf(".\n");
                }
            }

            printf("WHERE TO? >");

            //ensure readbuffer is reset and read in a line
            memset(readBuffer, '\0', sizeof(readBuffer));

            //read in the user's next room
            getline(&readString, &len, stdin);
            //tokenize out the newline
            token = strtok(readString, "\n");
            strcat(readBuffer, token);

            //check the input against the array of connections for the room
            for(i = 0; i < connectionCounter; i++){

                //check for a matching connection
                if(strcmp(readBuffer, connections[i]) == 0){
                    //as a valid connection was selected by the user
                    //increments number of steps, adjusts the current room to the chosen room
                    //and sets valid connection to true
                    totalSteps++;
                    strcpy(currentRoomName, connections[i]);
                    validConnection = true;
                    //intentionally breaks for loop, a valid connection was chosen
                    i = connectionCounter;
                }
            }
            //if for loop exits and no valid connection was found - invalid input
            if(strcmp(readBuffer, "time") == 0){

                threadResult = pthread_create(&timeThread, NULL, &printTime, NULL);

                if(threadResult != 0){
                    printf("DID NOT SET UP THREAD\n");
                }
                //unlock current(main) thread from mutex & join
                pthread_mutex_unlock(&adventureMutex);
                threadResult = pthread_join(timeThread, NULL);

                //lock current(main) thread
                pthread_mutex_lock(&adventureMutex);
            }
            else if(validConnection == false){
                printf("\nHUH, I DON'T UNDERSTAND THAT ROOM.  TRY AGAIN.\n");
            }
            //adds spacing after a prompt
            printf("\n");

        }

        //close the current room file
        //and build the new full file path to open
        fclose(currentRoomFile);
        memset(currentFileName, '\0', sizeof(currentFileName));
        strcpy(currentFileName, directory);
        strcat(currentFileName, "/");
        strcat(currentFileName, currentRoomName);
        //adds the room name to a string including every step taken
        strcat(totalPath, currentRoomName);
        strcat(totalPath, "\n");

        //open new room's file and reset connection counter
        currentRoomFile = fopen(currentFileName, "r");
        connectionCounter = 0;


    }while(strcmp(currentRoomName, endRoomName));
    //close the end room file (we already knew it was the last room before processing)
    //and congratulate the user and present them with their stats
    fclose(currentRoomFile);
    printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
    printf("YOU TOOK %d STEPS.  YOUR PATH TO VICTORY WAS:\n", totalSteps);
    printf("%s", totalPath);

}

int main(){


    playGame();
    //destroy mutex
    pthread_mutex_destroy(&adventureMutex);
    return 0;
}

