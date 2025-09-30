#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define MAX 200

void decode(char line[],char text[],int *textloc){
  char obj[MAX];
  sscanf(line, "T^%06X^%*2s^%s",textloc,obj);
  strcpy(text,"");

  char *token = strtok(obj,"^");
  while(token != NULL){
    strcat(text,token);
    token = strtok(NULL,"^");
  }
}

void printX(int rem,int *currloc,int textloc,int mode,FILE *fop){
  while ((mode == 1 && *currloc % 16 != rem) || (mode == 2 && *currloc != textloc) || (mode == 3 && *currloc % 16 != rem && *currloc != textloc)){
    if(*currloc % 16 == rem){
      fprintf(fop,"\n%04X\t",*currloc);
    }
    if((*currloc - rem) % 4 == 0){
      fprintf(fop,"  ");
    }
    fprintf(fop,"xx");
    (*currloc)++;
  }
}

void allocate(int rem,int *currloc,int textloc,char text[],FILE *fop){
  if(*currloc < textloc){
    printX(rem,currloc,textloc,3,fop);
    *currloc = (textloc / 16) * 16 + rem;
    printX(rem,currloc,textloc,2,fop);
  }

  for(int i=0;i<strlen(text)-1;i+=2){
    if(*currloc % 16 == rem){
      fprintf(fop,"\n%04X\t",*currloc);
    }
    if((*currloc - rem) % 4 == 0){
      fprintf(fop,"  ");
    }
    fprintf(fop,"%c%c",text[i],text[i+1]);
    (*currloc)++;
  }
}

void main(){
  FILE *fin = fopen("files/input.txt","r");
  FILE *fname = fopen("files/name.txt","r");
  FILE *flength = fopen("files/length.txt","r");
  FILE *fop = fopen("files/output.txt","w");

  fprintf(fop, "%-23s%-23s", "Address", "Content");
  char line[MAX],text[MAX],name1[MAX],name2[MAX];
  int startloc,currloc,textloc,length1,length2,rem;
  fgets(line,sizeof(line),fin);
  sscanf(line,"H^%6s ^%06X^%06X",name1,&currloc,&length1);
  rem = currloc % 16;

  fscanf(fname,"%s",name2);
  fscanf(flength,"%X",&length2);
  if(!(strcmp(name1,name2) == 0 && length1 == length2)){
    printf("Error occured");
    exit(0);
  }

  fgets(line,sizeof(line),fin);
  while(line[0] != 'E'){
    decode(line,text,&textloc);
    allocate(rem,&currloc,textloc,text,fop);
    fgets(line,sizeof(line),fin);
  }
  printX(rem,&currloc,0,1,fop);

  sscanf(line,"E^%06X",&startloc);
  fprintf(fop,"\n\nJumping to location %X",startloc);
}