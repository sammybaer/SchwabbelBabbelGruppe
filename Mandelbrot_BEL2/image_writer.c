// Bornberg
// An Image Writer program which reads the content of the shared memory segment,
// prepends a PPM header, and writes the image to a file.


// Allgemein

// shared memory accessed by 2 semaphores
// Sa and Sb with same key
// Sa: open
// Sb: closed

// End: The shared memory segment and semaphores must be removed manually using ipcs and ipcrm.
// ???????
// ipcrm -s nummer (shimid)

#include "image_writer.h"
#include "general_header.h"


void imge_printer(){
	char *openfileName = "mandelmap_";
	char ppm_input[100];
	int width = 1920;
	int height = 1080;
	int maxcolor = 255;
	unsigned int* big_mem_p3;
	// PIXEL **pRGB;

	 #if DEBUG
	 	printf("working til write\n");
	 #endif

	FILE * writeFile;
	int i;
	int filecounter = 1;

	openfileName ++;
	if(openfileName > 57){
		filecounter = 0;
		openfileName++;

	}
	openfileName = 48 + filecounter;



	// open the file for writing
	writeFile = fopen(openfileName, "w");
	if (writeFile == NULL)
	{
		perror("Error Message");
		exit(EXIT_FAILURE);
	}

	fprintf(writeFile,"P3\n");
	fprintf(writeFile,"%u %u\n", width, height);
	fprintf(writeFile,"maxcolor\n");

	// every pixel one time
	for (i = 0; i < width*height; i+=3){
		fprintf(writeFile, "%u %u %u\n", (*big_mem_p3+i), (*big_mem_p3+i+1), (*big_mem_p3+i+2));

	#if DEBUG
		printf("R: [%u]=%u\t", i, (*big_mem_p3+i));
		printf("G: [%u]=%u\t", i, (*big_mem_p3+i+1));
		printf("B: [%u]=%u\n", i, (*big_mem_p3+i+2));
	#endif
	}


	fflush(writeFile);
	fclose(writeFile);
}

int main (void)
{

	int count_transmission = 0;

	// shared memory
	int shmid;
	int *buf;
	int intbuffer[MAXMYMEM];
	int i;
	key_t key;
	int ret = 0;

	// semaphores
	int semid;
	struct sembuf sa, sb;
	union semun sema;
	union semun semb;

	// key
	key = ftok ("/etc/hostname", 'b');
	// check whether semaphore exists - if not create it
	semid = semget (key, 2, IPC_CREAT | IPC_EXCL | 0666); // 2 = anzahl
	if (semid == -1) {
		// if semaphore exists get the semaphore id
		semid = semget (key, 2, 0 | 0666); // 2 = anzahl
		if (semid == -1) {
			perror ("semget");
			exit (EXIT_FAILURE);
		}
	}//else{
		// if sempahore was created new initalize it
		sema.val = 0; // open
		semb.val = 1; // close
		semctl (semid, 0, SETVAL, sema); // nummer
		semctl (semid, 1, SETVAL, semb); // nummer

		puts ("semaphores created");
	//}

	// shared memory create
	shmid = shmget (key, MAXMYMEM, 0666);
	if (shmid >= 0) {
		// shared memory attach
		buf = shmat (shmid, 0, 0);
		if (buf == NULL) {
			perror ("shmat");
		} else {
			while (1) {
				// pend operation
				sb.sem_num = 1; // nummer von oben
				sb.sem_flg = SEM_UNDO;
				sb.sem_op = -1; // sb wird geschlossen nachdem ausgeführt wurde
				if (semop (semid, &sb, 1) == -1) {
					perror ("semop");
				}

				puts ("critical section");

				for (i = 0; i < MAXMYMEM; i++) {
					printf (" %d ", buf[i]);
					intbuffer [i] = buf[i];
				}
				puts ("\n");

				puts ("Read all the numbers");

				// post operation
				sa.sem_num = 0; // nummer von oben
				sa.sem_flg = SEM_UNDO;
				sa.sem_op = 1; // will sa öffnen
				if (semop (semid, &sa, 1) == -1) {
					perror ("semop");
				}

				puts ("outside critical section");



				// release access to Sa
				// Programm a initializes new set of
				// 128 int values

				count_transmission ++;
				printf ("End of get");
			}
		/*}else{
			perror("shmget");
		}*/

		}

	}
	shmdt (buf);
	// shared memory detach CAREFUL: Core Dumped Errors

	return 0;
}
