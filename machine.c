/*
 * Tyler Brennan
 * CPSC275 F 2019
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>    // Imported to use tolower()
#include <string.h>   // Imported to use strcmp()

#define BITMAX 127  // Positive and negative ranges for values
#define BITMIN -128
#define MEMLEN 8
#define REGLEN 4
#define FILELEN 100  // Hopefully the file is shorter than this
#define INPUT 50      // Can take in jump statements instead of integers or mem/reg spaces so it should be a long array

typedef struct{
  int index;
  char label[INPUT];
} jump;

int OOB(int input, int *OF);   // Takes in an int and checks if it is out of the range
int readInput(int *mem, int *reg, char param1[], char param2[], int *valS, int *valD);
void setFlags(int *SF, int *ZF, int val);

void read(int *mem, char param1[], char param2[], int *end, int *OF);       // Input data into m (OF if out of range)
int write(int *mem, int *reg, char param[]);   // Output is the value stored at mem/reg
void move(int *mem, int *reg, char param1[], char param2[], int *end);    // Movl ~ cannot move mem to mem
void add(int *mem, int *reg, char param1[], char param2[], int *end, int *OF, int *SF, int *ZF);     // Addl ~ sum stored in R
void sub(int *mem, int *reg, char param1[], char param2[], int *end, int *OF, int *SF, int *ZF);     // Subl ~ result stored in R (M-R)
void mult(int *mem, int *reg, char param1[], char param2[], int *end, int *OF, int *SF, int *ZF);    // imul ~ result stored in R
void divide(int *mem, int *reg, char param1[], char param2[], int *end, int *SF, int *ZF);     // idivl ~ result stored in R (M/R)
void mod(int *mem, int *reg, char param1[], char param2[], int *end, int *SF, int *ZF);     // idivl ~ result stored in R (M%R)
void comp(int *mem, int *reg, char param1[], char param2[], int *end, int *SF, int *ZF);    // R2-R1 and sets flags based off the result
void prints(int *mem, int *reg, int *ZF, int *SF, int *OF);

void jumpPt(jump *jumpSt, char input[], int jmpIndex, int i);       // Reads in jump points to jump array
void jumpTo(jump jumpSt[], char input[], char param[], int jmpLen, int *end, int *i, int ZF, int SF); // Compares and jumps if needed

void main(void){
  jump jumpSt[FILELEN];
  char instruction[INPUT][FILELEN];  // Will hold all the instructions
  int mem[MEMLEN], reg[REGLEN]; // Stores register and memory values
  int ZF, SF, OF;     // Three flags
  int i, j, c, n, count, temp, jmpIndex, start, end;
  char input[INPUT], param1[INPUT], param2[INPUT], tempStr[INPUT];

  i = n = jmpIndex = end = 0;
  start = -1;
  while((c = getchar()) != EOF){  // Read in entire file
    if(c == '\n'){
      for(i = i; i < INPUT; i++) // This is to clear the rest of the str to fix an error
        instruction[n][i] = '\0';      // There was an error where it would put characters after the command
      n++;
      i = 0;
    }
    else if(c == '#'){
      j = i;
      while((c = getchar()) != '\n');
      for(i = i; i < INPUT; i++) // This is to clear the rest of the str to fix an error
        instruction[n][i] = '\0';      // There was an error where it would put characters after the command
      if(j != 0)  // If the '#' is the first character, it will not save an empty string
        n++;
      i = 0;
    }
    else{
      instruction[n][i] = tolower(c);
      i++;
    }
  }
  ZF = SF = OF = 0;
  for(i = 0; i < REGLEN; i++)   // Set all registers to 1 + BITMAX
    reg[i] = BITMAX + 1;        // This helps prints() output '?' for empty registers
  for(i = 0; i < MEMLEN; i++)   // Do the same with memory space
    mem[i] = BITMAX + 1;
  for(i = 0; i < n; i++){       // Reads through array and scans every jump label to jump struct
    count = 0;
    for(j = 0; j < INPUT; j++){ // Starts by reading each first command into a string
      if(instruction[i][j] != ' ' && count == 0){
        input[j] = instruction[i][j];
      }
      else if(instruction[i][j] == ' ' && count == 0){
        input[j] = '\0';
        count++;
      }
    }
    if(!strcmp(input, ".start:")) // Checks to see start point
      start = i;
    if(input[0] == '.' && input[strlen(input)-1] == ':')  // Checks all jump labels
      jumpPt(jumpSt, input, jmpIndex++, i);
  }
  if(start == -1)     // If there is no .start: then don't run
    return;
  for(i = start; i < n; i++){   // Loops from the '.start:' line number
    count = 0;
    for(j = 0; j < INPUT; j++){
      if(instruction[i][j] != ' ' && count == 0)
        input[j] = instruction[i][j];
      else if(instruction[i][j] == ' ' && count == 0){
        input[j] = '\0';
        count++;
        temp = 0;
      }
      else if(instruction[i][j] != ' ' && count == 1)
        param1[temp++] = instruction[i][j];
      else if(instruction[i][j] == ' ' && count == 1){
        param1[temp] = '\0';
        count++;
        temp = 0;
      }
      else if(instruction[i][j] != ' ' && count == 2)
        param2[temp++] = instruction[i][j];
      else if(instruction[i][j] == ' ' && count == 2){
        param2[temp] = '\0';
        count++;
        temp = 0;
      }
      else if(instruction[i][j] != ' ' && count == 3)
        tempStr[temp++] = instruction[i][j];
      else if(instruction[i][j] == ' ' && count == 3)
        tempStr[temp] = '\0';
    }
    if(input[0] == '.'){        // If there is a label to start a statement, move all the parameters back
      strcpy(input, param1);
      strcpy(param1, param2);
      strcpy(param2, tempStr);
      tempStr[0] = '.';   // Will be used to determine if this line only held a single jump statement without commands
    }
    if (strcmp(input, "read") == 0)
      read(&mem[0], param1, param2, &end, &OF);
    else if (strcmp(input, "write") == 0){
      n = write(&mem[0], &reg[0], param1);    // i is used to store the value stored in reg/mem
      tempStr[0] = '.';
      if(n == BITMAX+1)
        printf("?\n\n");
      else
        printf("%d\n\n", n);
    }
    else if (strcmp(input, "move") == 0)
      move(&mem[0], &reg[0], param1, param2, &end);
    else if (strcmp(input, "add") == 0)
      add(&mem[0], &reg[0], param1, param2, &end, &OF, &SF, &ZF);
    else if (strcmp(input, "sub") == 0)
      sub(&mem[0], &reg[0], param1, param2, &end, &OF, &SF, &ZF);
    else if (strcmp(input, "mult") == 0)
      mult(&mem[0], &reg[0], param1, param2, &end, &OF, &SF, &ZF);
    else if (strcmp(input, "div") == 0){
      divide(&mem[0], &reg[0], param1, param2, &end, &SF, &ZF);
      OF = 0;
    }
    else if (strcmp(input, "mod") == 0){
      mod(&mem[0], &reg[0], param1, param2, &end, &SF, &ZF);
      OF = 0;
    }
    else if (strcmp(input, "comp") == 0)
      comp(&mem[0], &reg[0], param1, param2, &end, &SF, &ZF);
    else if (strcmp(input, "prints") == 0){
      prints(&mem[0], &reg[0], &ZF, &SF, &OF);
      OF = 0;
    }
    else if(strcmp(input, "quit") == 0){
      return;
    }
    else if(input[0] == '.' && input[strlen(input)-1] == ':'){}  // Should not print ??? if reads a jump point
    else if(input[0] == 'j')
      jumpTo(jumpSt, input, param1, jmpIndex++, &end, &i, ZF, SF);
    else if (tempStr[0] != '.' || OF == 1 || end == 1){   // Ends program if there is an error
      printf("???\n\n");
      return;
    }
    for(n = 0; n < INPUT; n++){
      input[n] = param1[n] = param2[n] = tempStr[n] = '\0';
    }
  }
}

/*
 * OOB checks if an integer input is within the proper range and sets the
 * overflow flag accordingly
 *
 * IN: input = the integer that was entered
 *     *OF = Pointer to the overflow flag in the ISAData struct
 * OUT: int = returns 1 if the number is out of the range
 */
int OOB(int input, int *OF){
  if(input > BITMAX || input < BITMIN){
    *OF = 1;
    return 1;
  }
  else{
    *OF = 0;
    return 0;
  }
}

/*
 * readInput is used when the command takes in two registers or a memory
 * location and a register, and reads the corresponding values into the struct.
 * Will end without proper syntax/if two memory locations are entered
 *
 * IN: *mem = pointer to the first index of mem (uses addition to get to other indexes)
 *     *reg = pointer to the first index of reg (uses addition to get to other indexes)
 *     param1[] = The source register/memory
 *     param2[] = The destination register/memory
 *     *valS = the value at the indicated source array + index
 *     *valD = the value at the indicated destination array + index
 * OUT: int = return 0 if two memory locations are read/if no comma is used or/index is OOB
 */
int readInput(int *mem, int *reg, char param1[], char param2[], int *valS, int *valD){
  if(param1[2] != ',')
    return 0;
  if(param1[0] == 'm' && param2[0] == 'm')  // Sets 0 for errors (like mem -> mem)
    return 0;
  else if(param1[0] == 'r' && param2[0] == 'm'){ // Sets valS/valD to values at array indexes
    if((param1[1] - '0') > 3 || (param2[1] - '0') > 7)
      return 0;
    *valS = *(reg+(param1[1] - '0'));
    *valD = *(mem+(param2[1] - '0'));
  }
  else if(param1[0] == 'm' && param2[0] == 'r'){
    if((param1[1] - '0') > 7 || (param2[1] - '0') > 3)
      return 0;
    *valS = *(mem+(param1[1] - '0'));
    *valD = *(reg+(param2[1] - '0'));
  }
  else if(param1[0] == 'r' && param2[0] == 'r'){
    if((param1[1] - '0') > 3 || (param2[1] - '0') > 3)
      return 0;
    *valS = *(reg+(param1[1] - '0'));
    *valD = *(reg+(param2[1] - '0'));
  }
  else
    return 0;
  return 1;
}

/*
 * setFlags sets ZF, SF, and OF based off of the value given
 *
 * IN: *SF = the sign flag that will set if the value is negative
 *     *ZF = the zero flag that will set if the value is 0
 *     val = the int that the flags will be set off of
 */
void setFlags(int *SF, int *ZF, int val){
   if(val > 0){
     *SF = 0;
     *ZF = 0;
   }
   else if(val < 0){
     *SF = 1;
     *ZF = 0;
   }
   else{
     *SF = 0;
     *ZF = 1;
   }
 }

/*
 * read takes in one integer value and a memory location/register to store that
 * value. It checks if the value is within
 *
 * IN: *mem = pointer to the first index of mem (uses addition to get to other indexes)
 *     param1[] = The number to be read into memory
 *     param2[] = The destination memory
 *     *end = used to end the program if there is an error
 *     *OF = pointer to the overflow flag
 */
void read(int *mem, char param1[], char param2[], int *end, int *OF){
  int i, n, data;        // Indexes for the dest in the mem array + the input
  n = strlen(param1);
  if(param1[n-1] != ','){
    *end = 1;
    return;
  }
  if(OOB((int)strtol(param1, NULL, 10), OF)){
    return;
  }
  if(param2[0] == 'm'){
    if((param2[1] - '0') > 7){
      *end = 1;
      return;
    }
    *(mem+(param2[1] - '0')) = (int)strtol(param1, NULL, 10);
  }
  else if(param2[0] == 'r')
    *end = 1;
  else
    *end = 1;
}

/*
 * write just simply prints out the value stored at the entered registers
 *
 * IN: *mem = pointer to the first index of mem (uses addition to get to other indexes)
 *     *reg = pointer to the first index of reg (uses addition to get to other indexes)
 *     param[] = The register/memory that will be written
 * OUT: int = the value stored at the inputted register/memory space
 */
int write(int *mem, int *reg, char param[]){
  int i, error = BITMAX+1;  // Index for the source of the value, and an error return value
  char source;  // Holds the character that indicates either mem or reg
  i = (param[1] - '0');
  if(param[0] == 'm'){
    if((param[1] - '0') > 7)
      return error;
    return *(mem+i);
  }
  else if(param[0] == 'r'){
    if((param[1] - '0') > 3)
      return error;
    return *(reg+i);
  }
  return error;
}

/*
 * move takes the value at the source and copies it to the destination
 *
 * IN: *mem = pointer to the first index of mem (uses addition to get to other indexes)
 *     *reg = pointer to the first index of reg (uses addition to get to other indexes)
 *     param1[] = The source register/memory
 *     param2[] = The destination register/memory
 *     *end = used to end the program if there is an error
 */
void move(int *mem, int *reg, char param1[], char param2[], int *end){
  int i, n, valS, valD;   // Index for the source and dest in the arrays and the values there
  n = strlen(param1);
  if(param1[n-1] != ','){
    *end = 1;
    return;
  }
  param1[n] = '\0';
  if(param1[0] == 'm' && param2[0] == 'm'){ // Sets 0 for errors (like mem -> mem)
    *end = 1;
    return;
  }
  else if(param1[0] == 'r' && param2[0] == 'm'){ // Sets valS/valD to values at array indexes
    if((param1[1] - '0') > 3 || (param2[1] - '0') > 7){
      *end = 1;
      return;
    }
    valS = *(reg+(param1[1] - '0'));
  }
  else if(param1[0] == 'm' && param2[0] == 'r'){
    if((param1[1] - '0') > 7 || (param2[1] - '0') > 3){
      *end = 1;
      return;
    }
    valS = *(mem+(param1[1] - '0'));
  }
  else if(param1[0] == 'r' && param2[0] == 'r'){
    if((param1[1] - '0') > 3 || (param2[1] - '0') > 3){
      *end = 1;
      return;
    }
    valS = *(reg+(param1[1] - '0'));
  }
  else{
    *end = 1;
    return;
  }
  if(param2[0] == 'm')
    *(mem+(param2[1] - '0')) = valS;
  else if(param2[0] == 'r')
    *(reg+(param2[1] - '0')) = valS;
}

/*
 * add = source + destination
 * sets flags based off of the result
 *
 * IN: *mem = pointer to the first index of mem (uses addition to get to other indexes)
 *     *reg = pointer to the first index of reg (uses addition to get to other indexes)
 *     param1[] = The source register/memory
 *     param2[] = The destination register/memory
 *     *end = used to end the program if there is an error
 *     *OF = pointer to the overflow flag
 *     *SF = pointer to the sign flag
 *     *ZF = pointer to the zero flag
 */
void add(int *mem, int *reg, char param1[], char param2[], int *end, int *OF, int *SF, int *ZF){
  int valS, valD, sum;   // The values at source[i] & destination[i] and sum
  int error = readInput(mem, reg, param1, param2, &valS, &valD);
  sum = valS + valD;
  if(!error){
    *end = 1;
    return;
  }
  else if(OOB(sum, OF)){
    *reg = BITMAX+1;
    return;
  }
  setFlags(SF, ZF, sum);
  *reg = sum;
}

/*
 * sub = destination - source
 * sets flags based off of the result
 *
 * IN: *mem = pointer to the first index of mem (uses addition to get to other indexes)
 *     *reg = pointer to the first index of reg (uses addition to get to other indexes)
 *     param1[] = The source register/memory
 *     param2[] = The destination register/memory
 *     *end = used to end the program if there is an error
 *     *OF = pointer to the overflow flag
 *     *SF = pointer to the sign flag
 *     *ZF = pointer to the zero flag
 */
void sub(int *mem, int *reg, char param1[], char param2[], int *end, int *OF, int *SF, int *ZF){
  int valS, valD, result;   // The values at source[i] & destination[i] and sum
  int error = readInput(mem, reg, param1, param2, &valS, &valD);
  result = valD - valS;
  if(!error || valS == (BITMAX+1) || valD == (BITMAX+1)){
    *end = 1;
    *reg = BITMAX+1;
    return;
  }
  else if(OOB(result, OF)){
    *reg = BITMAX+1;
    return;
  }
  setFlags(SF, ZF, result);
  *reg = result;
}

/*
 * mult = source * destination
 * sets flags based off of the result
 *
 * IN: *mem = pointer to the first index of mem (uses addition to get to other indexes)
 *     *reg = pointer to the first index of reg (uses addition to get to other indexes)
 *     param1[] = The source register/memory
 *     param2[] = The destination register/memory
 *     *end = used to end the program if there is an error
 *     *OF = pointer to the overflow flag
 *     *SF = pointer to the sign flag
 *     *ZF = pointer to the zero flag
 */
void mult(int *mem, int *reg, char param1[], char param2[], int *end, int *OF, int *SF, int *ZF){
  int valS, valD, result;   // The values at source[i] & destination[i] and sum
  int error = readInput(mem, reg, param1, param2, &valS, &valD);
  result = valS * valD;
  if(!error){
    *end = 1;
    *reg = BITMAX+1;
    return;
  }
  else if(OOB(result, OF)){
    *reg = BITMAX+1;
    return;
  }
  setFlags(SF, ZF, result);
  *reg = result;
}

/*
 * divide = source / destination
 * sets flags based off of the result
 *
 * IN: *mem = pointer to the first index of mem (uses addition to get to other indexes)
 *     *reg = pointer to the first index of reg (uses addition to get to other indexes)
 *     param1[] = The source register/memory
 *     param2[] = The destination register/memory
 *     *end = used to end the program if there is an error
 *     *SF = pointer to the sign flag
 *     *ZF = pointer to the zero flag
 */
void divide(int *mem, int *reg, char param1[], char param2[], int *end, int *SF, int *ZF){
  int valS, valD, result;   // The values at source[i] & destination[i] and sum
  int error = readInput(mem, reg, param1, param2, &valS, &valD);
  result = valS / valD;
  if(!error || valS == (BITMAX+1) || valD == (BITMAX+1)){
    *end = 1;
    *reg = BITMAX+1;
    return;
  }
  setFlags(SF, ZF, result);
  *reg = result;
}

/*
 * mod = source % destination
 * sets flags based off of the result
 *
 * IN: *mem = pointer to the first index of mem (uses addition to get to other indexes)
 *     *reg = pointer to the first index of reg (uses addition to get to other indexes)
 *     param1[] = The source register/memory
 *     param2[] = The destination register/memory
 *     *end = used to end the program if there is an error
 *     *SF = pointer to the sign flag
 *     *ZF = pointer to the zero flag
 */
void mod(int *mem, int *reg, char param1[], char param2[], int *end, int *SF, int *ZF){
  int valS, valD, result;   // The values at source[i] & destination[i] and sum
  int error = readInput(mem, reg, param1, param2, &valS, &valD);
  result = valS % valD;
  if(!error || valS == (BITMAX+1) || valD == (BITMAX+1)){
    *end = 1;
    *reg = BITMAX+1;
    return;
  }
  setFlags(SF, ZF, result);
  *reg = result;
}

/*
 * comp sets ZF, SF, and OF based off of the two memory/registers - does three
 * comparitsons in order to see if it is negative or equal to zero
 *
 * IN: *mem = pointer to the first index of mem (uses addition to get to other indexes)
 *     *reg = pointer to the first index of reg (uses addition to get to other indexes)
 *     param1[] = The source register/memory
 *     param2[] = The destination register/memory
 *     *end = used to end the program if there is an error
 *     *SF = pointer to the sign flag
 *     *ZF = pointer to the zero flag
 */
void comp(int *mem, int *reg, char param1[], char param2[], int *end, int *SF, int *ZF){
  int valS, valD, result;   // The values at source[i] & destination[i] and sum
  int error = readInput(mem, reg, param1, param2, &valS, &valD);
  if(!error){
    *end = 1;
    return;
  }
  if(valS > valD){
    *SF = 1;
    *ZF = 0;
  }
  else if(valS < valD){
    *SF = 0;
    *ZF = 0;
  }
  else{
    *SF = 0;
    *ZF = 1;
  }
}

/*
 * prints goes through each register and memory space to print and format
 * everything (if there is no value in them, a '?' is printed)
 *
 * IN: *mem = pointer to the first index of mem (uses addition to get to other indexes)
 *     *reg = pointer to the first index of reg (uses addition to get to other indexes)
 *     *OF = pointer to the overflow flag
 *     *SF = pointer to the sign flag
 *     *ZF = pointer to the zero flag
 */
void prints(int *mem, int *reg, int *ZF, int *SF, int *OF){
  int i;
  for(i = 0; i < REGLEN; i++){
    if(*(reg+i) == BITMAX+1)
      printf("?\t");
    else
      printf("%d\t", *(reg+i));
  }
  for(i = 0; i < MEMLEN; i++){
    if(*(mem+i) == BITMAX+1)
      printf("?\t");
    else
      printf("%d\t", *(mem+i));
  }
  printf("%d\t%d\t%d", *ZF, *SF, *OF);
  printf("\n--\t--\t--\t--\t--\t--\t--\t--\t--\t--\t--\t--\t--\t--\t--\n");
  printf("R0\tR1\tR2\tR3\tM0\tM1\tM2\tM3\tM4\tM5\tM6\tM7\tZF\tSF\tOF\n\n");
}

/*
 * Reads that there is a jump statement and stores it in the array of jump structs
 *
 * IN: *jumpSt = Array of jump statements that will be adjusted with new labels and Indexes
 *     input[] = Holds the label for the array of jump structs
 *     jmpIndex = Tells the function where to place the new jump information
 *     i = Index of the overarching for loop that will be saved in the array
 */
void jumpPt(jump *jumpSt, char input[], int jmpIndex, int i){
  int n;
  n = strlen(input);
  input[n-1] = '\0';
  (jumpSt + jmpIndex)->index = i;
  strcpy((jumpSt + jmpIndex)->label, input);
}

/*
 * Reads the jump comparison and decides if it needs to jump to the stated jump statement
 * using the sign and zero flags
 *
 * IN: jumpSt[] = Array of jump structs to find what index to jump to
 *     input[] = Array that should determine the jump comparison
 *     param[] = Array that holds the jump label
 *     jmpLen = Length of the jump struct array
 *     *end = used to end the program if there is an error
 *     *i = Index of the overarching for loop of the program that will be adjusted
 *     ZF = Flag used to compare
 *     SF = Flag used to compare
 */
void jumpTo(jump jumpSt[], char input[], char param[], int jmpLen, int *end, int *i, int ZF, int SF){
  int j, index, error;
  for(j = 0; j < jmpLen; j++)
    if(!strcmp(jumpSt[j].label, param)){
      index = j;
      error = 1;
    }
  if(!error)
    *end = 1;
  else if(!strcmp(input, "jmp"))
    *i = jumpSt[index].index;
  else if(!strcmp(input, "jne")){
    if(ZF == 0)
      *i = jumpSt[index].index;
    else
      return;
  }
  else if(!strcmp(input, "jg")){
    if(ZF == 0 && SF == 0)
      *i = jumpSt[index].index;
    else
      return;
  }
  else if(!strcmp(input, "jge")){
    if(SF == 0)
      *i = jumpSt[index].index;
    else
      return;
  }
  else if(!strcmp(input, "jl")){
    if(ZF == 0 && SF == 1)
      *i = jumpSt[index].index;
    else
      return;
  }
  else if(!strcmp(input, "jle")){
    if(SF == 1)
      *i = jumpSt[index].index;
    else
      return;
  }
  else
    *end = 1;
}
