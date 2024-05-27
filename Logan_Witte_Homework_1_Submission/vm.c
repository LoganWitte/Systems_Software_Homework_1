//Author: Logan Witte

#include <stdio.h>
#include <stdlib.h>

#define PAS_SIZE 500 //Size of process address space (pas)
#define PC_START 10 //Start position of text segment of pas
#define TESTING 0 //Boolean for toggling test statements

//Converts opcode mnemonic to corresponding opcodes
typedef enum opCodes{
    LIT = 1, //Pushes a constant value (literal) M onto the stack
    RTN, //AKA OPR, Operation to be performed on the data at the top of the stack. (or return from function)
    LOD, //Load value to top of stack from the stack location at offset M from L lexicographical levels down
    STO, //Store value at top of stack in the stack location at offset M from L lexicographical levels down
    CAL, //Call procedure at code index M (generates new Activation Record and PC <- M)
    INC, //Allocate M memory words (increment SP by M). First three are reserved to Static Link (SL), Dynamic Link (DL), and Return Address (RA)
    JMP, //Jump to instruction M (PC <- M)
    JPC, //Jump to instruction M if top stack element is 0
    SYS, //1: Write the top stack element to the screen, 2: Read in input from the user and store it on top of the stack, 3: End of program (Set “eop” flag to zero)
} opCodes;

//Stores values for current IR
typedef struct {
    int OP; //Operation code
    int L; //Lexicographical level
    int M; //Instruction specifier
} Instruction_Register; //Instruction Register to store current instruction

//Global variables
int pas[PAS_SIZE]; //Process address space
int activationChanges[PAS_SIZE]; //Keeps track of where activations change and a | is needed
int BP = PAS_SIZE - 1; //Base pointer - points to base of current activation record
int SP = PAS_SIZE; //Stack pointer - points to current top of stack
int PC = PC_START; //Program counter - points to next instruction to be executed
Instruction_Register IR; //Instruction register - stores next instruction to be executed

//Finds base L levels down
int base( int BP, int L) {
    int arb = BP; // arb = activation record base
    //find base L levels down
    while ( L > 0) {
        arb = pas[arb];
        L--;
    }
    return arb;
}

//Prints stack portion of pas
void printStack() {
    for(int i = PAS_SIZE - 1; i >= SP; i--) {
        if(i+1 < PAS_SIZE) {
            if(activationChanges[i+1]) {
                printf("| ");
            }
        }
        printf("%d ", pas[i]);
    }
    printf("\n");
}

//Prints stack portion of pas to file
void printStackToFile(FILE *fptr) {
    for(int i = PAS_SIZE - 1; i >= SP; i--) {
        if(i+1 < PAS_SIZE) {
            if(activationChanges[i+1]) {
                fprintf(fptr, "| ");
            }
        }
        fprintf(fptr, "%d ", pas[i]);
    }
    fprintf(fptr, "\n");
}

//Prints result of specified execution
void printExecutionResult(char * OPMnemonic) {
    printf("\t%s %d %d \t%d\t%d\t%d\t", OPMnemonic, IR.L, IR.M, PC, BP, SP);
    printStack();
}

//Prints result of specified execution to file
void printExecutionResultToFile(char * OPMnemonic, FILE *fptr) {
    fprintf(fptr, "\t%s %d %d \t%d\t%d\t%d\t", OPMnemonic, IR.L, IR.M, PC, BP, SP);
    printStackToFile(fptr);
}

int main(int argc, char **argv) {

    //Seperates program output if testing
    if(TESTING) {
        printf("\n-------------- PROGRAM OUTPUT BELOW --------------\n\n");
    }
    
    //Basic variables
    int endOfInstructions = 0;
    FILE *inputFile; //Stores input file
    FILE *outputFile = fopen("output.txt", "w"); //Stores output file

    //Recieves input file
    if(TESTING) {
        inputFile = fopen("input.txt", "r");
        if(inputFile == NULL) {
            printf("Input file is null");
            exit(-1);
        }
    }
    else {
        inputFile = fopen(argv[1], "r");
    }

    //Makes sure files are valid
    int exitFlag = 0;
    if(inputFile == NULL) {
        printf("Input file is null\n");
        exitFlag = 1;
    }
    if(outputFile == NULL) {
        printf("Output file is null\n");
        exitFlag = 1;
    }
    if(exitFlag) {
        exit(-1);
    }

    //Reads instructions into pas text segment
    for(endOfInstructions = PC; fscanf(inputFile, "%d %d %d", &pas[endOfInstructions], &pas[endOfInstructions+1], &pas[endOfInstructions+2]) == 3;  endOfInstructions += 3) {}
    fclose(inputFile);

    //Tests pas output
    if(TESTING) {
        printf("Text segment of PAS:\n");
        printf("pas[index]:\tOP\tL\tM\n");
        for(int i = PC; i < endOfInstructions; i += 3) {
            printf("pas[%d-%d]:\t%d\t%d\t%d\n", i, i+2, pas[i], pas[i+1], pas[i+2]);
        }
        printf("\n");
    }
    
    //Prints starting output
    printf("\t\t\tPC\tBP\tSP\tStack\n");  fprintf(outputFile, "\t\t\tPC\tBP\tSP\tStack\n");
    printf("Initial Values:\t\t%d\t%d\t%d\t", PC, BP, SP);    fprintf(outputFile, "Initial Values:\t\t%d\t%d\t%d\t", PC, BP, SP);
    printStack();   printStackToFile(outputFile);
    printf("\n");   fprintf(outputFile, "\n");

    //Runs fetch and execute cycles
    while(PC < endOfInstructions) {

        //Fetch Cycle
        IR.OP = pas[PC];
        IR.L = pas[PC+1];
        IR.M = pas[PC+2];
        PC += 3;

        //Execute Cycle
        switch(IR.OP) {
            case LIT:
                SP --;
                pas[SP] = IR.M;
                printExecutionResult("LIT");    printExecutionResultToFile("LIT", outputFile);
                break;
            
            case RTN:
                switch(IR.M) {
                    //RTN
                    case 0:
                        SP = BP + 1;
                        BP = pas[SP - 2];
                        PC = pas[SP - 3];
                        printExecutionResult("RTN");    printExecutionResultToFile("RTN", outputFile);
                        break;
                    //ADD
                    case 1:
                        pas[SP+1] = pas[SP+1] + pas[SP];
                        SP ++;
                        printExecutionResult("ADD");    printExecutionResultToFile("ADD", outputFile);
                        break;
                    //SUB
                    case 2:
                        pas[SP+1] = pas[SP+1] - pas[SP];
                        SP ++;
                        printExecutionResult("SUB");    printExecutionResultToFile("SUB", outputFile);
                        break;
                    //MUL
                    case 3:
                        pas[SP+1] = pas[SP+1] * pas[SP];
                        SP ++;
                        printExecutionResult("MUL");    printExecutionResultToFile("MUL", outputFile);
                        break;
                    //DIV
                    case 4:
                        pas[SP+1] = pas[SP+1] / pas[SP];
                        SP ++;
                        printExecutionResult("DIV");    printExecutionResultToFile("DIV", outputFile);
                        break;
                    //EQL
                    case 5:
                        pas[SP+1] = pas[SP+1] == pas[SP];
                        SP ++;
                        printExecutionResult("EQL");    printExecutionResultToFile("EQL", outputFile);
                        break;
                    //NEQ
                    case 6:
                        pas[SP+1] = pas[SP+1] != pas[SP];
                        SP ++;
                        printExecutionResult("NEQ");    printExecutionResultToFile("NEQ", outputFile);
                        break;
                    //LSS
                    case 7:
                        pas[SP+1] = pas[SP+1] < pas[SP];
                        SP ++;
                        printExecutionResult("LSS");    printExecutionResultToFile("LSS", outputFile);
                        break;
                    //LEQ
                    case 8:
                        pas[SP+1] = pas[SP+1] <= pas[SP];
                        SP ++;
                        printExecutionResult("LEQ");    printExecutionResultToFile("LEQ", outputFile);
                        break;
                    //GTR
                    case 9:
                        pas[SP+1] = pas[SP+1] > pas[SP];
                        SP ++;
                        printExecutionResult("GTR");    printExecutionResultToFile("GTR", outputFile);
                        break;
                    //GEQ
                    case 10:
                        pas[SP+1] = pas[SP+1] >= pas[SP];
                        SP ++;
                        printExecutionResult("GEQ");    printExecutionResultToFile("GEQ", outputFile);
                        break;
                }
                break;
            
            case LOD:
                SP --;
                pas[SP] = pas[base(BP, IR.L) - IR.M];
                printExecutionResult("LOD");    printExecutionResultToFile("LOD", outputFile);
                break;

            case STO:
                pas[base(BP, IR.L) - IR.M] = pas[SP];
                SP ++;
                printExecutionResult("STO");    printExecutionResultToFile("STO", outputFile);
                break;

            case CAL:
                pas[SP - 1] = base(BP, IR.L);
                pas[SP - 2] = BP;
                pas[SP - 3] = PC;
                BP = SP - 1;
                PC = IR.M;
                activationChanges[SP] = 1;
                printExecutionResult("CAL");    printExecutionResultToFile("CAL", outputFile);
                break;
            
            case INC:
                SP = SP - IR.M;
                printExecutionResult("INC");    printExecutionResultToFile("INC", outputFile);
                break;
            
            case JMP:
                PC = IR.M;
                printExecutionResult("JMP");    printExecutionResultToFile("JMP", outputFile);
                break;
            
            case JPC:
                if(pas[SP] == 0) {
                    PC = IR.M;
                    SP ++;
                }
                printExecutionResult("JPC");    printExecutionResultToFile("JPC", outputFile);
                break;

            case SYS:
                if(IR.M == 1) {
                    printf("Output result is: %d\n", pas[SP]);  fprintf(outputFile, "Output result is: %d\n", pas[SP]);
                    SP ++;
                }
                else if(IR.M == 2) {
                    SP --;
                    printf("Please enter an integer: ");
                    scanf("%d", &pas[SP]);
                    fprintf(outputFile, "Please enter an integer: %d\n", pas[SP]);
                }
                else if(IR.M == 3) {
                    printExecutionResult("SYS");    printExecutionResultToFile("SYS", outputFile);
                    fclose(outputFile);
                    if(TESTING) {
                        printf("Closed due to EOP\n");
                    }
                    exit(0);
                }
                printExecutionResult("SYS");    printExecutionResultToFile("SYS", outputFile);
                break;
        }

    }

    fclose(outputFile);
    return 0;
}