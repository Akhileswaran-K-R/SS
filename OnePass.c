#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define SIZE 200

typedef struct{
  char label[20];
  int loc[20];
  int n;
}ForwardRef;

ForwardRef* addRef(ForwardRef *item,char label[20],int loc){
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

int searchRef(ForwardRef *list[],char label[20],int n){
  for(int i=0;i<n;i++){
    if(strcmp(list[i]->label,label) == 0){
      return i;
    }
  }
  return -1;
}

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

int searchSymtab(FILE *fsymtab,char label[20],char symaddr[20]){
  char symbol[20],addr[20],line[100];
  rewind(fsymtab);
  fscanf(fsymtab, "%*s %*s");
  
  while(fscanf(fsymtab,"%s %s",symbol,addr) != EOF){
    if(strcmp(label,symbol) == 0){
      strcpy(symaddr,addr);
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
  FILE *flength = fopen("files/length.txt","r");
  ForwardRef *list[20] = {NULL};

  fprintf(fsymtab,"%-8s%s\n","Label","LOC");
  char line[SIZE],label[20],opcode[20],operand[20],symbol[20],symaddr[20],mnemonic[20],hexacode[20],length[20],objcode[20],text[SIZE];
  int locctr = 0x0,start = 0x0,found,i,n = 0 ,count = 0,startobj;

  fscanf(flength,"%*s %*s %s",length);
  fgets(line,sizeof(line),fin);
  fgets(line,sizeof(line),fin);
  decode(line,label,opcode,operand);

  if(strcmp(opcode,"START") == 0){
    locctr = start = (int)strtol(operand,NULL,16);
    fprintf(fout,"H^%-6s^%06X^%06X\n",label,start,strtol(length,NULL,16));
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
          fprintf(fout,"T^%06X^%02X^%s\n",startobj,(strlen(text) - count + 1) / 2,text);
          strcpy(text,"");
          count = 0;
          startobj = locctr;
        }

        for(int j=0;j<list[i]->n;j++){
          fprintf(fout,"T^%06X^02^%04X\n",list[i]->loc[j],locctr);
        }
      }
      fseek(fsymtab,0,SEEK_END);
      fprintf(fsymtab,"%-8s%04X\n",label,locctr);
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
        fprintf(fout,"T^%06X^%02X^%s\n",startobj,(strlen(text) - count + 1) / 2,text);
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
    fprintf(fout,"T^%06X^%02X^%s\n",startobj,(strlen(text) - count + 1) / 2,text);
  }
  fprintf(fout,"E^%06X",start);

  fclose(fin);
  fclose(fout);
  fclose(fsymtab);
  fclose(foptab);
  fclose(flength);
}