#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_ROOM_NAME_SIZE 32
#define TOTAL_ROOMS 7

enum  ROOM_TYPE {MID_ROOM, START_ROOM, END_ROOM};


struct Room{

    int connectionIndex[TOTAL_ROOMS - 1];
    char roomName[MAX_ROOM_NAME_SIZE];
    enum ROOM_TYPE roomType;
    int numConnections;
};

void setupRooms(){
    struct Room allRooms[7];
    int i = 0, j = 0;
    int counter = 0, randNum = 0;

    //set all connections to -1 (invalid) and initialize numconnections to 0
    for(i = 0; i < TOTAL_ROOMS; i++){
        for(j = 0; j < TOTAL_ROOMS - 1; j++){
            allRooms[i].connectionIndex[j] = -1;
        }
        allRooms[i].numConnections = 0;
    }


    //Set up the start and end room, ensuring the start room is not overwritten by
    //the end room
    //as the first room in the array is already randomly determined, no need to further randomize
    allRooms[0].roomType = START_ROOM;
    //ensure all other room types set to base room
    for(i = 1; i < 7; i++){
        allRooms[i].roomType = MID_ROOM;
    }
    //randomly chooses a number from 1 to 6 to set the end room to
    randNum = rand() % (TOTAL_ROOMS - 1) + 1;
    allRooms[randNum].roomType = END_ROOM;

    //set up a 2d char array holding all the possible room names
    char roomNames[10][25];
    strcpy(roomNames[0], "Holodeck");
    strcpy(roomNames[1], "Engineering");
    strcpy(roomNames[2], "Cargo-Bay");
    strcpy(roomNames[3], "Bridge");
    strcpy(roomNames[4], "Ten-Forward");
    strcpy(roomNames[5], "Crew-Quarters");
    strcpy(roomNames[6], "Captain's-Quarters");
    strcpy(roomNames[7], "Sickbay");
    strcpy(roomNames[8], "Environmental-Control");
    strcpy(roomNames[9], "Warp-Nacelles");


    counter=0;
    //set each room to a random room name, ensuring no duplication
    while(counter < 7){
        randNum = rand() % 10;
        //room name is not blank, e.g. not already used
        if(strcmp(roomNames[randNum], "") != 0){
            strcpy(allRooms[counter].roomName, roomNames[randNum]);
            //sets a used roomName to an empty string to avoid duplicates
            strcpy(roomNames[randNum], "");
            counter++;
        }
    }




    int currentRoom = 0, randomRoom = 0, randomConnections = 0;

    //randomly sets up the connections for each room
    while(currentRoom < 7){
        //generates a random number 1 - 100, and then sets number of random connections based on that number
        // I wanted more granularity in my randomizer method, since it was skewing very high most of the time since
        //each subsequent room had a chance to back-fill previous rooms
        randomConnections = rand() % 100 + 1;
        if(randomConnections <= 50){
            randomConnections = 3;
        }
        else if(randomConnections > 50 && randomConnections <= 80){
            randomConnections = 4;
        }
        else if(randomConnections > 80 && randomConnections < 95){
            randomConnections = 5;
        }
        else{
            randomConnections = 6;
        }




        while(allRooms[currentRoom].numConnections < randomConnections){

            randomRoom = rand() % TOTAL_ROOMS;

            //Make sure the randomly selected room is not the same as the current room we are trying to make connections for
            if(randomRoom != currentRoom){
                for(i = 0; i <= allRooms[currentRoom].numConnections; i++){
                    if(allRooms[currentRoom].connectionIndex[i] == randomRoom){
                        //connection between the two already exists - break loop
                        i = allRooms[currentRoom].numConnections;
                    }
                    else if(allRooms[currentRoom].connectionIndex[i] == -1){
                        //empty connection space, create new connection
                        allRooms[currentRoom].connectionIndex[i] = randomRoom;
                        //have already checked for duplicates, can assign to next empty spot on other connection array
                        allRooms[randomRoom].connectionIndex[allRooms[randomRoom].numConnections] = currentRoom;
                        //increment number of connections for both rooms
                        allRooms[currentRoom].numConnections = allRooms[currentRoom].numConnections + 1;
                        allRooms[randomRoom].numConnections = allRooms[randomRoom].numConnections + 1;
                        //cause for loop to break so we don't get duplicative entries due to the numconnections being incremented
                        i = allRooms[currentRoom].numConnections;


                    }
                }
            }
        }
        currentRoom++;

    }


    //set up the directory and files for writing
    //copy all the room details from thes tructs
    //and write to the various room files
    char directoryName[50];
    char pid[6];
    char* directoryBegin = "moyerjo.rooms.";
    char fileName[84];
    memset(directoryName, '\0', 50);
    memset(pid, '\0', 6);
    sprintf(pid, "%d",getpid());
    strcpy(directoryName, directoryBegin);
    strcat(directoryName, pid);
    mkdir(directoryName, 0700);
    FILE *roomFile;

    for(i=0; i < 7; i++) {
        //generate the full path of the filename and open a file to write
        memset(fileName, '\0', 84);
        strcpy(fileName, "./");
        strcat(fileName, directoryName);
        strcat(fileName, "/");
        strcat(fileName, allRooms[i].roomName);
        roomFile = fopen(fileName, "w");

        //print the room name to the file
        fprintf(roomFile, "ROOM NAME: %s\n", allRooms[i].roomName);


        //print all the connections to the file
        for(j = 0; j < allRooms[i].numConnections; j++){
            fprintf(roomFile, "CONNECTION %d: %s\n", j + 1, allRooms[allRooms[i].connectionIndex[j]].roomName);
        }

        //print the room type to the file depending on the room type enum
        switch((int)allRooms[i].roomType){
        case 0:
            fprintf(roomFile, "ROOM TYPE: MID_ROOM\n");
            break;
        case 1:
            fprintf(roomFile, "ROOM TYPE: START_ROOM\n");
            break;
        case 2:
            fprintf(roomFile, "ROOM TYPE: END_ROOM\n");
            break;
        default:
            break;
        }
    fclose(roomFile);

    }


}

int main() {

    //seed random number generator
    srand((unsigned)time(NULL));

    setupRooms();






    return 0;
}