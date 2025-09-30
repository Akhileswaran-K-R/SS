#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define MAX 200
#define MIN 20

void decode(char line[],char loc[],char opcode[],char operand[]){
  char *token = strtok(line," \n\t"),strings[4][MIN];
  int count = 0;
  while(token != NULL){
    strcpy(strings[count],token);
    count++;
    token = strtok(NULL," \n\t");
  }

  strcpy(loc,strings[0]);
  if(count == 2){
    strcpy(opcode,strings[1]);
    strcpy(operand,"");
  }else if(count == 3){
    strcpy(opcode,strings[1]);
    strcpy(operand,strings[2]);
  }else{
    strcpy(opcode,strings[2]);
    strcpy(operand,strings[3]);
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
  FILE *fin = fopen("files/intermediate.txt","r");
  FILE *fout = fopen("files/record.txt","w");
  FILE *fsymtab = fopen("files/symtab.txt","r");
  FILE *foptab = fopen("files/optab.txt","r");
  FILE *flisting = fopen("files/listing.txt","w");
  FILE *flength = fopen("files/length.txt","r");

  char line1[MAX],line2[MAX],loc[MIN],opcode[MIN],operand[MIN],symbol[MIN],symaddr[MIN],mnemonic[MIN],hexacode[MIN],name[MIN],objcode[MIN + 2],length[MIN],start[MIN],startobj[MIN],text[MAX];

  fgets(line1,sizeof(line1),fin);
  line1[strcspn(line1, "\n")] = '\0';
  fprintf(flisting, "%s%-12s\n", line1, "Object Code");

  fscanf(fin,"%s %s %s %s\n",start,name,opcode,operand);
  fprintf(flisting,"%-6s%-10s%-10s%-10s\n",start,name,opcode,operand);
  fscanf(flength,"%*s %*s %s",length);
  fprintf(fout,"H^%-6s^%06X^%06X\n",name,(int)strtol(start,NULL,16),(int)strtol(length,NULL,16));

  fgets(line1,sizeof(line1),fin);
  strcpy(line2,line1);
  decode(line1,loc,opcode,operand);

  int count = 0,found,newText = 0;
  strcpy(startobj,start);
  while(strcmp(opcode,"END") != 0){
    strcpy(objcode,"-1");
    found = searchOptab(foptab,opcode,hexacode);

    if(found){
      strcpy(objcode,hexacode);
      if(strcmp(operand,"") != 0){
        if(searchSymtab(fsymtab,operand,symaddr)){
          strcat(objcode,symaddr);
        }else{
          printf("\nSymbol not found in symtab\n");
          exit(0);
        }
      }else{
        sprintf(objcode+2,"%04d",0);
      }
    }else if(strcmp(opcode,"BYTE") == 0){
      if(operand[0] == 'C'){
        int j = 0;
        for(int i=2;i<strlen(operand)-1;i++){
          sprintf(objcode + j,"%02X",operand[i]);
          j += 2;
        }
      }else{
        for(int i=2;i<strlen(operand)-1;i++){
          sprintf(objcode + i - 2,"%c",operand[i]);
        }
      }
    }else if(strcmp(opcode,"WORD") == 0){
      sprintf(objcode,"%06X",atoi(operand));
    }else{
      newText = 1;
    }

    if(strcmp(objcode,"-1") != 0){
      line2[strcspn(line2, "\n")] = '\0';
      fprintf(flisting,"%s%-12s\n",line2,objcode);

      if(strlen(text) + strlen(objcode) - count > 60 || newText){
        text[strlen(text) - 1] = '\0';
        fprintf(fout,"T^%06X^%02X^%s\n",(int)strtol(startobj,NULL,16),(int)((strlen(text) - count + 1) / 2),text);
        strcpy(text,"");
        count = 0;
        newText = 0;
        strcpy(startobj,loc);
      }

      if(count == 0){
        strcpy(text,objcode);
      }else{
        strcat(text,objcode);
      }
      count++;
      strcat(text,"^");
    }else{
      fprintf(flisting,"%s",line2);
    }
    fgets(line1,sizeof(line1),fin);
    strcpy(line2,line1);
    decode(line1,loc,opcode,operand);
  }
  
  fprintf(flisting,"%s",line2);
  if(count != 0){
    text[strlen(text) - 1] = '\0';
    fprintf(fout,"T^%06X^%02X^%s\n",(int)strtol(startobj,NULL,16),(int)((strlen(text) - count + 1) / 2),text);
  }

  if(strcmp(operand,"") != 0){
    searchSymtab(fsymtab,operand,symaddr);
    strcpy(start,symaddr);
  }
  fprintf(fout,"E^%06X",(int)strtol(start,NULL,16));
}