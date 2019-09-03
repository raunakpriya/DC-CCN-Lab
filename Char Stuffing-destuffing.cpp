#include<conio.h>
#include<string.h>
#include<stdio.h>
#define DLE 16
#define STX 2
#define ETX 3
void main()
{ 
	char ch, arr[50]={DLE,STX},rec[50];
int i,j,len=2;
printf("\n enter the datastream:ctrl+p->DLE ctrl+b->STX ctrl+c->ETX\n");
while((ch=getch())!='\r')
{
if
(ch==DLE){
	arr[len++]=DLE;
	printf("DLE");
}
else if(ch==STX)
	printf("STX");
else if(ch==ETX)
	printf("ETX");
else printf("%c",ch);
arr[len++]=ch;
}
arr[len++]=DLE;
arr[len++]=ETX;
printf("\n bit stuffed stream is:\n");
for(i=0;i<len;i++){
	ch=arr[i];
	if(ch==DLE)
		printf("DLE");
	else if(ch==STX)
		printf("STX");
	else if(ch==ETX)
		printf("ETX");
	else printf("%c",ch);
}
printf("\n the destuffed stream is:\n");
for(j=2;j<len-2;j++)
{
	ch=arr[j];
	if(ch==DLE){
		printf("DLE");
		j++;
	}
	else if(ch==STX)
		printf("STX");
	else if(ch==ETX)
		printf("ETX");
	else printf("%c",ch);
}
getch();
}

