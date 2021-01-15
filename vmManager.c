#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>

#define TOTAL_ENTRIES_TLB 16 //Number entries in the Translation lookaside buffer (size of tlb)
#define TOTAL_ENTRIES_PAGETABLE 256 //Number of entries in the page table (size of page table)
#define SIZE_OF_FRAME 256 //Size of each frame - 2^8 - 8 bit offset
#define PHYSICAL_MEMORY_SIZE TOTAL_ENTRIES_PAGETABLE*SIZE_OF_FRAME //Physical memory of size 65,536 bytes (256 frames and 256 bytes size of a frame)
#define PAGE 256 //When there is a page fault
#define PAGE_NUMBER_MASK 65280
#define OFFSET_MASK 255

FILE* addresses;
FILE* BACKINGSTORE;


int TLB[TOTAL_ENTRIES_TLB][2]; //Will hold the page numbers and frame number in the tlb
int physicalMemory[PHYSICAL_MEMORY_SIZE]; //Will hold physical memory of size 65,536 bytes (256 frames and 256 bytes size of a frame)
int pageTable[TOTAL_ENTRIES_PAGETABLE]; //Will hold the page numbers in the page table
char Buffer[256]; //Holds what is read form the backing_store file

int logicalAddress = 0;
int offsetNumber = 0;
int pageNumber = 0;
int physicalAddress = 0;
int firstAvailableFrame = 0;
int Value = 0;//Will store the byte retrived from the physical memory
int frameNumber = 0;
int numberOfTLBEntries = 0;
int tlbHitCount = 0;
float tlbHitRate = 0;
int addressCount = 0;
int pageFaultCount = 0;
float pageFaultRate = 0;


void readFromBackingStore(int pageNumber) {
	fseek(BACKINGSTORE, pageNumber * PAGE, SEEK_SET);
	fread(Buffer, sizeof(char), PAGE, BACKINGSTORE);
	pageTable[pageNumber] = firstAvailableFrame;

	for (int i = 0; i < PAGE; i++)
	{
		physicalMemory[firstAvailableFrame * 256 + i] = Buffer[i];
	}
	pageFaultCount++;
	firstAvailableFrame++;

	//FIFO algorithm for the tlb
	if (numberOfTLBEntries == 16)
		numberOfTLBEntries--;

	for (int i = numberOfTLBEntries; i > 0; i--)
	{
		TLB[i][0] = TLB[i - 1][0];
		TLB[i][1] = TLB[i - 1][1];
	}

	if (numberOfTLBEntries <= 15)
		numberOfTLBEntries++;

	TLB[0][0] = pageNumber;
	TLB[0][1] = pageTable[pageNumber];
	physicalAddress = pageTable[pageNumber] * 256 + offsetNumber;
}
void getPhysicalMemory(int pageNumber) {
	//If it is in tlb, then it is tlb hit
	for (int i = 0; i < numberOfTLBEntries; i++)
	{
		if (TLB[i][0] == pageNumber)
		{
			frameNumber = TLB[i][1];
			physicalAddress = frameNumber * 256 + offsetNumber;
		}
	}
	if (!(frameNumber == -1))
	{
		tlbHitCount++;
		//printf("\nTLBHIT");
	}
	else if (pageTable[pageNumber]!=-1) {
		//printf("\nTLB MISS");
		frameNumber = pageTable[pageNumber];
	}
	else {
		//printf("\nPageFault");
		readFromBackingStore(pageNumber);
	}
}

int main(int argc, char* argv[])
{
	//Check to see if user inputs addresses.txt
	if (argc != 2)
	{
		fprintf(stderr, "Usage ./VMManager <Filename.txt> \n");
		exit(1);
	}
	//Open addresses.txt, BACKING_STORE.bin, and
	//Create Output.txt to store program results
	 addresses = fopen(argv[1], "r");
	 BACKINGSTORE = fopen("BACKING_STORE.bin", "rb");	
	 FILE* Output = fopen("Output.txt", "w");



	int Index;

	//Declare and initialize pageTable[] array to -1

	memset(pageTable, -1, 256 * sizeof(int));

	//Declare and initialize TLB[][] 2D array to -1

	memset(TLB, -1, TOTAL_ENTRIES_TLB * 2 * sizeof(TLB[0][0]));

	//Read each address from addresses.txt
	while (fscanf(addresses, "%d", &logicalAddress) == 1)
	{
		addressCount++;

		//set the page number and offset for each logical address
		pageNumber = logicalAddress & PAGE_NUMBER_MASK;
		pageNumber = pageNumber >> 8;
		offsetNumber = logicalAddress & OFFSET_MASK;
		frameNumber = -1;
		getPhysicalMemory(pageNumber);
		//Gets the value from the bin file provided
		Value = physicalMemory[physicalAddress];
		//print the addresses and value to Output.txt
		fprintf(Output, "Page Number: %d Offset: %d \n", pageNumber, offsetNumber);
		fprintf(Output, "Virtual Address: %d Physical Address: %d Value: %d \n\n", logicalAddress, physicalAddress, Value);
	}

	//The statistics of the program
	pageFaultRate = pageFaultCount * 1.0f / addressCount;
	tlbHitRate = tlbHitCount * 1.0f / addressCount;

	//Close files provided for the project
	fclose(addresses);
	fclose(BACKINGSTORE);

	//Print the statistics of the program to Output.txt
	fprintf(Output, "Number of Addresses: %d\n", addressCount);
	fprintf(Output, "Number of Page Faults: %d\n", pageFaultCount);
	fprintf(Output, "Page Fault Rate: %f\n", pageFaultRate);
	fprintf(Output, "TLB Hits: %d\n", tlbHitCount);
	fprintf(Output, "TLB Hit Rate %f\n", tlbHitRate);

	//Close Output.txt
	fclose(Output);
	//Print the statistics on the console
	printf("\nThe Translated address has been stored in the output.txt.\n");
	printf("Number of Addresses: %d\n", addressCount);
	printf("Number of Page Faults: %d\n", pageFaultCount);
	printf("Page Fault Rate: %f\n", pageFaultRate);
	printf("TLB Hits: %d\n", tlbHitCount);
	printf("TLB Hit Rate %f\n", tlbHitRate);
	return 0;

}
