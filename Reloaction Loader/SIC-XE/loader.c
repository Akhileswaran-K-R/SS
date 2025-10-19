#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define MAX 200

void decode(char line[],char text[],int *textloc,int offset){
  char obj[MAX];
  sscanf(line, "T^%06X^%*2s^%s",textloc,obj);
  *textloc += offset;
  strcpy(text,"");

  char *token = strtok(obj,"^");
  while(token != NULL){
    strcat(text,token);
    token = strtok(NULL,"^");
  }
}

void allocate(int textloc,char text[],FILE *fop){
  int currloc = textloc; 

  for(int i=0;i<strlen(text)-1;i+=2){
    fprintf(fop,"%04X: %c%c\n",currloc,text[i],text[i+1]);
    currloc++;
  }
}

void modify(char line[],int offset,int startloc,FILE *fop){
  int addr,size,loc;
  char obj[MAX] = "",hex[MAX],text[MAX];
  sscanf(line,"M^%06X^%02X+%*s",&addr,&size);
  addr += offset + startloc - 1;

  rewind(fop);
  fgets(line,MAX,fop);
  fgets(line,MAX,fop);

  while(fgets(line,MAX,fop) != NULL){
    sscanf(line,"%X: %*2s",&loc);
    if(loc == addr){
      break;
    }
  }

  long point = ftell(fop);
  for(int i=0;i<(size+1)/2;i++){
    fscanf(fop,"%*s %2s",hex);
    strcat(obj,hex);
  }

  int newobj = strtol(obj,NULL,16) + offset;
  sprintf(text,"%X",newobj);
  fseek(fop,point,SEEK_SET);
  allocate(++addr,text,fop);
  fseek(fop,0,SEEK_END);
}

void main(){
  FILE *fin = fopen("files/input.txt","r");
  FILE *fname = fopen("files/name.txt","r");
  FILE *flength = fopen("files/length.txt","r");
  FILE *fop = fopen("files/output.txt","w+");

  
  char line[MAX],text[MAX],name1[MAX],name2[MAX];
  int startloc,textloc,length1,length2,offset;
  fgets(line,sizeof(line),fin);
  sscanf(line,"H^%6s ^%06X^%06X",name1,&startloc,&length1);

  fscanf(fname,"%s",name2);
  fscanf(flength,"%X",&length2);
  if(!(strcmp(name1,name2) == 0 && length1 == length2)){
    printf("Wrong program loaded\n");
    exit(0);
  }

  printf("Enter the new offset address: ");
  scanf("%X",&offset);

  fprintf(fop,"Loading program %s of length %X into memory\n\n",name1,length1);
  fgets(line,sizeof(line),fin);
  while(line[0] != 'E'){
    if(line[0] == 'T'){
      decode(line,text,&textloc,offset);
      allocate(textloc,text,fop);
    }else if(line[0] == 'M'){
      modify(line,offset,startloc,fop);
    }
    fgets(line,sizeof(line),fin);
  }

  sscanf(line,"E^%06X",&startloc);
  fprintf(fop,"\nJumping to location %X",startloc + offset);
}