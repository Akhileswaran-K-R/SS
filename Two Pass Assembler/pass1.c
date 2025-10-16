#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define MAX 200
#define MIN 20

void decode(char line[],char label[],char opcode[],char operand[]){
  char strings[3][MIN];
  int count = sscanf(line,"%s %s %s",strings[0],strings[1],strings[2]);

  if(count == 1){
    strcpy(label,"");
    strcpy(opcode,strings[0]);
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

int searchOptab(FILE *foptab,char opcode[],char hexacode[]){
  char mnemonic[MIN],line[MAX];
  rewind(foptab);
  fgets(line,sizeof(line),foptab);

  while(fscanf(foptab,"%s %s",mnemonic,hexacode) != EOF){
    if(strcmp(opcode,mnemonic) == 0){
      return 1;
    }
  }
  return 0;
}

int searchSymtab(FILE *fsymtab,char label[],char symaddr[]){
  char symbol[MIN],line[100];
  rewind(fsymtab);
  fgets(line,sizeof(line),fsymtab);
  
  while(fscanf(fsymtab,"%s %s",symbol,symaddr) != EOF){
    if(strcmp(label,symbol) == 0){
      return 1;
    }
  }
  return 0;
}

void main(){
  FILE *fin = fopen("files/input.txt","r");
  FILE *fout = fopen("files/symtab.txt","w");
  FILE *fsymtab = fopen("files/symtab.txt","r");
  FILE *foptab = fopen("files/optab.txt","r");
  FILE *finter = fopen("files/intermediate.txt","w"); 
  FILE *flength = fopen("files/length.txt","w");

  fprintf(fout,"%-8s%s\n","Label","LOC");
  fprintf(finter, "%-6s%-10s%-10s%-10s\n", "LOC", "Label", "Opcode", "Operand");

  char line[MAX],label[MIN],opcode[MIN],operand[MIN],symbol[MIN],symaddr[MIN],mnemonic[MIN],hexacode[MIN];
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
      if(searchSymtab(fsymtab,label,symaddr)){
        printf("\nDuplicate symbol found\n");
        exit(0);
      }
      fprintf(fout,"%-8s%04X\n",label,locctr);
    }
    found = searchOptab(foptab,opcode,hexacode);

    fprintf(finter, "%04X\t%-10s%-10s%-10s\n", locctr, label, opcode, operand);
    if(found || strcmp(opcode,"WORD") == 0){
      locctr += 3;
    }else if(strcmp(opcode,"RESW") == 0){
      locctr += atoi(operand) * 3;
    }else if(strcmp(opcode,"RESB") == 0){
      locctr += atoi(operand);
    }else if(strcmp(opcode,"BYTE") == 0){
      if(operand[0] == 'C'){
        locctr += strlen(operand) - 3;
      }else{
        locctr += (strlen(operand) - 3)/2;
      }
    }else{
      printf("\nInvalid operation\n");
      exit(0);
    }

    fgets(line,sizeof(line),fin);
    decode(line,label,opcode,operand);
  }
  fprintf(finter, "%04X\t%-10s%-10s%-10s", locctr, label, opcode, operand);
  fprintf(flength, "Program size: %X",(locctr - start));

  fclose(fin);
  fclose(fout);
  fclose(fsymtab);
  fclose(foptab);
  fclose(finter);
}