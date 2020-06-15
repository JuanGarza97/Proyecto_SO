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
int usedTime[128][2];
int end = 0;

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

void setUsedTime(int toSet, int s)
{
  for(int i = 0; i < 128; i++)
  {
     if(usedTime[i][0] == toSet)
     {
        usedTime[i][1] = s;
        break;
     }
  }  
}

void restartUsedTime(int toRestart)
{
  setUsedTime(toRestart, 0);
}

void incrementUsedTime()
{
  for(int i = 0; i < 128; i++)
  {
//     if(usedTime[i][0] == toIncrement)
  //   {
        if(usedTime[i][1])
        {
          usedTime[i][1]++;
        }
    // }
  }  
}

int whoIsFirst(int s1 ,int s2)
{
  int first = 0;
  for(int i = 0; i < 128; i++)
  {
    if(entranceOrder[i] == s1)
    {
       first = s1;
       break;
    }
    else if(entranceOrder[i] == s2)
    {
       first = s2;
       break;
    }
  } 
  return first;
}

int getLeastUsed(int toCheck)
{
  int least = 0;
  int leastTime = 0;
  int lastLeast = 0;
  for(int i = 0; i < 128; i++)
  {
    if(usedTime[i][0])
    {
      if(!least)
      {
         least = usedTime[i][0];
         leastTime = usedTime[i][1];
      }
      else if(usedTime[i][1] <= leastTime)
      {
         if(usedTime[i][1] == leastTime)
         {
            lastLeast = least;
            least = whoIsFirst(usedTime[i][0], least);
            if (least == usedTime[i][0])
            { 
               leastTime = usedTime[i][1];
            }
         }
      }
    }
  }
  if(least == toCheck)
  {
    least = lastLeast;
  }
  return least;
}




void corrimientoS(int toSwap, int size)
{
  int swapCount = 0;
  int move = 0;
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
}

void swapIn(int toSwap, int size)
{
    corrimientoS(toSwap, size);
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
  int paginaSwapeadas[128];
  int direccionSwapeada[128];
  int paginasAsignadas[128];
  int paginasProcess = 0;
  if(!algRemplazo)
  {
    if(entranceOrder[0] == toSwap)
    {
      toCompare = entranceOrder[1];
    }
    else
    {
      toCompare = entranceOrder[0];
    } 
  }
  else
  {
     toCompare = getLeastUsed(toSwap);
  }
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
               paginaSwapeadas[swapCount] = i / 16 + 1;
               direccionSwapeada[swapCount] = j;
               swapCount++;
               break;
             }
           } 
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
  for(int i = 0; i < swapCount; i++)
  {
     printf("Pagina %d del proceso %d swappeada al marco %d del área de swapping\n", paginaSwapeadas[i], toCompare, direccionSwapeada[i]);
  }
  int i = 0;
    swapCount = 0;
    while(1)
    {
      if(!M[i])
      {
        M[i] = toSwap;
        swapCount++;
        if(swapCount % 16 == 0)
        {
           paginasAsignadas[paginasProcess] = (i / 16) + 1;
           tablaPaginasMemoria[i / 16][0] = toSwap;
           tablaPaginasMemoria[i / 16][1] = swapCount / 16;
           paginasProcess++;
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
  if(paginasAsignadas[0])
  {
    printf("Se le asignaron al proceso %d los marcos de pagina: %d", toSwap, paginasAsignadas[0]);
    for(int i = 1; i < size / 16; i++)
    {
      if(!paginasAsignadas[i])
      {
        break;
      }
      printf(", %d", paginasAsignadas[i]);
    }
    printf("\n");
  }
  totalSwapOut++;
  restartUsedTime(toCompare); 
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

void printSwap()
{
  for(int i = 0; i < 4096; i++)
  {
     if(!S[i])
     {
        printf("*\n");
     }
     else
     {
        printf("%d\n", S[i]);
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
    usedTime[totalProcess][0] = args[1];
    usedTime[totalProcess][1] = 1;
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
      if(paginasAsignadas[0])
      {
        printf("Se le asignaron al proceso %d los marcos de pagina: %d", args[1], paginasAsignadas[0]);
        for(int i = 1; i < paginasProcess; i++)
        {
          printf(", %d", paginasAsignadas[i]);
        }
        printf("\n");
      }
    }
    else
    {
      push(entranceOrder, args[1], 1);
      swapOut(args[1], args[0]);
    }
  }
  incrementUsedTime();
}

void A(int args[])
{
  int counter = 0;
  int found = 0;
  int toCompare = 0;
  int toSwap = args[1];
  int tablasPaginaRemplazo[128][2];
  int toCheck = 0;
  if(!algRemplazo)
  {
    if(entranceOrder[0] == toSwap)
    {
      toCompare = entranceOrder[1];
    }
    else
    {
      toCompare = entranceOrder[0];
    } 
  }
  else
  {
     toCompare = getLeastUsed(toSwap);
  }
  setUsedTime(toCompare, 1);
  if(args[0] / 16 <= 0)
  {
     toCheck = 1;
  }
  else
  {
     toCheck = args[0] / 16;
  }
  for(int i = 0; i < 128; i++)
  {
    if(tablaPaginasMemoria[i][0] == args[1] && tablaPaginasMemoria[i][1] == toCheck)
    {
      found = 1;
      if(args[2] == 1)
      {
         printf("Pagina %d del proceso %d modificada\n", args[0], args[1]);
      }
      printf("Direccion virtual: %d, Direccion real: %d\n", args[0], i + 1);
      break;
    }
  }
  if(!found)
  {  
    //swapIn(args[1], args[0]);
    for(int i = 0; i < 128; i++)
    {
      if(tablaPaginasSwap[i][0] == args[1] && tablaPaginasSwap[i][1] == toCheck)
      {
        found = 1;
        for(int j = 0; j < 128; j++)
        {
          if(tablaPaginasMemoria[j][0] == toCompare)
          {
            //tablaPaginaRemplazo[j][0] = tablaPaginasMemoria[j][0]
            //tablaPaginaRemplazo[j][1] = tablaPaginasMemoria[j][1]
            tablaPaginasMemoria[j][0] = tablaPaginasSwap[i][0];
            tablaPaginasMemoria[j][1] = tablaPaginasSwap[i][1];
            int k = 0;
            for(k = 0; k < 16; k++)
            {
               M[k+j] = tablaPaginasMemoria[j][0];
               S[i+k-1] = 0;
            }
            break;
          }
        }
      }
    }
    for(int i = 0; i < 128; i++)
    {
      tablaPaginasSwap[i][0] = tablaPaginasSwap[i + 1][0];
      tablaPaginasSwap[i][1] = tablaPaginasSwap[i + 1][1];
       if(timeStamp[i][0] == args[1])
       {
          timeStamp[i][1]++;
       }
    }
    totalSwapIn++;
    for(int i = 0; i < 128; i++)
  {
    if(tablaPaginasMemoria[i][0] == args[1] && tablaPaginasMemoria[i][1] == toCheck)
    {
      found = 1;
      printf("Pagina %d del proceso %d regresada al marco de memoria %d\n", args[0], args[1], i + 1);
      if(args[2] == 1)
      {
         printf("Pagina %d del proceso %d modificada\n", args[0], args[1]);
      }
      printf("Direccion virtual: %d, Direccion real: %d\n", args[0], i + 1);
      break;
    }
    
  }
   }
  if(!found)
  {
    printf("No se encontro al proceso %d en el memoria ni en el area de swapping\n", args[1]);
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
    printf("Marcos liberados de memoria: %d", pop(marcosLiberados, 3) + 1);
    marcosLibres++;
  }
  else
  {
    printf("No se encontro el proceso %d en memoria\n", toRemove);
  }

  while(!isempty(3))
  {
    printf(", %d", pop(marcosLiberados, 3) + 1);
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
  if(!isempty(3))
  {
    printf("Marcos liberados de area de swapping: %d", pop(marcosLiberados, 3));
  }
  else
  {
    printf("No se encontro el proceso %d en el area de swapping\n", toRemove);
  }
  int toPrint = 0;
  while(!isempty(3))
  {
    toPrint = pop(marcosLiberados, 3);
    if(toPrint)
    {
      printf(", %d", toPrint);
    }
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

void E()
{
  printf("Adios\n");
  end = 1;
  marcosLibres = 128;
  marcosLiberados[256];
  bitRef = 0;
  while(!isempty(1))
  {
    pop(entranceOrder, 1);
  }
  totalPaginas = 0;
  for(int j = 0; j < 2; j++)
  {
    for(int i = 0; i < 128; i++)
    {
      tablaPaginasMemoria[i][j] = 0;
      tablaPaginasSwap[i][j] = 0;
      timeStamp[i][j] = 0;
      pageFaults[i][j] = 0;
      usedTime[i][j] = 0;
    }
  }
  for(int i = 0; i < 2048;i++)
  {
    M[i] = 0;
  }
  for(int i = 0; i < 4096; i++)
  {
    S[i] = 0;
  }
  for(int i = 0; i < 6; i++)
  {
    top[i] = -1;
  }
  totalSwapOut = 0;
  totalSwapIn = 0;
  totalProcess = 0;
  turnAroundPromedio = 0;

}

void menu(char message[])
{
  if(!end)
  {
    char opr;
    //scanf("%[^\n]%*c", message);
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
        E();
      break;
    }
  }
}

void readFile()
{
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen("ArchivoTrabajo-1.txt", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
        printf("%s\n", line);
        menu(line);
    }

    fclose(fp);
    if (line)
        free(line);
    //exit(EXIT_SUCCESS);
}

void main()
{
  printf("Algoritmo FIFO\n");
  readFile();
  algRemplazo = 1;
  end = 0;
  printf("Algoritmo LRU\n");
  readFile();
}
