#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>

#define PATIENT_TIME 3 // This time can be changed (cannot be 0)
#define TEST_TIME 8 // testing time

#define PATIENT_NUMBER 120 // created total patient
#define STAFF_NUMBER 8 // Staff thread number
#define ROOM_NUMBER 8 // Total room number
#define CAPACITY_OF_ROOM 3 // capacity of each room

#define NOTCREATED -1// if room is not created
#define ANNOUNCING 0 // if there are one or two patients in room
#define VENTILATING 1 // if room is empty
#define BUSY 2 // if room is full

sem_t testing[ROOM_NUMBER]; // For the patients in room
sem_t waitingHole; // For the waiting patients at the hole of hospital
sem_t testRoom[ROOM_NUMBER]; // For the testRoom
sem_t mutex; // For the critical sections

int remainingPlacesInTestRoom[ROOM_NUMBER] = {0, 0, 0, 0, 0, 0, 0, 0}; // Keep the number of the patients in the each test room
int staffStatus[STAFF_NUMBER] = {-1, -1, -1, -1, -1, -1, -1, -1}; // Keep the status of the healthcare staff
int starvationStatus[STAFF_NUMBER] = {0, 0, 0, 0, 0, 0, 0, 0}; // Keep the service number of rooms to prevent starvation

int createdPatients = 0; // Current totoal creadted patients
int testedPatients = 0; // Patients who have been tested
int roomId = -1; // to determine which room it is

int compareStarvation(int starvation[STAFF_NUMBER]){ //Finding the minumum starvation number for all rooms
	
	int counter = 1;
	int returnValue = starvation[0];
	while(counter < ROOM_NUMBER){
		if(starvation[counter] < starvation[counter - 1]){
			 returnValue = starvation[counter];
		}
	     	counter++;
	}
	return returnValue;
}

bool isBusy(int busyState){// if returns true then the room is at busy state
	if(busyState==2){
		return true;
	}
	else{
		return false;
	}
}

bool isVentilating(int ventilateState){//if returns true then the room is at ventilate state
	if(ventilateState==1){
		return true;
	}
	else{
		return false;
	}
}

bool isNotCreated(int notCretaedState){// if returns true then the room is at NotCreated state
	if(notCretaedState==-1){
		return true;
	}
	else{
		return false;
	}
}

bool isAnnouncing(int announceState){// if returns true then the room is at announce state
	if(announceState==0){
		return true;
	}
	else{
		return false;
	}
}

int comparator(int number1, int number2){ // compare two numbers
	
	if(number1 == number2){
		return 0;
	}
	else
		return -1;
	
}

void simulation() // Simulate the program
{

	printf("\033[0m"); //Resets the text to default color.
	printf("\033[0;32m");//Set the text to the color green.
	printf(" \n\t\t\t*************************************\n\n");
	printf("\033[0m"); //Resets the text to default color.
	printf("\033[0;32m");
	printf("\t\t\t\tCOVID-19 TEST SIMULATION\n");
	printf("\033[0m"); //Resets the text to default color.
	printf("\033[0;31m"); //Set the text to the color red.
	printf("   'P' means patient and 'A' means avaliable stretcher for the patient waiting to be tested. \n");
    	printf("\033[0m"); //Resets the text to default color.
	printf("\033[0;32m");//Set the text to the color green.
	printf(" \n\t\t\t*************************************\n");
	printf("\033[0m"); //Resets the text to default color.

	int counterRoomNumber=0; // to define which room it is
	int counterStretcher=0; // for counting strecther in while loop
	
	while(counterRoomNumber < ROOM_NUMBER){
		printf("\033[0;31m"); //Set the text to the color red.
		printf("Room Number: TR-%d --> ",counterRoomNumber+1);
		printf("\033[0m"); //Resets the text to default color.
		
		while(counterStretcher < remainingPlacesInTestRoom[counterRoomNumber]){
			printf("|");
			printf("\033[0;36m");//cyan
			printf(" P ");
			printf("\033[0m"); //Resets the text to default color.
			counterStretcher++;
		}
		counterStretcher=0;
		int blankStretcher = CAPACITY_OF_ROOM - remainingPlacesInTestRoom[counterRoomNumber]; // to define blank strecther of a room
		while(counterStretcher < blankStretcher){
			printf("|");
			printf("\033[0;32m");//green
			printf(" A ");
			printf("\033[0m"); //Resets the text to default color.
			counterStretcher++;
		}
		counterStretcher=0;
		printf("|");
		
		switch(staffStatus[counterRoomNumber]){ // to define status o staff
			
			case VENTILATING:
				printf("The room is ventilating!!!");
				break;
			case BUSY:
				printf("This room is full!!!");
				break;
			case ANNOUNCING:
				if(blankStretcher !=0){
					printf("ANNOUNCING! Last %d patient for having a test!", blankStretcher);
				}
				else{
					printf("This room is full!!!");
				}
				break;				
			case  NOTCREATED:
				printf("This room cannot be used!!!");
				break;
		
		}
		printf(" #The number of starvation: %d #\n",starvationStatus[counterRoomNumber]);

		counterRoomNumber++;
	}
	printf("Patients who have been tested: %d\n",testedPatients);
	printf("Total Patient: %d\n", createdPatients);
    
}


void *patient(void *patientId)
{
    int id = (int)patientId; // defining id of patient
    int roomID;

    sem_wait(&mutex); // lock until global variable changes
    createdPatients++; 
    sem_post(&mutex); // for unlock


    sem_wait(&waitingHole); // waiting at hole.

    sem_wait(&mutex);

    while (1)
    {
       
		int minStarvation = compareStarvation(starvationStatus);// to determine which room patients will enter	   
                int maxTotalStretcher = 0; // maximum total strecher in room
		int counterStarvation=0; // created for while loop 
		
		while(counterStarvation<ROOM_NUMBER){ // to define the room which patient will enter
			
			if (isBusy(staffStatus[counterStarvation]) == true || isNotCreated(staffStatus[counterStarvation]) == true || remainingPlacesInTestRoom[counterStarvation] >= CAPACITY_OF_ROOM)
            {
		counterStarvation++;
                continue;
            }

          if(remainingPlacesInTestRoom[counterStarvation] > maxTotalStretcher && remainingPlacesInTestRoom[counterStarvation] < CAPACITY_OF_ROOM){
                roomId = counterStarvation;  
                maxTotalStretcher = remainingPlacesInTestRoom[counterStarvation];             
            }
            else if(comparator(remainingPlacesInTestRoom[counterStarvation],maxTotalStretcher) == 0 && remainingPlacesInTestRoom[counterStarvation] < CAPACITY_OF_ROOM){
                if(starvationStatus[counterStarvation] <= minStarvation){ 
                    roomId = counterStarvation; 
                }
            }
			counterStarvation++;
	}
        
        if (isBusy(staffStatus[roomId]) == false && isNotCreated(staffStatus[roomId]) == false && remainingPlacesInTestRoom[roomId] < CAPACITY_OF_ROOM)
        {
			
				
				roomID = roomId; // The result of the starvation control will be used for current patient.
				remainingPlacesInTestRoom[roomID]++; 

				switch(remainingPlacesInTestRoom[roomID]){
					case CAPACITY_OF_ROOM:
						starvationStatus[roomID]++;
						break;
				}

				if(remainingPlacesInTestRoom[roomID] < CAPACITY_OF_ROOM) 
					sem_post(&waitingHole); // Waking a patient from waiting hole 

				break;
			
        }

    }
	bool isVentilate = isVentilating(staffStatus[roomID]); // checking the ventilating status ( ventilate or not)
	switch(isVentilate){
		case true:
			sem_post(testRoom + roomID);
			sem_post(&mutex);
		case false:
			 sem_post(&mutex);
	}
    sem_wait(&mutex);
    
    if (remainingPlacesInTestRoom[roomID] < CAPACITY_OF_ROOM) // Patients will fill the form in the test room, until the room is full.
    {
        sem_post(&mutex);
        sem_wait(testing + roomID);
        sleep(1);
    }
    else if (comparator(remainingPlacesInTestRoom[roomID],CAPACITY_OF_ROOM) == 0) //ıf room is full
    {
        sem_post(&mutex);
        sem_post(testing + roomID);
        sem_post(testing + roomID);
        sem_post(testing + roomID);
    }

    return NULL;
}



void *staff(void *staffId)
{
    int id = (int)staffId; // to determine staff id
    int counter = 0;// counter for while loop
	
	while(1){
		
		sem_post(&waitingHole); // A patient waiting in the waiting hole will be warn by staff
        	sem_wait(&mutex);
        	staffStatus[id] = ANNOUNCING;
        	sem_post(&mutex);
       
        sem_wait(&mutex);
        int totalNumberInRooms = 0;
		
		while(counter < ROOM_NUMBER){
			totalNumberInRooms += remainingPlacesInTestRoom[counter];
			counter++;
		}
		counter=0;
		int noPatient = createdPatients - testedPatients - totalNumberInRooms; // patient number
		
		if ( comparator(createdPatients,0) == 0 || (comparator(noPatient,0) == 0 && comparator(remainingPlacesInTestRoom[id],0) == 0))
        { // if there is no patient in the waiting hole, the staff will ventilate the test room
            staffStatus[id] = VENTILATING; // changes the staff status to VENTILATING
            simulation();
            sem_post(&mutex);
            sem_wait(testRoom + id); 
           
        }
        else
        {
            sem_post(&mutex);
        }
		
        sem_wait(&mutex);
        staffStatus[id] = ANNOUNCING;
        sem_post(&mutex);
		int controller=1;
		
		while (controller==1) // The staff is announcing reamining stretcher int his/her room.
        {
            sem_wait(&mutex);
            if (comparator(remainingPlacesInTestRoom[id],CAPACITY_OF_ROOM) == 0) // if the room is full
            {
                staffStatus[id] = BUSY; // change the status of staff to BUSY
                sem_post(&mutex);
		controller=0;
                break;
            }

            simulation();
            sem_post(&mutex);
            sleep(1); //Announcing time for observe simulation easily
        }
		sleep(TEST_TIME); // Testing time

        sem_wait(&mutex);
        simulation();
        testedPatients += remainingPlacesInTestRoom[id]; 
        remainingPlacesInTestRoom[id] = 0;
       
        sem_post(&mutex);
	}
	return NULL;
}

int main(int argc, char *argv[])
{
    	int countFlag=0;
    pthread_t patientThreads[PATIENT_NUMBER];// this threads creating for patients
    pthread_t staffThreads[STAFF_NUMBER]; //this threads creating for staff

    srand(time(NULL));

    sem_init(&waitingHole, 0, 0);

    sem_init(&mutex, 0, 1);
	
	while(countFlag < ROOM_NUMBER){
		sem_init(testRoom + countFlag, 0, 0);
		countFlag++;
	}
	countFlag=0;
	
	while(countFlag < ROOM_NUMBER){
		sem_init(testing + countFlag, 0, 0);
		countFlag++;
	}
	countFlag=0;
    
	while(countFlag < STAFF_NUMBER){
		pthread_create(staffThreads + countFlag, NULL, &staff, (void *)countFlag); 
		countFlag++;
	}
	countFlag=0;

	while(countFlag < PATIENT_NUMBER){ // thread creation for each patient
        pthread_create(patientThreads + countFlag, NULL, &patient, (void *)countFlag);
        sleep(rand() % PATIENT_TIME); // random paient creation time
		countFlag++;
	}
	countFlag=0;
	

    while(countFlag < STAFF_NUMBER){ // join all staff's thread
		pthread_join(staffThreads[countFlag], NULL);
		countFlag++;
	}
	countFlag=0;
	
    while(countFlag < PATIENT_NUMBER){// join all patient's thread
		pthread_join(patientThreads[countFlag], NULL);
		countFlag++;
	}
	countFlag=0;

   
    
    return 0;
}