#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/************************************************************************************************************/
/*                       User Guide                                                                         */   
/*         *** Everything to change is located right beneath this guide                                     */                  
/*  Data to input:                                                                                          */                     
/*      * fileName1 is the file with the project choices in. It can be produced by Excel.                   */
/*	     Each column represents a pair of students, and each row a project. Values of 1 to 4 should be  */
/*           filled in, representing the pairs preference. If the pair have not picked a project,           */
/*           leave that cell empty.  */
/*           Make sure the file is a CSV file. There is a Excel error where leaving the top / bottom row    */
/*           empty can cause Excel to leave it out when saving the file as a CSV. This can be               */
/*           avoided with a dummy column of letters. See example file "Example of working around Excel error.csv"     */
/*                                                                                                          */
/*      * fileName2 is the file with the supervisor constraints in.                                         */
/* 	     Each project has a row, and each supervisor a column. For each project a supervisor            */
/*           supervises, a weighting is assigned. This should be between 0 and 1, and a decimal             */
/*           (not a fraction). Leave any other cells empty. See example file (include example).             */
/*           The weightings provide flexibility in the number of projects supervised by one lecturer but ensure that a */ 
	     /*           lecturer cannot be assigned more WORK than is appropriate.                        */
/*           They are allowed up to (and including) a total of unit total weight or else the allocation is rejected. For example,  */
/*           a supervisor can be assigned two projects with weighting 0.5, but can't be assigned            */ 
/*           three projects with weighting 0.5. Alternatively a supervisor may be the main supervisor on one and three   */
/*           supervisor on two others another so one we can indicate this by weights 0.66, 0.33, 0.33 0.33.      */
/*           Should be a CSV file.                                                                          */
/*                                                                                                          */
/* Variables to change:                                                                                     */
/*   * rows - this is the number of projects on offer (and hence the number of rows in both                 */
/*            your fileName1 and fileName2.                                                                 */
/*   * cols - this is the number of pairs who have chosen projects (and hence the number of                 */
/*            columns in fileName1). Update the constant "#define cols as well.                                                                      */
/*   * numLec - the number of supervisors (and hence the number of columns in fileName2).                   */
/*   * weightings - 'weighti' is for preference i used in the function 'energy'. Needs to be integers.                           */
/*                                                                                                          */
/* When compiling in the terminal, compile this program with the random num generator file 'ranvec.c'       */
/* and '-lm' for math library                                                                               */ 
/*                                                                                                          */
/*  Output files:                                                                                           */
/* The data saves to file 'finalConfig.txt' which contains the pair number, project number they are given   */
/* and their preference for this project.                                                                   */
/************************************************************************************************************/

/*Variables to change */
int rows = 67; /* NUMBER OF PROJECTS */
int cols = 24; /* NUMBER OF PAIRS (some might be singletons) */
#define cols 24
int numLec = 30; /* NUMBER OF LECTURERS */
char fileName1[] = "Dataset3CSV.csv"; /* This file has the data to fill choices - is passed into readChoices */
char fileName2[] = "LecturersDataset3CSV.csv"; /* This file has the data to fill in supConstraint - is passed into readLecturers */

/*weightings*/
/***THIS IS VERSION WITH 4.7, 4.15, 3, 2.3 (out of 5)**/
float weight1 = (float)100/(float)cols;
float weight2 = ((float)100/(float)cols) * ((float)4.15/(float)4.7);
float weight3 = ((float)100/(float)cols) * ((float)3/(float)4.7);
float weight4 = ((float)100/(float)cols) * ((float)2.35/(float)4.7);

/*global variables*/
double rands[100000]; /* home to random numbers */
double temp = 5; /* starting temperature */


float energy( int projPref[] ); /* calculates energy of a given allocation */
int projClashFullCount(int projNum[]); /* counts clashes between allocations */
void generateRandomNumbers(); //ranvec.c
int randomNum( float random, int divisor ); /* turns a random number into modulo divisor so we can use it */
void changeAllocationByPref( int choices[rows][cols], int projNum[cols], int projPref[cols], int changes[] ); /* changes allocation of ONE PAIRS project based on random choice of preference */
void readChoices( int choices[rows][cols] ); /* reads in the choices file */
void readLecturers( float supConstraint[rows][numLec] ); /* reads in the lecturer constraint file */
int countViolations( int projNum[],float supConstraint[rows][numLec] ); /* counts violations of constraints */
int countSupConstraintClashes( float supConstraint[rows][numLec], int projNum[], int proj); /*counts violations of lectuere constraint */
void createInitialConfiguration( int choices[rows][cols], int projNum[cols], int projPref[cols], int changes[], float supConstraint[rows][numLec] ); /* does what it says */
void cycleOfMoves( int choices[rows][cols], int projNum[cols], int projPref[cols], int changes[], float supConstraint[rows][numLec], FILE* saveData ); /* Does all the moves for a fixed temp.*/
void init_vector_random_generator(int ,int);
void vector_random_generator(int, double *);
/* end of function initialisations */

int main() {

	int choices[rows][cols]; /* This has the choices the pair made in. We import it from csv file. */
	float supConstraint[rows][numLec]; /* This has all the data needed for calculating supervisor constraints in - including which projects a supervisor has and how many they can supervise. Imported from csv file */
	int i,j; 
	int projNum[cols]; /* for each pair, stores what number project they are currently assigned */
	int projPref[cols]; /* for each pair, stores what preference their currently assigned project is. NOTE the preference stored here is not zero-indexed. */
	int changes[3]; /* 0 is PAIR, 1 is PROJECT, 2 is PREF */	
	
	FILE *finalConfig; /* this file saves the final configuration - which pair have what project */
	FILE *saveData;

	generateRandomNumbers(); 
	
	/* read in Data */
	readChoices( choices);
	readLecturers( supConstraint );
	
	createInitialConfiguration( choices, projNum, projPref, changes, supConstraint );
	/* We have a starting configuration WITH NO VIOLATIONS. */
	saveData = fopen("newData.txt", "w");
	
	/* Simulated Annealing time 
	   So, we stay at one temperature until either 1000*cols moves or 100*cols Succesful Moves. 
	   Then decrease and go again.	
	*/
	while ( temp >= 0 ) {
		cycleOfMoves( choices, projNum, projPref, changes, supConstraint, saveData );
		/* decrease temp */
		temp=temp-0.001;
	}
	
	printf("Final energy is %f\n", energy(projPref));
	finalConfig = fopen("finalConfig.txt", "a");	
	/* print final configuration to file */
	for (i=0; i<cols; i++) {
		fprintf(finalConfig, "%d,%d,%d\n", i+1, projNum[i]+1, projPref[i]);
	}
	fprintf(finalConfig, "Final energy: %f\n", energy(projPref) );
	fclose(finalConfig);
	

	return 0;
}

void cycleOfMoves( int choices[rows][cols], int projNum[cols], int projPref[cols], int changes[], float supConstraint[rows][numLec], FILE* saveData ) {
	int successfulmoves = 0;
	int moves = 0;
	int same = 0;
	float trialEnergy, currentEnergy;
	float changeEnergy, ratioEnergy;
	int lecClashes;
	
	generateRandomNumbers(); 
	currentEnergy = energy( projPref );
	printf("Temperature %f\nCurrent Energy = %f\n\n", temp,currentEnergy);
	while ( moves < ( 1000 * cols ) && successfulmoves < ( 100 * cols ) ) { 
		moves++;
		successfulmoves++; 
		/* change the allocation here */
		changeAllocationByPref( choices, projNum, projPref, changes );

		//printf(" weight 1: %f, weight 2: %f, weight 3: %f, weight 4: %f\n", weight1, weight2, weight3, weight4);
		trialEnergy = energy( projPref ); /* energy of our new allocation */
		//printf("current energy and trial energy, %d, %d\n", currentEnergy, trialEnergy);
		changeEnergy = (float)( trialEnergy - currentEnergy ); 
		ratioEnergy = fabs( (float)trialEnergy / (float)currentEnergy ); /*need to be floats for things to work */

		vector_random_generator(1,rands);
		
		lecClashes=countSupConstraintClashes( supConstraint, projNum, projNum[changes[0]] ); /* projNum[changes[0]] != changes[1] at this point. The former is current proj, the latter old proj */		

		if ( projClashFullCount( projNum ) > 0 ) { /* Reject configuration due to clash - revert changes and reduce succesful move counter */
			projNum[changes[0]] = changes[1];
			projPref[changes[0]] = changes[2];

			successfulmoves--;
		//	printf("ttttttttttttttthere was a clash\n");
		} else if (temp > 0 && rands[0] > exp( -changeEnergy / temp ) ) { /* Reject configuration due to energy - revert changes */
			projNum[changes[0]] = changes[1];
			projPref[changes[0]] = changes[2];
			successfulmoves--;
		//	printf("reject due to energy\n");

		} else if ( temp == 0 && trialEnergy > currentEnergy){ /* Reject due to energy in T=0 case */

			projNum[changes[0]] = changes[1];
			projPref[changes[0]] = changes[2];
			successfulmoves--;
		//	printf("reject due to energy\n");
		} else if ( lecClashes>0 ) { /* reject due to lecturer constraint violation */
			projNum[changes[0]] = changes[1];
			projPref[changes[0]] = changes[2];
			successfulmoves--;
		//	printf("reject due to lecturers\n");
		}
			
		if ( currentEnergy == trialEnergy ) { /* This is (in theory) impossible. We count because as its a nice tracker for if things are broken. */
			//printf("This shouldn't be happening?\n\n");
			same++;
			successfulmoves--;
		}	
		
		currentEnergy = energy( projPref );
		//fprintf(saveData, "%d ", currentEnergy);
		//fprintf(saveData, "\n");
				
	}
}


/* calculates energy of a given allocation. The weights are in here. RETURNS energy */
float energy( int projPref[] ) {
	int i = 0;
	float energy = 0;
	for ( i=0; i<cols; i++ ) {
		switch ( projPref[i] ) {
			case 1:
				energy -= weight1;
				break;
			case 2:
				energy -= weight2;
				break;
			case 3:
				energy -= weight3;	
				break;
			case 4:
				energy -= weight4;
				break;
		}
	}
	
	return energy;
}

/* Counts how many clashes there are in the allocation. RETURNS this. If 0, no clashes. */
int projClashFullCount ( int projNum[] ){
	int i, j, count = 0;
	for ( i = 0; i < cols; i++) {
		for ( j = i; j < cols; j++) {
			if ( i != j ) {
				if ( projNum[i] == projNum[j] ) {
					count++;
				 }
			}
		}
	}
	return count;
}

/* important number generator thingy */
void generateRandomNumbers() {

	long int seed, nrand=100000;
  
	time((time_t *)&seed);
	init_vector_random_generator(seed,nrand);
}

/* takes a random Number (which is between 0 and 1) and makes it between whatever we want. RETURNS this number. */ 
int randomNum( float random, int divisor ) {
	int number;
	random = random * 10000;
  	number = (int) random%divisor;
	return number;
}

/* This functions CHANGES THE ALLOCATION. Based on picking a pair, and then picking a project, and then making the change. Stores the change nicely in the changes function.*/
void changeAllocationByPref( int choices[rows][cols], int projNum[cols], int projPref[cols], int changes[] ) {
	int pair, pref;
	int j;
	int go = 0; /* while looper */
	
	vector_random_generator(1, rands);
	pair = randomNum(rands[0], cols);
	//printf("\npair current pref is %d\n", projPref[pair]);

	while( go == 0){ /* avoid picking same preference - waste of a move and time. */
		vector_random_generator(1,rands);
		pref = randomNum(rands[0], 4);
		//printf("chosen pref is %d\n", pref+1);
		if ( projPref[pair] != pref+1 ) { /* then project will try and change. pref+1 as pref in [0,3] need [1,4] */
			go = 1;
		}
	}
	changes[0] = pair;
	changes[1] = projNum[pair];
	changes[2] = projPref[pair]; /* = choices[changes[1]][changes[0]] = choices[projNum[pair]][pair]*/
	//printf("Energy before reallocation is %d\n", energy(projPref));
	/* make the change */
	for( j=0; j<rows; j++){
		if( choices[j][pair] == pref+1 ) {
			projNum[pair] = j;
			projPref[pair] = pref+1;
		}
	}
	//printf("Energy after reallocation is %d\n", energy(projPref));

}

/* Does what it says. RETURNS a count */
int countViolations( int projNum[], float supConstraint[rows][numLec] ) {
	int count=0;
	int k;
	count += projClashFullCount(projNum);
	for ( k=0; k<cols; k++ ) {
		count +=countSupConstraintClashes( supConstraint, projNum, k );
	}

	return count;
}
	
/* counts how many times the lecturer/supervisor constraint is violated. */
int countSupConstraintClashes(float supConstraint[rows][numLec], int projNum[], int proj){ //for each project assigned to a pair, how many times is lecturer constraint violated.
	int i, j, l; 
	/*
	j is lectrer
	i is project / row
	l is pair.
	*/
	float sum = 0;
	int clash = 0;
	/*so, for the project proj we look across the row to see which supervisors it has. Then we go down the supervisors column and sum up the energy of the projects allocated ONLY (projNum==i bit). If sum > 1, violation */
	for( j = 0; j < numLec; j++ ) {
		sum = 0;
		if( supConstraint[proj][j] != 0 ) {
			for( i = 0; i < rows; i++ ) {
				for( l = 0; l < cols; l++ ) {
					if( projNum[l] == i ) {
						sum += supConstraint[i][j];
					}
				}
			}

			if ( sum > 1) { 
				clash++;
			}
		}
	}
	return clash;
}

/* create an initial configuration. Start at random, and then accept any change (again randomly determined) that reduces the number of constraints being violated. We are finished when no constraints are being vioalted. */

void createInitialConfiguration( int choices[rows][cols], int projNum[cols], int projPref[cols], int changes[], float supConstraint[rows][numLec] ) {

	int violationCount1, violationCount2; /* count number of violations. 1 is "old", 2 is "current" */
	int pref; /* integer from 1 to 4 */
	int i, j; /* loop counters */
	for ( i=0; i<cols; i++ ) {
      
		vector_random_generator(1,rands);
		pref = randomNum(rands[0], 4);
		
		for( j=0; j<rows; j++) { /* find the choice with the preference, and assign it */	
			if( choices[j][i] == pref+1 ){
				projNum[i] = j;
				projPref[i] = pref + 1;
			}
		}
	}	

	
	violationCount1 = countViolations( projNum, supConstraint );
	while( violationCount1 > 0 ){
	  //	  printf("violationCount1=%i\n",violationCount1);

		changeAllocationByPref( choices, projNum, projPref, changes );
		violationCount2 = countViolations( projNum, supConstraint );
		if( violationCount2 > violationCount1 ) { /* In this case, the number of violations has INCREASED, so we REJECT it and REVERT to the old allocation. */
			projNum[changes[0]] = changes[1];
			projPref[changes[0]] = changes[2];
		} else { /* update violationCount1 */	
			violationCount1 = violationCount2; 
		}
		
	}

}

/* read in the data for which pairs have what projects as their choices */
void readChoices ( int choices[rows][cols] ) {
	FILE *data;
	int x, y=0, z=0; /* x is our character as an integer, y is the row of the choices array, and z the column. */
	char c='A', d=','; /* c is the character currently being read. d is the last character read. Start with a comma. */
	data = fopen(fileName1, "r");
	/* read through file char by char */
	while( c != EOF ) {
		c =  fgetc(data);
		x = c - '0'; /* turn to int */
		switch( c ) {
			case ',': /* if two commas in a row, then a zero...Can't think of a case where this isn't true, unless the data was weirdly spaced or something. */
				if ( d == ',' ) { /* last choice was a comma then we have had an "empty space" in our data table - so a zero */
					choices[y][z] = 0;
					z++;
				}
				break;
				
			case '\n': /* new line - so reset column, move on to new row, make d a comma again */
				y++;
				z = 0;
				c = ','; /* because c becomes d after the switch, and this helps tell if there is a zero at the start of a new line. */
				break;
				
			case EOF: /* end of file */
				break;
				
			case '\r': /* this happens at end of lines, before new lines */
				if ( d==',' ) { /* if comma last thing before end of line, zero again. */
					choices[y][z] = 0;
				};
				break;
				
			case '1':
			case '2':
			case '3':
			case '4':
				choices[y][z] = x;
				z++;
				break;
			default:
				break;
			}

		d = c; /* update last char. */
		}	
	fclose(data);
}

/* reads in the lecturer constraint into supConstraint */
void readLecturers( float supConstraint[rows][numLec] ) {
	FILE* data;
	int y = 0, z = 0, i = 1; /* y is the ROW (project), z is the COLUMN (suprevisor). i keeps track of decimal places in the numbers being read in. */
	char c='A', d=','; /* c is char being read in. d the previous char */
	float tempVar=0, x; /* x is the float version of the charcter being read in c. tempVar is a temporary sum */
	data = fopen(fileName2, "r");
	
	while( c != EOF ) { 
		c =  fgetc(data);
		x = (float) c - '0'; 
		
		switch(c) {
			case ',': 
				if ( d==',' ) { /* empty cell. add zero */
					supConstraint[y][z] = 0;
					z++;
				} else if ( tempVar>0 ) { /* if we have a non-empty value, we add it to the array */
					supConstraint[y][z] = tempVar;
					z++;
					i = 1;
					tempVar = 0;
				}
				break;
				
			case '\n': /* reset column, new row */
				y++;
				z = 0;
				c = ','; 
				break;
				
			case EOF: /* end of file */
				break;
				
			case '\r': /* this happens at end of lines, before new lines */
				if ( d == ',' ) { /* if comma last thing before end of line, zero again. */
					supConstraint[y][z] = 0;
				} else if ( tempVar>0 ) {
					supConstraint[y][z] = tempVar;
					z++;
					i = 1;
					tempVar = 0;
				}
				break;
				
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			case '0':
			/* this is messy to deal with decimals */
				tempVar += x/i;
				i = i*10;
				break;
				
			case '.':
				break;
			default:
				break;
			}

		d = c; /* update last char. */
		}
	fclose(data);
	
}	
	
