#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define SIZE 200

void decode(char line[SIZE],char loc[20],char opcode[20],char operand[20]){
  char *token = strtok(line," \n\t"),strings[4][20];
  int count = 0;
  while(token != NULL){
    strcpy(strings[count],token);
    count++;
    token = strtok(NULL," \n\t");
  }

  strcpy(loc,strings[0]);
  if(count == 2){
    strcpy(opcode,strings[2]);
    strcpy(operand,"");
  }else if(count == 3 && strcmp(strings[2],"RSUB") == 0){
    strcpy(opcode,strings[2]);
    strcpy(operand,"");
  }else if(count == 3){
    strcpy(opcode,strings[1]);
    strcpy(operand,strings[2]);
  }else{
    strcpy(opcode,strings[2]);
    strcpy(operand,strings[3]);
  }
}

void main(){
  FILE *fin = fopen("files/intermediate.txt","r");
  FILE *fout = fopen("files/record.txt","w");
  FILE *fsymtab = fopen("files/symtab.txt","r");
  FILE *foptab = fopen("files/optab.txt","r");
  FILE *finter = fopen("files/intermediate.txt","r+");
  FILE *flength = fopen("files/length.txt","r");

  char line[SIZE],loc[20],opcode[20],operand[20],symbol[20],symaddr[20],mnemonic[20],hexacode[20],name[20],obcode[20],length[20],start[20],startobj[20],text[SIZE];
  fseek(finter,36,SEEK_CUR);
  fprintf(finter,"%-12s\n","Object Code");

  fgets(line,sizeof(line),fin);
  fscanf(fin,"%s %s *%s %*s",start,name);
  fscanf(flength,"%*s %*s %s",length);
  fprintf(fout,"H^%-06s^%06s^%06s\n",name,start,length);

  fgets(line,sizeof(line),fin);
  decode(line,loc,opcode,operand);

  int count = 0;
  strcpy(startobj,start);
  while(strcmp(opcode,"END") != 0){
    strcpy(obcode,"-1");
    rewind(foptab);
    fgets(line,sizeof(line),foptab);
    int found = 0;
    while(fscanf(foptab,"%s %s",mnemonic,hexacode) != EOF){
      if(strcmp(opcode,mnemonic) == 0){
        found = 1;
        break;
      }
    }

    if(found == 1){
      strcpy(obcode,hexacode);
      if(strcmp(operand,"") != 0){
        rewind(fsymtab);
        fgets(line,sizeof(line),fsymtab);
        found = 0;
        
        while(fscanf(fsymtab,"%s %s",symbol,symaddr) != EOF){
          if(strcmp(operand,symbol) == 0){
            found = 1;
            break;
          }
        }

        if(found == 1){
          strcat(obcode,symaddr);
        }else{
          printf("\nSymbol not found in symtab\n");
          exit(0);
        }
      }else{
        sprintf(obcode+2,"%04d",0);
      }
    }else if(strcmp(opcode,"BYTE") == 0){
      if(operand[0] == 'C'){
        int j = 0;
        for(int i=2;i<strlen(operand)-1;i++){
          sprintf(obcode + j,"%02X",operand[i]);
          j += 2;
        }
      }else{
        for(int i=2;i<strlen(operand)-1;i++){
          sprintf(obcode + i - 2,"%c",operand[i]);
        }
      }
    }else if(strcmp(opcode,"WORD") == 0){
      sprintf(obcode,"%06X",atoi(operand));
    }

    if(strcmp(obcode,"-1") != 0){
      fseek(finter,35,SEEK_CUR);
      fprintf(finter,"%-12s\n",obcode);

      if(strlen(text) + strlen(obcode) - count <=60){
        if(count == 0){
          strcpy(text,obcode);
        }else{
          strcat(text,obcode);
        }
        count++;
        strcat(text,"^");
      }else{
        text[strlen(text) - 1] = '\0';
        fprintf(fout,"T^%06s^%02X^%s\n",startobj,(strlen(text) - count + 1) / 2,text);
        strcpy(text,"");
        count = 0;
        strcpy(startobj,loc);
      }
    }else{
      fseek(finter,49,SEEK_CUR);
    }
    fgets(line,sizeof(line),fin);
    decode(line,loc,opcode,operand);
  }

  if(strcmp(text,"") != 0){
    text[strlen(text) - 1] = '\0';
    fprintf(fout,"T^%06s^%02X^%s\n",startobj,(strlen(text) - count + 1) / 2,text);
  }
  fprintf(fout,"E^%06s",start);
}