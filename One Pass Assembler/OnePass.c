#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define MAX 200
#define MIN 20

typedef struct{
  char label[MIN];
  int loc[MIN];
  int n;
}ForwardRef;

ForwardRef* addRef(ForwardRef *item,char label[],int loc){
  if(item == NULL){
    ForwardRef *newNode = (ForwardRef*)malloc(sizeof(ForwardRef));
    strcpy(newNode->label,label);
    newNode->loc[0] = loc;
    newNode->n = 1;
    return newNode;
  }

  item->loc[item->n] = loc;
  (item->n)++;
  return item;
}

int searchRef(ForwardRef *list[],char label[],int n){
  for(int i=0;i<n;i++){
    if(list[i] && strcmp(list[i]->label,label) == 0){
      return i;
    }
  }
  return -1;
}

void decode(char line[],char label[],char opcode[],char operand[]){
  char *token = strtok(line," \n"),strings[3][MIN];
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
  FILE *fout = fopen("files/record.txt","w");
  FILE *fsymtab = fopen("files/symtab.txt","w+");
  FILE *foptab = fopen("files/optab.txt","r");
  ForwardRef *list[MIN] = {NULL};

  fprintf(fsymtab,"%-8s%s\n","Label","LOC");
  char line[MAX],label[MIN],opcode[MIN],operand[MIN],symbol[MIN],symaddr[MIN],mnemonic[MIN],hexacode[MIN],length[MIN],objcode[MIN + 2],text[MAX],name[MIN];
  int locctr = 0x0,start = 0x0,found,i,n = 0 ,count = 0,startobj;

  fgets(line,sizeof(line),fin);
  fgets(line,sizeof(line),fin);
  decode(line,label,opcode,operand);

  if(strcmp(opcode,"START") == 0){
    locctr = start = (int)strtol(operand,NULL,16);
    strcpy(name,label);
    fprintf(fout,"%-25s","");
    fgets(line,sizeof(line),fin);
    decode(line,label,opcode,operand);
  }

  startobj = start;
  while(strcmp(opcode,"END") != 0){
    strcpy(objcode,"-1");
    if(strcmp(label,"") != 0){
      if(searchSymtab(fsymtab,label,symaddr)){
        printf("\nDuplicate symbol found\n");
        exit(0);
      }

      i = searchRef(list,label,n);
      if(i != -1){
        if(count != 0){
          text[strlen(text) - 1] = '\0';
          fprintf(fout,"\nT^%06X^%02X^%s",startobj,(int)((strlen(text) - count + 1) / 2),text);
          strcpy(text,"");
          count = 0;
          startobj = locctr;
        }

        for(int j=0;j<list[i]->n;j++){
          fprintf(fout,"\nT^%06X^02^%04X",list[i]->loc[j],locctr);
        }
        free(list[i]);
        list[i] = NULL;
      }
      fseek(fsymtab,0,SEEK_END);
      fprintf(fsymtab,"%-8s%04X\n",label,locctr);
    }

    found = searchOptab(foptab,opcode,hexacode);

    if(found){
      strcpy(objcode,hexacode);
      if(strcmp(operand,"") != 0){
        if(searchSymtab(fsymtab,operand,symaddr)){
          strcat(objcode,symaddr);
        }else{
          sprintf(objcode+2,"%04d",0);
          i = searchRef(list,operand,n);
          if(i == -1){
            list[n] = addRef(list[n],operand,locctr+1);
            n++;
          }else{
            list[i] = addRef(list[i],operand,locctr+1);
          }
        }
      }else{
        sprintf(objcode+2,"%04d",0);
      }
      locctr += 3;
    }else if(strcmp(opcode,"WORD") == 0){
      sprintf(objcode,"%06X",atoi(operand));
      locctr += 3;
    }else if(strcmp(opcode,"RESW") == 0){
      locctr += atoi(operand) * 3;
    }else if(strcmp(opcode,"RESB") == 0){
      locctr += atoi(operand);
    }else if(strcmp(opcode,"BYTE") == 0){
      if(operand[0] == 'C'){
        int j = 0;
        for(int i=2;i<strlen(operand)-1;i++){
          sprintf(objcode + j,"%02X",operand[i]);
          j += 2;
        }
        locctr += strlen(operand) - 3;
      }else{
        for(int i=2;i<strlen(operand)-1;i++){
          sprintf(objcode + i - 2,"%c",operand[i]);
        }
        locctr += (strlen(operand) - 3)/2;
      }
    }else{
      printf("\nInvalid operation\n");
      exit(0);
    }

    if(strcmp(objcode,"-1") != 0){
      if(strlen(text) + strlen(objcode) - count > 60){
        text[strlen(text) - 1] = '\0';
        fprintf(fout,"\nT^%06X^%02X^%s",startobj,(int)((strlen(text) - count + 1) / 2),text);
        strcpy(text,"");
        count = 0;
        startobj = locctr;
      }

      if(count == 0){
        strcpy(text,objcode);
      }else{
        strcat(text,objcode);
      }
      count++;
      strcat(text,"^");
    }

    fgets(line,sizeof(line),fin);
    decode(line,label,opcode,operand);
  }

  if(count != 0){
    text[strlen(text) - 1] = '\0';
    fprintf(fout,"\nT^%06X^%02X^%s",startobj,(int)((strlen(text) - count + 1) / 2),text);
  }

  if(name){
    fseek(fout,0,SEEK_SET);
    fprintf(fout,"H^%-6s^%06X^%06X",name,start,locctr - start);
    fseek(fout,0,SEEK_END);
  }

  if(strcmp(operand,"") != 0){
    searchSymtab(fsymtab,operand,symaddr);
    start = strtol(symaddr,NULL,16);
  }
  fprintf(fout,"\nE^%06X",start);
  

  for(int j=0;j<n;j++){
    if(list[j] != NULL){
      printf("\nInvalid Symbol\n");
    }
  }

  fclose(fin);
  fclose(fout);
  fclose(fsymtab);
  fclose(foptab);
}