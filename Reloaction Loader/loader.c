#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define MAX 200
#define MIN 20

void convert(int hex,int bin[]){
  int i = 0;
  while(hex != 0){
    bin[i++] = hex % 2;
    hex /= 2;
  }

  while(i != 12){
    bin[i++] = 0;
  }
}

void decode(char line[],char text[],int *textloc,int newloc){
  char obj[MAX],temp[MIN];
  int i=11,hex,bin[MIN];
  sscanf(line,"T^%06X^%*2s^%03X^%s",textloc,&hex,obj);
  *textloc += newloc;
  strcpy(text,"");

  convert(hex,bin);
  char *token = strtok(obj,"^");
  while(token != NULL){
    if(bin[i--] == 1){
      int code = (int)strtol(token,NULL,16);
      int opcode = code / 0x10000;
      int operand = code % 0x10000;
      operand += newloc;
      sprintf(temp,"%02X%04X",opcode,operand);
      strcat(text,temp);
    }else{
      strcat(text,token);
    }
    token = strtok(NULL,"^");
  }
}

void allocate(int textloc,char text[],FILE *fop){
  int currloc = textloc; 

  for(int i=0;i<strlen(text)-1;i+=2){
    fprintf(fop,"%X: %c%c\n",currloc,text[i],text[i+1]);
    currloc++;
  }
}

void main(){
  FILE *fin = fopen("files/input.txt","r");
  FILE *fname = fopen("files/name.txt","r");
  FILE *flength = fopen("files/length.txt","r");
  FILE *fop = fopen("files/output.txt","w");

  
  char line[MAX],text[MAX],name1[MAX],name2[MAX];
  int startloc,textloc,length1,length2,newloc;
  fgets(line,sizeof(line),fin);
  sscanf(line,"H^%6s ^%*6s^%06X",name1,&length1);

  fscanf(fname,"%s",name2);
  fscanf(flength,"%X",&length2);
  if(!(strcmp(name1,name2) == 0 && length1 == length2)){
    printf("Wrong program loaded\n");
    exit(0);
  }
  printf("Enter the program address: ");
  scanf("%X",&newloc);

  fprintf(fop,"Loading program %s of length %X into memory\n\n",name1,length1);
  fgets(line,sizeof(line),fin);
  while(line[0] != 'E'){
    decode(line,text,&textloc,newloc);
    allocate(textloc,text,fop);
    fgets(line,sizeof(line),fin);
  }

  sscanf(line,"E^%06X",&startloc);
  fprintf(fop,"\nJumping to location %X",startloc + newloc);
}