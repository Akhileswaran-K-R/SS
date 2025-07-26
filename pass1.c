#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define SIZE 200

void decode(char line[SIZE],char label[20],char opcode[20],char operand[20]){
  char *token = strtok(line," \n"),strings[3][20];
  int count = 0;
  while(token != NULL){
    strcpy(strings[count],token);
    count++;
    token = strtok(NULL," \n");
  }

  if(count == 1){
    strcpy(label,"");
    strcpy(opcode,strings[0]);
    strcpy(operand,"");
  }else if(count == 2 && strcmp(strings[1],"RSUB") == 0){
    strcpy(label,strings[0]);
    strcpy(opcode,strings[1]);
    strcpy(operand,"");
  }else if(count == 2){
    strcpy(label,"");
    strcpy(opcode,strings[0]);
    strcpy(operand,strings[1]);
  }else{
    strcpy(label,strings[0]);
    strcpy(opcode,strings[1]);
    strcpy(operand,strings[2]);
  }
}

void main(){
  FILE *fin = fopen("files/input.txt","r");
  FILE *fout = fopen("files/symtab.txt","w");
  FILE *fsymtab = fopen("files/symtab.txt","r");
  FILE *foptab = fopen("files/optab.txt","r");
  FILE *finter = fopen("files/intermediate.txt","w"); 

  fprintf(fout,"%-8s%s\n","Label","LOC");
  fprintf(finter, "%-6s%-10s%-10s%-10s\n", "LOC", "Label", "Opcode", "Operand");

  char line[SIZE],label[20],opcode[20],operand[20],symbol[20],symaddr[20],mnemonic[20],hexacode[20];
  int locctr = 0x0,start = 0x0,found;

  fgets(line,sizeof(line),fin);
  fgets(line,sizeof(line),fin);
  decode(line,label,opcode,operand);
  if(strcmp(opcode,"START") == 0){
    locctr = start = (int)strtol(operand,NULL,16);
    fprintf(finter, "%04X\t%-10s%-10s%-10s\n", locctr, label, opcode, operand);
    fgets(line,sizeof(line),fin);
    decode(line,label,opcode,operand);
  }

  while(strcmp(opcode,"END") != 0){
    if(strcmp(label,"") != 0){
      rewind(fsymtab);
      fgets(line,sizeof(line),fsymtab);
      
      while(fscanf(fsymtab,"%s %s",symbol,symaddr) != EOF){
        if(strcmp(label,symbol) == 0){
          printf("\nDuplicate symbol found\n");
          exit(0);
        }
      }
      fprintf(fout,"%-8s%04X\n",label,locctr);
    }

    rewind(foptab);
    fgets(line,sizeof(line),foptab);
    found = 0;
    while(fscanf(foptab,"%s %s",mnemonic,hexacode) != EOF){
      if(strcmp(opcode,mnemonic) == 0){
        found = 1;
        break;
      }
    }

    fprintf(finter, "%04X\t%-10s%-10s%-10s\n", locctr, label, opcode, operand);
    if(found || strcmp(opcode,"WORD") == 0){
      locctr += 3;
    }else if(strcmp(opcode,"RESW") == 0){
      locctr += (int)strtol(operand,NULL,16) * 3;
    }else if(strcmp(opcode,"RESB") == 0){
      locctr += (int)strtol(operand,NULL,16);
    }else if(strcmp(opcode,"BYTE") == 0){
      if(operand[0] == 'C'){
        locctr += strlen(operand) - 3;
      }else{
        locctr += 1;
      }
    }else{
      printf("%s",opcode);
      printf("\nInvalid operation\n");
      exit(0);
    }

    fgets(line,sizeof(line),fin);
    decode(line,label,opcode,operand);
  }
  fprintf(finter, "%04X\t%-10s%-10s%-10s\n\n", locctr, label, opcode, operand);
  fprintf(finter, "Program size: %X",(locctr - start));

  fclose(fin);
  fclose(fout);
  fclose(fsymtab);
  fclose(foptab);
  fclose(finter);
}