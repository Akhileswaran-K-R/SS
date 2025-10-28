#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define MAX 200
#define MID 50
#define MIN 20

void processLine(FILE *fin,FILE *fdeftab,FILE *fnamtab,FILE *fout,int *EXPANDING,char line[],char label[],char opcode[],char operand[]);

void decode(char line[],char label[],char opcode[],char operand[]){
  char strings[3][MID];
  int count = sscanf(line,"%s %s %s",strings[0],strings[1],strings[2]);

  if(count == 1){
    strcpy(label,"");
    strcpy(opcode,strings[0]);
    strcpy(operand,"");
  }else if(count == 2 && strcmp(strings[1],"MACRO") == 0){
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

void getLine(FILE *fp,int EXPANDING,char line[],char label[],char opcode[],char operand[]){
  fgets(line,MAX,fp);
  decode(line,label,opcode,operand);

  if(EXPANDING){
    FILE *fargtab = fopen("files/argtab.txt","r");
    char args[MIN][MIN];
    int i = 0;
    while(fscanf(fargtab,"%s\n",args[i++]) != EOF);
    fclose(fargtab);

    char newop[MID] = "";
    for(int i=0;operand[i]!='\0';i++){
      if(operand[i] == '?'){
        i++;
        strcat(newop,args[operand[i] - '0' - 1]);
      }else{
        size_t len = strlen(newop);
        newop[len] = operand[i];
        newop[len + 1] = '\0';
      }
    }
    strcpy(operand,newop);
  }
}

int search(FILE *fnamtab,char opcode[],long *start,long *end){
  rewind(fnamtab);
  char name[MIN];

  while(fscanf(fnamtab,"%s %ld %ld",name,start,end) != EOF){
    if(strcmp(name,opcode) == 0){
      return 1;
    }
  }
  return 0;
}

void define(FILE *fin,FILE *fdeftab,FILE *fnamtab,int EXPANDING,char line[],char label[],char opcode[],char operand[]){
  fseek(fnamtab,0,SEEK_END);
  fseek(fdeftab,0,SEEK_END);
  fprintf(fnamtab,"%s %ld ",label,ftell(fdeftab));
  fprintf(fdeftab,"%-9s%-9s%-9s\n",label,opcode,operand);
  int LEVEL = 1,i = 0;
  char strings[MIN][MIN];

  if(strcmp(operand,"") != 0){
    char *token = strtok(operand,"&,");
    while(token != NULL){
      strcpy(strings[i++],token);
      token = strtok(NULL,"&,");
    }
  }

  while(LEVEL > 0){
    getLine(fin,EXPANDING,line,label,opcode,operand);

    if(strchr(operand,'&')){
      char newop[MID] = "";
      for(int j=0;operand[j]!='\0';j++){
        if(operand[j] == '&'){
          for(int k=0;k<i;k++){
            size_t len = strlen(strings[k]);
            if (strncmp(&operand[j+1], strings[k], len) == 0){
              sprintf(newop+strlen(newop),"?%d",k+1);
              j += len;
              break;
            }
          }
        }else{
          size_t len = strlen(newop);
          newop[len] = operand[j];
          newop[len + 1] = '\0';
        }
      }
      strcpy(operand,newop);
    }

    fprintf(fdeftab,"%-9s%-9s%-9s\n",label,opcode,operand);
    if(strcmp(opcode,"MACRO") == 0){
      LEVEL++;
    }else if(strcmp(opcode,"MEND") == 0){
      LEVEL--;
    }
  }
  fprintf(fnamtab,"%ld\n",ftell(fdeftab));
}

void expand(FILE *fin,FILE *fdeftab,FILE *fnamtab,FILE *fout,int *EXPANDING,char line[],char label[],char opcode[],char operand[],long start,long end){
  *EXPANDING = 1;
  fseek(fdeftab,start,SEEK_SET);
  fgets(line,MAX,fdeftab);
  fprintf(fout,".%-8s%-9s%-9s\n",label,opcode,operand);
  char prevLabel[MID];
  strcpy(prevLabel,label);

  if(strcmp(operand,"") != 0){
    FILE *fargtab = fopen("files/argtab.txt","w");
    char *token = strtok(operand,",");
    int i = 0;
    while(token != NULL){
      fprintf(fargtab,"%s\n",token);
      token = strtok(NULL,",");
    }
    fclose(fargtab);
  }

  int i = 1;
  while(ftell(fdeftab) != end){
    getLine(fdeftab,*EXPANDING,line,label,opcode,operand);
    if(strcmp(opcode,"MEND") != 0){
      if(i == 1){
        strcpy(label,prevLabel);
        i++;
      }
      processLine(fin,fdeftab,fnamtab,fout,EXPANDING,line,label,opcode,operand);
    }
  }
  *EXPANDING = 0;
}

void processLine(FILE *fin,FILE *fdeftab,FILE *fnamtab,FILE *fout,int *EXPANDING,char line[],char label[],char opcode[],char operand[]){
  long start,end;
  int found = search(fnamtab,opcode,&start,&end);

  if(found){
    expand(fin,fdeftab,fnamtab,fout,EXPANDING,line,label,opcode,operand,start,end);
  }else if(strcmp(opcode,"MACRO") == 0){
    define(fin,fdeftab,fnamtab,*EXPANDING,line,label,opcode,operand);
  }else{
    fprintf(fout,"%-9s%-9s%-9s\n",label,opcode,operand);
  }
}

void main(){
  FILE *fin = fopen("files/input.txt","r");
  FILE *fdeftab = fopen("files/deftab.txt","w+");
  FILE *fnamtab = fopen("files/namtab.txt","w+");
  FILE *fout = fopen("files/output.txt","w");

  char line[MAX],label[MID],opcode[MID],operand[MID];
  int EXPANDING = 0;

  do{
    getLine(fin,EXPANDING,line,label,opcode,operand);
    processLine(fin,fdeftab,fnamtab,fout,&EXPANDING,line,label,opcode,operand);
  }while(strcmp(opcode,"END") != 0);

  fclose(fin);
  fclose(fdeftab);
  fclose(fnamtab);
  fclose(fout);
}