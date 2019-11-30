/*
 * Tyler Brennan
 * CPSC275 F 2019
 */

#include <stdio.h>
#include <ctype.h>    // Imported to use tolower()
#include <string.h>   // Imported to use strcmp()

#define BITMAX 127  // Positive and negative ranges for values
#define BITMIN -128
#define MAXLEN 7    // Length for method calls (longest is prints at 6 characters + '\0')
#define MEMLEN 8
#define REGLEN 4

int OOB(int input, int *OF);   // Takes in an int and checks if it is out of the range
int readInput(int *mem, int *reg, int *valS, int *valD);
void setFlags(int *SF, int *ZF, int val);

void read(int *mem, int *OF);       // Input data into m (OF if out of range)
int write(int *mem, int *reg);   // Output is the value stored at mem/reg
void move(int *mem, int *reg);    // Movl ~ cannot move mem to mem
void add(int *mem, int *reg, int *OF, int *SF, int *ZF);     // Addl ~ sum stored in R
void sub(int *mem, int *reg, int *OF, int *SF, int *ZF);     // Subl ~ result stored in R (M-R)
void mult(int *mem, int *reg, int *OF, int *SF, int *ZF);    // imul ~ result stored in R
void div(int *mem, int *reg, int *SF, int *ZF);     // idivl ~ result stored in R (M/R)
void mod(int *mem, int *reg, int *SF, int *ZF);     // idivl ~ result stored in R (M%R)
void comp(int *mem, int *reg, int *SF, int *ZF);    // R2-R1 and sets flags based off the result
void prints(int *mem, int *reg, int *ZF, int *SF, int *OF);

void main(void){
  int mem[MEMLEN], reg[REGLEN]; // Stores register and memory values
  int ZF, SF, OF;     // Three flags
  int iS, iD;         // The int in r1, r0, m5, etc. for the array index
  char source, dest;  // Should read in either m or r for which array to use
  int i;
  char input[MAXLEN] = "";

  ZF = SF = OF = 0;
  for(i = 0; i < REGLEN; i++)   // Set all registers to 1 + BITMAX
    reg[i] = BITMAX + 1;        // This helps prints() output '?' for empty registers
  for(i = 0; i < MEMLEN; i++)   // Do the same with memory space
    mem[i] = BITMAX + 1;

  printf("SM$ ");
  scanf(" %s", &input[0]);      // Read in the first command
  for (i = 0; i < MAXLEN; i++)  // Use tolower to get a lowercase word
    input[i] = tolower(input[i]);
  while(strcmp(input, "quit") != 0){
    if (strcmp(input, "read") == 0)
      read(&mem[0], &OF);
    else if (strcmp(input, "write") == 0){
      i = write(&mem[0], &reg[0]);    // i is used to store the value stored in reg/mem
      if(i == BITMAX+1)
        printf("?\n");
      else
        printf("%d\n", i);
    }
    else if (strcmp(input, "move") == 0)
      move(&mem[0], &reg[0]);
    else if (strcmp(input, "add") == 0)
      add(&mem[0], &reg[0], &OF, &SF, &ZF);
    else if (strcmp(input, "sub") == 0)
      sub(&mem[0], &reg[0], &OF, &SF, &ZF);
    else if (strcmp(input, "mult") == 0)
      mult(&mem[0], &reg[0], &OF, &SF, &ZF);
    else if (strcmp(input, "div") == 0){
      div(&mem[0], &reg[0], &SF, &ZF);
      OF = 0;
    }
    else if (strcmp(input, "mod") == 0){
      mod(&mem[0], &reg[0], &SF, &ZF);
      OF = 0;
    }
    else if (strcmp(input, "comp") == 0)
      comp(&mem[0], &reg[0], &SF, &ZF);
    else if (strcmp(input, "prints") == 0){
      prints(&mem[0], &reg[0], &ZF, &SF, &OF);
      OF = 0;
    }
    else
      printf("???\n");
    if(OF == 1)
      printf("???\n");
    printf("SM$ ");
    scanf(" %s", &input[0]);
    for (i = 0; i < MAXLEN; i++)
      input[i] = tolower(input[i]);
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
 *     *valS = the value at the indicated source array + index
 *     *valD = the value at the indicated destination array + index
 * OUT: int = return 0 if two memory locations are read/if no comma is used or/index is OOB
 */
int readInput(int *mem, int *reg, int *valS, int *valD){
  int iS, iD;   // Holds the index for the source and destination in the arrays
  char source, dest;  // Holds the character that indicates either mem or reg
  char comma;   // If input is correct, should scan in a ','
  scanf(" %c%d %c", &source, &iS, &comma);
  if(comma != ','){         // Invalid syntax
    scanf("%d", &iD);  // Clear the input
    return 0;
  }
  scanf(" %c%d", &dest, &iD);  // this should be m or r with the following number
  source = tolower(source);
  dest = tolower(dest);
  if(source == 'm' && dest == 'm')  // Sets 0 for errors (like mem -> mem)
    return 0;
  else if(source == 'r' && dest == 'm'){ // Sets valS/valD to values at array indexes
    if(iS > 3 || iD > 7)
      return 0;
    *valS = *(reg+iS);
    *valD = *(mem+iD);
  }
  else if(source == 'm' && dest == 'r'){
    if(iS > 7 || iD > 3)
      return 0;
    *valS = *(mem+iS);
    *valD = *(reg+iD);
  }
  else if(source == 'r' && dest == 'r'){
    if(iS > 3 || iD > 3)
      return 0;
    *valS = *(reg+iS);
    *valD = *(reg+iD);
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
 *     *OF = pointer to the overflow flag
 */
void read(int *mem, int *OF){
  int i, data;        // Indexes for the dest in the mem array + the input
  char dest;  // Holds the character that indicates either mem or reg
  char comma;
  scanf("%d %c", &data, &comma);  // This should get the int input and the ','
  if(comma != ','){
    scanf("%d", &i);  // Clears out the input (final int)
    printf("???\n");
    return;
  }
  else if(OOB(data, OF)){
    scanf(" %c%d", &dest, &i);  // Clears out the input
    return;
  }
  scanf(" %c%d", &dest, &i);  // this should be m/r with the index
  if(tolower(dest) == 'm'){
    if(i > 7){
      printf("???\n");
      return;
    }
    *(mem+i) = data;
  }
  else if(tolower(dest) == 'r')
    printf("???\n");
  else
    printf("???\n");
}

/*
 * write just simply prints out the value stored at the entered registers
 *
 * IN: *mem = pointer to the first index of mem (uses addition to get to other indexes)
 *     *reg = pointer to the first index of reg (uses addition to get to other indexes)
 * OUT: int = the value stored at the inputted register/memory space
 */
int write(int *mem, int *reg){
  int i, error = BITMAX+1;  // Index for the source of the value, and an error return value
  char source;  // Holds the character that indicates either mem or reg
  scanf(" %c%d", &source, &i);  // this should be m or r with the following number
  if(tolower(source) == 'm'){
    if(i > 7)
      return error;
    return *(mem+i);
  }
  else if(tolower(source) == 'r'){
    if(i > 3)
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
 */
void move(int *mem, int *reg){
  int iS, iD, valS;   // Index for the source and dest in the arrays and the values there
  char source, dest;  // Holds the character that indicates either mem or reg
  char comma;   // If input is correct, should scan in a ','
  scanf(" %c%d %c", &source, &iS, &comma);
  if(comma != ','){         // Invalid syntax
    scanf("%d", &iD);  // Clear the input
    printf("???\n");
    return;
  }
  scanf(" %c%d", &dest, &iD);  // this should be m or r with the following number
  source = tolower(source);
  dest = tolower(dest);
  if(source == 'm' && dest == 'm'){ // Sets 0 for errors (like mem -> mem)
    printf("???\n");
    return;
  }
  else if(source == 'r' && dest == 'm'){ // Sets valS/valD to values at array indexes
    if(iS > 3 || iD > 7){
      printf("???\n");
      return;
    }
    valS = *(reg+iS);
  }
  else if(source == 'm' && dest == 'r'){
    if(iS > 7 || iD > 3){
      printf("???\n");
      return;
    }
    valS = *(mem+iS);
  }
  else if(source == 'r' && dest == 'r'){
    if(iS > 3 || iD > 3){
      printf("???\n");
      return;
    }
    valS = *(reg+iS);
  }
  else{
    printf("???\n");
    return;
  }
  if(dest == 'm')
    *(mem+iD) = valS;
  else if(dest == 'r')
    *(reg+iD) = valS;
}

/*
 * add = source + destination
 * sets flags based off of the result
 *
 * IN: *mem = pointer to the first index of mem (uses addition to get to other indexes)
 *     *reg = pointer to the first index of reg (uses addition to get to other indexes)
 *     *OF = pointer to the overflow flag
 *     *SF = pointer to the sign flag
 *     *ZF = pointer to the zero flag
 */
void add(int *mem, int *reg, int *OF, int *SF, int *ZF){
  int valS, valD, sum;   // The values at source[i] & destination[i] and sum
  int error = readInput(mem, reg, &valS, &valD);
  sum = valS + valD;
  if(!error){
    printf("???\n");
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
 *     *OF = pointer to the overflow flag
 *     *SF = pointer to the sign flag
 *     *ZF = pointer to the zero flag
 */
void sub(int *mem, int *reg, int *OF, int *SF, int *ZF){
  int valS, valD, result;   // The values at source[i] & destination[i] and sum
  int error = readInput(mem, reg, &valS, &valD);
  result = valD - valS;
  if(!error || valS == (BITMAX+1) || valD == (BITMAX+1)){
    printf("???\n");
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
 *     *OF = pointer to the overflow flag
 *     *SF = pointer to the sign flag
 *     *ZF = pointer to the zero flag
 */
void mult(int *mem, int *reg, int *OF, int *SF, int *ZF){
  int valS, valD, result;   // The values at source[i] & destination[i] and sum
  int error = readInput(mem, reg, &valS, &valD);
  result = valS * valD;
  if(!error){
    printf("???\n");
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
 * div = source / destination
 * sets flags based off of the result
 *
 * IN: *mem = pointer to the first index of mem (uses addition to get to other indexes)
 *     *reg = pointer to the first index of reg (uses addition to get to other indexes)
 *     *SF = pointer to the sign flag
 *     *ZF = pointer to the zero flag
 */
void div(int *mem, int *reg, int *SF, int *ZF){
  int valS, valD, result;   // The values at source[i] & destination[i] and sum
  int error = readInput(mem, reg, &valS, &valD);
  result = valS / valD;
  if(!error || valS == (BITMAX+1) || valD == (BITMAX+1)){
    printf("???\n");
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
 *     *SF = pointer to the sign flag
 *     *ZF = pointer to the zero flag
 */
void mod(int *mem, int *reg, int *SF, int *ZF){
  int valS, valD, result;   // The values at source[i] & destination[i] and sum
  int error = readInput(mem, reg, &valS, &valD);
  result = valS % valD;
  if(!error || valS == (BITMAX+1) || valD == (BITMAX+1)){
    printf("???\n");
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
 *     *SF = pointer to the sign flag
 *     *ZF = pointer to the zero flag
 */
void comp(int *mem, int *reg, int *SF, int *ZF){
  int valS, valD, result;   // The values at source[i] & destination[i] and sum
  int error = readInput(mem, reg, &valS, &valD);
  if(!error){
    printf("???\n");
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
  printf("R0\tR1\tR2\tR3\tM0\tM1\tM2\tM3\tM4\tM5\tM6\tM7\tZF\tSF\tOF\n");
}
