#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int M[2048];
int S[4096];

int marcosLibres = 128;
int marcosLiberados[256];
int bitRef = 0;
int entranceOrder[2048];
int totalPaginas = 0;
int tablaPaginasMemoria[128][2];
int tablaPaginasSwap[128][2];
int top[6] = {-1, -1, -1, -1, -1, -1}; //0: S 1:entranceOrder 2:M 3: MarcosLiberados 4: tablaPaginasMemoria 5:tablaPaginasSwap
float timeStamp[128][2];
int algRemplazo = 0; //Si es 0 es FIFO, si es 1 es LRU
int pageFaults[128][2];
int totalSwapOut = 0;
int totalSwapIn = 0;
int totalProcess = 0;
float turnAroundPromedio = 0;

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


void swapIn(int toSwap, int size)
{
  if(!algRemplazo)
  {
    int move = 0;
    int s = 0;
    int swapCount = 0;
    int pos = 0;
    for(int i = 0;;i++)
    {
      if(S[i] == toSwap)
      {
         move = 1;
         swapCount++;
      }
      if(move)
      {
        S[i] = S[i + 1];
      }
      if(swapCount >= size)
      {
        break;
      }  
    }
    for(int j = 0; j < 128; j++)
    {
      if(tablaPaginasSwap[j][0] == toSwap)
      {
         tablaPaginasSwap[j][0] = 0;
         tablaPaginasSwap[j][1] = 0;
         break;
       }
     }
     for(int i = 0; i < 128;i++)
     {
      tablaPaginasSwap[i][0] = tablaPaginasSwap[i + 1][0]; 
      tablaPaginasSwap[i][1] = tablaPaginasSwap[i + 1][1];
     }
  }
  for(int i = 0; i  < 128; i++)
  {
     if(timeStamp[i][0] == toSwap)
     {
        timeStamp[i][1]++;
     }
  }
  totalSwapIn++;
}

void swapOut(int toSwap, int size)
{
  int swapCount = 0;
  int toCompare = 0;
  if(entranceOrder[0] == toSwap)
  {
    toCompare = entranceOrder[1];
  }
  else
  {
    toCompare = entranceOrder[0];
  } 
  if(!algRemplazo)
  {
    int s = 0;
    for(int i = 0; i < 2048;i++)
    {
      if(M[i] == toCompare)
      {  
        if(s < size)
        {
         push(S, M[i], 0);
         if(i % 16 == 0)
         {
           for(int j = 0; j < 128; j++)
           {
             if(!tablaPaginasSwap[j][0])
             {
               tablaPaginasSwap[j][0] = M[i];
               tablaPaginasSwap[j][1] = i / 16;
               break;
             }
           } 
           //push(tablaPaginasSwap[0], M[i], 5);
           //push(tablaPaginasSwap[1], (i / 16), 5);
         }
         M[i] = 0;
         s++;
        }
        else
        {
         break;
        }
      }
    }
    if(indexOf(M, toCompare) < 0)
    {
      removeTop();
    }
  }
  int i = 0;
    while(1)
    {
      if(!M[i])
      {
        M[i] = toSwap;
        swapCount++;
        if(swapCount % 16 == 0)
        {
           tablaPaginasMemoria[i / 16][0] = toSwap;
           tablaPaginasMemoria[i / 16][1] = swapCount / 16;
        }
        if(swapCount >= size)
        {
          break;
        }
      }
      i++;
    }
  for(int i = 0; i  < 128; i++)
  {
     if(timeStamp[i][0] == toSwap)
     {
        timeStamp[i][1]++;
     }
     if(pageFaults[i][0] == toSwap)
     {
        pageFaults[i][1]++;
     }
  }
  totalSwapOut++;
   for(int j = 0; j < 128;j++)
  {
    printf("Swap: [%d][%d]\n", tablaPaginasSwap[j][0], tablaPaginasSwap[j][1]);
  }  
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
  int contador = 0;
  int contAssign = 0;
  int paginasAsignadas[128];
  int paginasProcess = 0;
  if(indexOf(S, args[1]) >= 0)
  {
    push(entranceOrder, args[1], 1);
    swapIn(args[1], args[0]);
    swapOut(args[1], args[0]);
  }
  else if(indexOf(M, args[1]) < 0)
  {  
    timeStamp[totalProcess][0] = args[1];
    timeStamp[totalProcess][1] = 1;
    pageFaults[totalProcess][0] = args[1];
    pageFaults[totalProcess][1] = 0;
    totalProcess++;

    if (spacesNeeded <= marcosLibres)
    {
      push(entranceOrder, args[1], 1);
      
      while(1)
      {
        contador++;
        if(!M[i])
        {
	  M[i] = args[1];
	  if(contador % 16 == 0)
	  {
            paginasAsignadas[paginasProcess] = (i / 16) + 1;
	    tablaPaginasMemoria[totalPaginas][0] = args[1];
	    tablaPaginasMemoria[totalPaginas][1] = paginasProcess + 1;
            paginasProcess++;
	    totalPaginas++;
	  }
        }
        if(paginasProcess >= spacesNeeded)
        {
            break;
        }
        i++;
      }
      bitRef = i;
      marcosLibres -= paginasProcess;
      printf("Se le asignaron al proceso %d los marcos de pagina: %d", args[1], paginasAsignadas[0]);
      for(int i = 1; i < paginasProcess; i++)
      {
        printf(", %d", paginasAsignadas[i]);
      }
      printf("\n");
    }
    else
    {
      push(entranceOrder, args[1], 1);
      swapOut(args[1], args[0]);
      //swapIn(args[1]);
    }
  }
 // printMemory();
  for(int j = 0; j < 128;j++)
  {
    printf("Memoria: [%d][%d]\n", tablaPaginasMemoria[j][0], tablaPaginasMemoria[j][1]);
  }  
}

void A(int args[])
{
  int counter = 0;
  int found = 0;
  int toCompare = 0;
  int toSwap = args[1];
  if(entranceOrder[0] == toSwap)
  {
    toCompare = entranceOrder[1];
  }
  else
  {
    toCompare = entranceOrder[0];
  } 
  if(!algRemplazo)
  {
  for(int i = 0; i < 128; i++)
  {
    if(tablaPaginasMemoria[i][0] == args[1] && tablaPaginasMemoria[i][1] == args[0])
    {
      found = 1;
      if(args[2] == 1)
      {
         printf("Pagina %d del proceso %d modificada\n", args[0], (i * 16) + 1);
      }
      printf("Direccion virtual: %d, Direccion real: %d\n", args[0], (i * 16) + 1);
      break;
    }
  }
  if(!found)
  {
    for(int i = 0; i < 128; i++)
    {
      if(tablaPaginasSwap[i][0] == args[1] && tablaPaginasSwap[i][1] == args[0])
      {
        found = 1;
        for(int j = 0; j < 128; j++)
        {
          if(tablaPaginasMemoria[j][0] == toCompare)
          {
            tablaPaginasMemoria[j][0] = tablaPaginasSwap[i][0];
            tablaPaginasMemoria[j][1] = tablaPaginasSwap[i][1];
            for(int k = 0; k < 16; k++)
            {
               M[k*j] = tablaPaginasMemoria[j][0];
            }
            break;
          }
        }
      }
    }
    swapIn(args[1], args[0]);
    for(int i = 0; i < 128; i++)
  {
    if(tablaPaginasMemoria[i][0] == args[1] && tablaPaginasMemoria[i][1] == args[0])
    {
      found = 1;
      if(args[2] == 1)
      {
         printf("Pagina %d del proceso %d modificada\n", args[0], (i * 16) + 1);
      }
      printf("Direccion virtual: %d, Direccion real: %d\n", args[0], (i * 16) + 1);
      break;
    }
  }
   }
 }
  for(int i = 0; i  < 128; i++)
  {
     if(timeStamp[i][0] == args[1])
     {
        timeStamp[i][1] += 0.1;
     }
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
   
  for(int i = 0; i < 128; i++)
  { 
    turnAroundPromedio += timeStamp[i][1];
    if(pageFaults[i][0])
    {
      printf("Total Page Faults for process %d: %d\n", pageFaults[i][0], pageFaults[i][1]);
      //pageFaults[i][0] = 0;
      //pageFaults[i][1] = 0;
    }
  }
  turnAroundPromedio /= totalProcess;
  printf("Turnaround promedio: %f\n", turnAroundPromedio);
  turnAroundPromedio = 0;
  printf("Total Swap-out: %d\n", totalSwapOut);
  printf("Total Swap-in: %d\n", totalSwapIn);
  totalSwapOut = 0;
  totalSwapIn = 0;
  /*while(!isempty(0))
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
  }*/
}

void menu()
{
  char opr;
  char message[100];
  int cont = 1;

  while(cont)
  {
    scanf("%[^\n]%*c", message);
    int args[4];
    char *split = strtok(message," ");
    int i = 0;
    opr = *split;
    split = strtok(NULL," ");
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
