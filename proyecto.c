#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int M[2048];
int S[4096];

int marcosLibres = 128;
int marcosLiberados[256];
int bitRef = 0;
int bitMod = 0;
int entranceOrder[2048];
int top[4] = {-1, -1, -1, -1}; //0: S 1:entranceOrder 2:M 3: MarcosLiberados
float timeStamp = 0;
int algRemplazo = 0; //Si es 0 es FIFO, si es 1 es LRU
int pageFaults = 0;
int totalSwapOut = 0;
int totalSwapIn = 0;

int isempty(int t) {

   if(top[t] == -1)
      return 1;
   else
      return 0;
}
   
int isfull(int t) {

   if(top[t] == 4096)
      return 1;
   else
      return 0;
}

int peek(int array[], int t) {
   return array[top[t]];
}

void removeTop(){
  int i = 0;
  while(entranceOrder[i])
  {
    entranceOrder[i] = entranceOrder[i + 1];
    i++;
  }
  top[1] = top[1] - 1;
}

int pop(int array[], int t) {
   int data;
	
   if(!isempty(t)) {
      data = array[top[t]];
      top[t] = top[t] - 1;   
      return data;
   } else {
      printf("Could not retrieve data, Stack is empty.\n");
   }
}

int push(int array[], int data, int t) {

   if(!isfull(t)) {
      top[t] = top[t] + 1;   
      array[top[t]] = data;
   } else {
      printf("Could not insert data, Stack is full.\n");
   }
}

int indexOf(int array[], int c) {
    int pos = -1;
    int i;
    for (i = 0; i < 2048 && pos == -1; i++)
    {
        if (array[i] == c) {
            pos = i;
        }
    }
    return pos;
}


void swapIn(int toSwap)
{
  if(!algRemplazo)
  {
    int move = 0;
    int s = 0;
    int pos = 0;
    for(int i = 0;;i++)
    {
      if(!S[i])
      {
        break;
      } 
      if(S[i] == toSwap)
      {
         move = 1;
         pos++; 
      }
      if(move)
      {
        S[i] = S[i + 1];
      }  
    }
    int posToSwap = 0;
    for(int i = 0; i < 2048;i++)
    {
      if(!M[i])
      {
        if(s == 0)
        {
          posToSwap = i;
        }
        s++;
        if(s >= pos)
        {
           break;
        }
      }
      else
      {
         s = 0;
      }
    }
    for(int i = posToSwap; i < posToSwap + pos; i++)
    {
        M[i] = toSwap;
    }
  }
  timeStamp++;
  totalSwapIn++;
}

void swapOut(int toSwap, int size)
{
  if(!algRemplazo)
  {
    int s = 0;
    for(int i = 0; i < 2048;i++)
    {
      if(M[i] == entranceOrder[0])
      {  
        if(s < size)
        {
         push(S, M[i], 0);
         M[i] = toSwap;
         s++;
        }
        else
        {
         break;
        }
      }
    }
    if(indexOf(M, toSwap) < 0)
    {
      removeTop();
    }
  }
  timeStamp++;
  pageFaults++;
  totalSwapOut++;
}

void cargarPagina()
{
  timeStamp++;
}

void accessPagina()
{
  timeStamp += 0.1;
}

void printMemory()
{
  for(int i = 0; i < 2048; i++)
  {
     if(!M[i])
     {
        printf("*\n");
     }
     else
     {
        printf("%d\n", M[i]);
     }
  }
}

void P(int args[])
{
  int spacesNeeded = ((args[0] / 16) + (args[1] % 16 != 0)) - 1;
  int i = 0; 
  if(indexOf(S, args[1]) >= 0)
  {
    push(entranceOrder, args[1], 1);
    swapIn(args[1]);
  }
  else if(indexOf(M, args[1]) < 0)
  {  
    if (spacesNeeded <= marcosLibres)
    {
      push(entranceOrder, args[1], 1);
      for(i = bitRef; i < bitRef + args[0]; i++)
      {
        M[i] = args[1];
      }
      bitRef = i;
      marcosLibres -= args[0] / 16;
    }
    else
    {
      push(entranceOrder, args[1], 1);
      swapOut(args[1], args[0]);
      swapIn(args[1]);
    }
  }
  printMemory();  
}

void A(int args[])
{
  int counter = 0;
  if(indexOf(M, args[1]) >= 0)
  {
    for(int j = 0; j < 2048; j++)
    {
      if(M[j] == args[1])
      {
        counter++;
        if(counter == args[0] + 1)
        {
           if(args[2] == 1)
           {
              printf("Pagina %d del proceso %d modificada\n", args[0], counter / 16);
           }
           printf("Direccion virtual: %d, Direccion real: %d\n", args[0], j);
           break;
        } 
      }
    }
  }
  else
  {
    printf("Process is not in memory\n");
  }
}

void L(int toRemove)
{
  int removedCounter = 0;
  for(int i = 0; i < 2048;i++)
  {
    if(M[i] == toRemove)
    {
      M[i] = 0;
      removedCounter++;
      if(removedCounter % 16 == 0)
      {
        push(marcosLiberados, i / 16, 3);
      }
    }
  }
  removedCounter = 0;
  if (!isempty(3))
  {
    printf("Marcos liberados de memoria: %d", pop(marcosLiberados, 3));
    marcosLibres++;
  }
  while(!isempty(3))
  {
    printf(", %d", pop(marcosLiberados, 3));
    marcosLibres++;
  }
  printf("\n");
  for(int i = 0; i < 4096;i++)
  {
    if(S[i] == toRemove)
    {
      S[i] = 0;
      removedCounter++;
      if(removedCounter % 16 == 0)
      {
        push(marcosLiberados, i / 16, 3);
      }
    }
  }
  if (!isempty(3))
  {
    printf("Marcos liberados de area de swapping: %d", pop(marcosLiberados, 3));
  }
  while(!isempty(3))
  {
    printf(", %d", pop(marcosLiberados, 3));
  }
  printf("\n");
}

void F()
{
  //print turnaround time
  printf("Total Page Faults: %d", pageFaults);
  pageFaults = 0;
  printf("Total Swap-out: %d", totalSwapOut);
  printf("Total Swap-in: %d", totalSwapIn);
  totalSwapOut = 0;
  totalSwapIn = 0;
  while(!isempty(0))
  {
    pop(S, 0);
  }
  while(!isempty(1))
  {
    pop(entranceOrder, 0);
  }
  while(!isempty(2))
  {
    pop(M, 0);
  }
}

void menu()
{
  char opr;
  char message[100];
  int cont = 1;

  while(cont)
  {
    scanf("%c %[^\n]%*c", &opr, message);
    int args[4];
    char *split = strtok(message," ");
    int i = 0;
    while(split != NULL)
    {
      args[i] = atoi(split);
      split=strtok(NULL," ");
      i++;
    }
    switch(opr)
    { 
      case 'P':
       P(args);
      break;
     case 'A':
       A(args);
     break;
     case 'L':
        L(args[0]);
     break;
     case 'C':
       printf("%s \n", message);
     break;
     case 'F':
       F();
     break;
     case 'E':
       printf("Adios\n");
       cont = 0;
     break;
    }
  }
}

void main()
{
  menu();
}
