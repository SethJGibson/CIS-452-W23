int numBakers;
char numInput[1];

do {
		printf("Number of bakers [1-255]: ");
		fgets(numInput, sizeof(numBakers) + 1, stdin);
		numBakers = atoi(numInput);
} while (numBakers < 1);


