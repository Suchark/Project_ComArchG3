#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAXLINELENGTH 1000
#define SIZE 100

int readAndParse(FILE *, char *, char *, char *, char *, char *);
int isNumber(char *);
int convertNum(int num);

int hashCode(int key);
struct address *search(int key);
void insert(int key, char val[MAXLINELENGTH]);
struct address* delete(struct address* item);
void displayTable();

struct address
{
    int key;
    char val[MAXLINELENGTH];
};

struct address* hashArray[SIZE]; 
struct address* dummyItem;
struct address* item;

int main(int argc, char *argv[])
{
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];
    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",argv[0]);
        exit(1);
    }

    dummyItem = (struct address*) malloc(sizeof(struct address));
    dummyItem->val[MAXLINELENGTH] = -1;  
    dummyItem->key = -1; 

    inFileString = argv[1];
    outFileString = argv[2];

    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL) {
        printf("error in opening inFileStrin %s\n", inFileString);
        exit(1);
    }
    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL) {
        printf("error in opening outFileString %s\n", outFileString);
        exit(1);
    }

    /* here is an example for how to use readAndParse to read a line from
        inFilePtr */
    // if (! readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2) ) {
    //     /* reached end of file */
    // }

    // keep key .fill
    int i = 0;
    char temp[MAXLINELENGTH] = "x";
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2))
    {
        sprintf(temp, "%s", label);
        insert(i,temp);
        i++;
    }
    /* this is how to rewind the file ptr so that you start reading from the
        beginning of the file */
    rewind(inFilePtr);

    int dex = 0b00000000000000000000000000000000;
    int twoCom = 0b11111111111111111111111111111111;
    int now = 0;
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2))
    {
        dex = 0b00000000000000000000000000000000;
        twoCom = 0b0000000000000000;
        if (!strcmp(opcode, "add")) {
            dex |= 0b000 << 22;
            if(isNumber(arg0)&&isNumber(arg1)&&isNumber(arg2)){
                dex |= atoi(arg0) << 19;
                dex |= atoi(arg1) << 16;
                dex |= atoi(arg2) << 0;
                
            } 
        }else if(!strcmp(opcode, "nand")){
            dex |= 0b001 << 22;
            if(isNumber(arg0)&&isNumber(arg1)&&isNumber(arg2)){
                dex |= atoi(arg0) << 19;
                dex |= atoi(arg1) << 16;
                dex |= atoi(arg2) << 0;
            }
        }else if(!strcmp(opcode, "lw")){
            dex |= 0b010 << 22;
            if(isNumber(arg0)&&isNumber(arg1)){
                dex |= atoi(arg0) << 19;
                dex |= atoi(arg1) << 16;
            }
            if (isNumber(arg2))
            {
                dex |= atoi(arg2) << 0;
            }else{
                for (int j = 0; j < SIZE; j++)
                {
                    if(!strcmp(arg2, hashArray[j]->val)){
                        dex |= hashArray[j]->key << 0;
                        break;
                    }
                }
            }
        }else if(!strcmp(opcode, "sw")){
            dex |= 0b011 << 22;
            if(isNumber(arg0)&&isNumber(arg1)){
                dex |= atoi(arg0) << 19;
                dex |= atoi(arg1) << 16;
            }
            if (isNumber(arg2))
            {
                dex |= atoi(arg2) << 0;
            }else{
                for (int j = 0; j < SIZE; j++)
                {
                    if(!strcmp(arg2, hashArray[j]->val)){
                        dex |= hashArray[j]->key << 0;
                        break;
                    }
                }
            }
        }else if(!strcmp(opcode, "beq")){
            dex |= 0b100 << 22;
            if(isNumber(arg0)&&isNumber(arg1)){
                dex |= atoi(arg0) << 19;
                dex |= atoi(arg1) << 16;
            }
            if (isNumber(arg2))
            {
                dex |= atoi(arg2) << 0;
            }else{
                for (int j = 0; j < SIZE; j++)
                {
                    if(!strcmp(arg2, hashArray[j]->val)){
                        if(now > hashArray[j]->key){
                            twoCom = (1 << 16) + (~(hashArray[j]->key+1)+1);
                            dex |= twoCom;
                        }else{
                            dex |= hashArray[j]->key << 0;
                        }
                        break;
                    }
                }
            }
        }else if(!strcmp(opcode, "jalr")){
            dex |= 0b101 << 22;
            if(isNumber(arg0)&&isNumber(arg1)){
                dex |= atoi(arg0) << 19;
                dex |= atoi(arg1) << 16;
            }
        }else if(!strcmp(opcode, "halt")){
            dex |= 0b110 << 22;
        }else if(!strcmp(opcode, "noop")){
            dex |= 0b111 << 22;
        }else if(!strcmp(opcode, ".fill")){
            if(isNumber(arg0)){
                dex |= atoi(arg0) << 0;
            }else{
                for (int k = 0; k < SIZE; k++)
                {
                    if(!strcmp(arg0, hashArray[k]->val)){
                        dex |= hashArray[k]->key << 0;
                        break;
                    }
                }
            }
        }
        fprintf(outFilePtr,"%d\n",dex);
        now++;
    }

    /* after doing a readAndParse, you may want to do the following to test the
        opcode */
    if (!strcmp(opcode, "add")) {
        /* do whatever you need to do for opcode "add" */
    }

    return(0);
}

/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
    char *arg1, char *arg2)
{
    char line[MAXLINELENGTH];
    char *ptr = line;
    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

    /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
	/* reached end of file */
        return(0);
    }
    /* check for line too long (by looking for a \n) */
    if (strchr(line, '\n') == NULL) {
        /* line too long */
	    printf("error: line too long\n");
	    exit(1);
    }

    /* is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n ]", label)) {
	/* successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n ]%[^\t\n ]%*[\t\n ]%[^\t\n ]%*[\t\n ]%[^\t\n ]%*[\t\n ]%[^\t\n ]",
        opcode, arg0, arg1, arg2);
    return(1);
}

int isNumber(char *string)
{
    /* return 1 if string is a number */
    int i;
    return( (sscanf(string, "%d", &i)) == 1);
}

int hashCode(int key){
    return key % SIZE;
}



struct address *search(int key){
    //get the hash
    int hashIndex = hashCode(key);

    //move in array untill an empty
    while (hashArray[hashIndex] != NULL){
        if (hashArray[hashIndex]->key == key){
            return hashArray[hashIndex];
        }

        ++hashIndex;

        hashIndex %= SIZE;
        
    }
    return NULL;
}

void insert(int key,char val[MAXLINELENGTH]){
    struct address *item = (struct address*) malloc(sizeof(struct address));
     
    strcpy( item->val, val);
    item->key = key;

    //get the hash 
    int hashIndex = hashCode(key);

    //move in array until an empty or deleted cell
    while(hashArray[hashIndex] != NULL && hashArray[hashIndex]->key != -1) {
        //go to next cell
        ++hashIndex;
		
        //wrap around the table
        hashIndex %= SIZE;
    }

    hashArray[hashIndex] = item; 
    
}

struct address* delete(struct address* item) {
   int key = item->key;

   //get the hash 
   int hashIndex = hashCode(key);

   //move in array until an empty 
   while(hashArray[hashIndex] !=NULL) {
	
      if(hashArray[hashIndex]->key == key) {
         struct address* temp = hashArray[hashIndex]; 
			
         //assign a dummy item at deleted position
         hashArray[hashIndex] = dummyItem; 
         return temp;
      } 
		
      //go to next cell
      ++hashIndex;
		
      //wrap around the table
      hashIndex %= SIZE;
   }  
	
   return NULL;        
}

void displayTable() {
   int i = 0;
	
   for(i = 0; i<SIZE; i++) {
	
      if(hashArray[i] != NULL)
         printf(" (%d,%s)",hashArray[i]->key,hashArray[i]->val);
      else
         printf(" ~~ ");
   }
	
   printf("\n");
}

int convertNum(int num){
    if (num & (1<<15))
    {
        num -= (1<<16);
    }
    return num;
}