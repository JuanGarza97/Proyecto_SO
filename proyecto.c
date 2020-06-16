#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define Marcos 128
#define tamanoPagina 16

int M[Marcos*tamanoPagina]; //arreglo de Memoria
int S[Marcos*tamanoPagina*2]; //arreglo de area de swapping

int marcosLibres = Marcos;  //cantidad de marcosLibres
int marcosLiberados[Marcos]; //marcos que se han liberado
int bitRef = 0; //bit de referencia
int entranceOrder[Marcos*tamanoPagina]; //Marca el orden de entrada, se utiliza para el algoritmo FIFO
int totalPaginas = 0; //Total de paginas asignadas
int tablaPaginasMemoria[Marcos][2]; //arreglo de tabla paginas de memoria donde la primer columna indica el proceso y la segunda la pagina
int tablaPaginasSwap[Marcos][2];  //arreglo de tabla paginas de memoria donde la primer columna indica el proceso y la segunda la pagina
int top[6] = {-1, -1, -1, -1, -1, -1}; //Indica el top de las pilas 0: S 1:entranceOrder 2:M 3: MarcosLiberados 4: tablaPaginasMemoria 5:tablaPaginasSwap
float timeStamp[Marcos][2]; //tiempo total en memoria de los procesos
int algRemplazo = 0; //Si es 0 es FIFO, si es 1 es LRU
int pageFaults[Marcos][2]; //page faults por proceso
int totalSwapOut = 0; //Total de swap-outs realizados
int totalSwapIn = 0; //Total de swap-in realizados
int totalProcess = 0; //Total de procesos
float turnAroundPromedio = 0; //Turnaround promedio
int usedTime[Marcos][2]; //Tiempo en el que los procesos han estado en memoria, se reinicia cada vez que el proceso se va al area de swapping. Esto se utiliza para el LRU
int end = 0; //Booleano para indicar que termino el programa

int isempty(int t) { //Esta funcion sirve para checar si el arreglo indicado se enuentra vacio

   if(top[t] == -1)
      return 1;
   else
      return 0;
}
   
int isfull(int t) { //Esta funcion sirve para checar si el arreglo indicado se encuentra lleno

   int toCompare = Marcos;
   switch(t)
   {
     case 0:
       toCompare = Marcos*tamanoPagina*2;
     break;
     case 1:
     case 2:
       toCompare = Marcos*tamanoPagina;
     break;
     case 3:
       toCompare = Marcos;
     break;
   }
     
   if(top[t] == toCompare)
      return 1;
   else
      return 0;
}

void removeTop(){ //Esta funcion se utiliza al momento de checar cual es el proceso que llego primero. Utilizado en el algoritmo FIFO
  int i = 0;
  while(entranceOrder[i])
  {
    entranceOrder[i] = entranceOrder[i + 1];
    i++;
  }
  top[1] = top[1] - 1;
}

int pop(int array[], int t) { //Esta funcion saca el ultimo elemento de una pila y lo regresa
   int data;
	
   if(!isempty(t)) {
      data = array[top[t]];
      top[t] = top[t] - 1;   
      return data;
   } else {
      printf("Could not retrieve data, Stack is empty.\n");
   }
}

int push(int array[], int datoAgregar, int t) { //Esta funcion agrega un elemento a la pila

   if(!isfull(t)) {
      top[t] = top[t] + 1;   
      array[top[t]] = datoAgregar;
   } else {
      printf("Could not insert data, Stack is full.\n");
   }
}

int indexOf(int array[], int c) { //Esta funcion busca el numero c en array[]
    int pos = -1;
    int i;
    for (i = 0; i < Marcos*tamanoPagina && pos == -1; i++)
    {
        if (array[i] == c) {
            pos = i;
        }
    }
    return pos;
}

void setUsedTime(int toSet, int s) //Esta funcion se utiliza para cambiar el tiempo en el que ha sido usado un proceso. Utilizado en el agoritmo LRU.
{
  for(int i = 0; i < Marcos; i++)
  {
     if(usedTime[i][0] == toSet)
     {
        usedTime[i][1] = s;
        break;
     }
  }  
}

void restartUsedTime(int toRestart) //Esta funcion se utiliza para cambiar el tiempo de un proceso a 0, indicando que no se encuentra en memoria. Utilizado en el algoritmo LRU.
{
  setUsedTime(toRestart, 0);
}

void incrementUsedTime() //Esta funcion incrementa el tiempo utilizado de todos los procesos que se encuentran en memoria por 1
{
  for(int i = 0; i < Marcos; i++)
  {
        if(usedTime[i][1])
        {
          usedTime[i][1]++;
        }
  }  
}

int whoIsFirst(int s1 ,int s2)  //Cuando hay un empate en el algoritmo LRU se utiliza esta funcion para checar cual proceso entro primero, si el s1 o el s2
{
  int first = 0;
  for(int i = 0; i < Marcos; i++)
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

int getLeastUsed(int toCheck) //Esta funcion regresa el Least Recent Used para utilizarlo en el algoritmo LRU
{
  int least = 0;
  int leastTime = 0;
  int lastLeast = 0;
  for(int i = 0; i < Marcos; i++)
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




void corrimientoS(int toSwap, int size) //Como el area de swapping es continua, esta funcion se utiliza para recorrer los elementos del area de swapping despues de haber realizado un swap-out
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

void swapIn(int toSwap, int size) //Esta funcion realiza el proceso de swap-in
{
    corrimientoS(toSwap, size);
    for(int j = 0; j < Marcos; j++)
    {
      if(tablaPaginasSwap[j][0] == toSwap)
      {
         tablaPaginasSwap[j][0] = 0;
         tablaPaginasSwap[j][1] = 0;
         break;
       }
     }
     for(int i = 0; i <Marcos;i++)
     {
      tablaPaginasSwap[i][0] = tablaPaginasSwap[i + 1][0]; 
      tablaPaginasSwap[i][1] = tablaPaginasSwap[i + 1][1];
     }
  for(int i = 0; i  < Marcos; i++)
  {
     if(timeStamp[i][0] == toSwap)
     {
        timeStamp[i][1]++;
     }
  }
  totalSwapIn++;
}

void swapOut(int toSwap, int size) //Esta funcion realiza el proceso de swap-out
{
  int swapCount = 0;
  int toCompare = 0;
  int paginaSwapeadas[Marcos];
  int direccionSwapeada[Marcos];
  int paginasAsignadas[Marcos];
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
    for(int i = 0; i < Marcos*tamanoPagina;i++)
    {
      if(M[i] == toCompare)
      {  
        if(s < size)
        {
         push(S, M[i], 0);
         if(i % tamanoPagina == 0)
         {
           for(int j = 0; j < Marcos; j++)
           {
             if(!tablaPaginasSwap[j][0])
             {
               tablaPaginasSwap[j][0] = M[i];
               tablaPaginasSwap[j][1] = i / tamanoPagina;
               paginaSwapeadas[swapCount] = i / tamanoPagina + 1;
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
        if(swapCount % tamanoPagina == 0)
        {
           paginasAsignadas[paginasProcess] = (i / tamanoPagina) + 1;
           tablaPaginasMemoria[i / tamanoPagina][0] = toSwap;
           tablaPaginasMemoria[i / tamanoPagina][1] = swapCount / tamanoPagina;
           paginasProcess++;
        }
        if(swapCount >= size)
        {
          break;
        }
      }
      i++;
    }
  for(int i = 0; i  < Marcos; i++)
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

void printMemory() //Esta funcion imprime la memoria utilizada para el debuggeo
{
  for(int i = 0; i < Marcos*tamanoPagina; i++)
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

void printSwap() //Esta funcion imprime el area de swapping utilizada para el debuggeo
{
  for(int i = 0; i < Marcos*tamanoPagina*2; i++)
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


void P(int args[]) //Esta funcion realiza la operacion P de asignacion de memoria el proceso indicado. Si el proceso ya se encuentra en memoria no hace nada, si el proceso no se encuentra en memoria pero si en el area de swapping entonces lo trae de regreso al area de memoria utilizando el algoritmo indicado
{
  int spacesNeeded = ((args[0] / tamanoPagina) + (args[1] % tamanoPagina != 0)) - 1; //Indica cuantas paginas va a necesitar el proceso
  int i = 0;
  int contador = 0;
  int contAssign = 0;
  int paginasAsignadas[Marcos];
  int paginasProcess = 0;
  if(indexOf(S, args[1]) >= 0) //En el caso de que el proceso se encuentre en el area de swapping
  {
    push(entranceOrder, args[1], 1); //Agrega el proceso al orden de entrada
    swapIn(args[1], args[0]); //Realiza swap-in
    swapOut(args[1], args[0]); //Realiza swap-out
  }
  else if(indexOf(M, args[1]) < 0) //En el caso de que el proceso no se encuentre en memoria ni en el area de swapping, agrega el proceso a memoria
  {  
    timeStamp[totalProcess][0] = args[1];
    timeStamp[totalProcess][1] = 1;
    pageFaults[totalProcess][0] = args[1];
    pageFaults[totalProcess][1] = 0;
    usedTime[totalProcess][0] = args[1];
    usedTime[totalProcess][1] = 1;
    totalProcess++;

    if (spacesNeeded <= marcosLibres) //Checa si hay espacio suficiente para el proceso
    {
      push(entranceOrder, args[1], 1); //Agrega el proceso al orden de entrada
      
      while(1)
      {
        contador++;
        if(!M[i])
        {
	  M[i] = args[1];
	  if(contador % tamanoPagina == 0)
	  {
            paginasAsignadas[paginasProcess] = (i / tamanoPagina) + 1;
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
    else if(spacesNeeded <= Marcos*tamanoPagina) //Si no hay espacio suficiente para el proceso, libera espacio para poder colocarlo
    {
      push(entranceOrder, args[1], 1); //Agrega el proceso al orden de entrada
      swapOut(args[1], args[0]); //Realiza swap-out
    }
    else
    {
      printf("La memoria no es lo suficientemente grande para ese proceso\n");
    }
  }
  incrementUsedTime();
}

void A(int args[]) //Realiza la operacion A de accesar memoria para ver o modificar la pagina indicada del proceso indicado. Si la pagina se encuentra en el area de swapping, la regresa a memoria
{
  int counter = 0;
  int found = 0;
  int toCompare = 0;
  int toSwap = args[1];
  int tablasPaginaRemplazo[Marcos][2];
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
  if(args[0] / tamanoPagina <= 0)
  {
     toCheck = 1;
  }
  else
  {
     toCheck = args[0] / tamanoPagina;
  }
  for(int i = 0; i < Marcos; i++)
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
    for(int i = 0; i < Marcos; i++)
    {
      if(tablaPaginasSwap[i][0] == args[1] && tablaPaginasSwap[i][1] == toCheck)
      {
        found = 1;
        for(int j = 0; j < Marcos; j++)
        {
          if(tablaPaginasMemoria[j][0] == toCompare)
          {
            tablaPaginasMemoria[j][0] = tablaPaginasSwap[i][0];
            tablaPaginasMemoria[j][1] = tablaPaginasSwap[i][1];
            int k = 0;
            for(k = 0; k < tamanoPagina; k++)
            {
               M[k+j] = tablaPaginasMemoria[j][0];
               S[i+k-1] = 0;
            }
            break;
          }
        }
      }
    }
    for(int i = 0; i < Marcos; i++)
    {
      tablaPaginasSwap[i][0] = tablaPaginasSwap[i + 1][0];
      tablaPaginasSwap[i][1] = tablaPaginasSwap[i + 1][1];
       if(timeStamp[i][0] == args[1])
       {
          timeStamp[i][1]++;
       }
    }
    totalSwapIn++;
    for(int i = 0; i < Marcos; i++)
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
  for(int i = 0; i  < Marcos; i++)
  {
     if(timeStamp[i][0] == args[1])
     {
        timeStamp[i][1] += 0.1;
     }
  }
}

void L(int toRemove) //Libera el proceso tanto del area de memoria como del area de swapping
{
  int removedCounter = 0;
  //Libera los marcos de pagina ocupados por el proceso toRemove de memoria
  for(int i = 0; i < Marcos*tamanoPagina;i++)
  {
    if(M[i] == toRemove)
    {
      M[i] = 0;
      removedCounter++;
      if(removedCounter % tamanoPagina == 0)
      {
        push(marcosLiberados, i / tamanoPagina, 3);
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
//////////////////////////////////////////////////////////////////

//Libera los marcos de pagina ocupados por el proceso toRemove del area de swapping
  for(int i = 0; i < Marcos*tamanoPagina*2;i++)
  {
    if(S[i] == toRemove)
    {
      S[i] = 0;
      removedCounter++;
      if(removedCounter % tamanoPagina == 0)
      {
        push(marcosLiberados, i / tamanoPagina, 3);
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
////////////////////////////////////////////////////////////////////////////////////
}

void F()  //Esta funcion imprime estadisticas hasta el momento
{
  //print turnaround time promedio y page faults por proceso
   
  for(int i = 0; i < Marcos; i++)
  { 
    turnAroundPromedio += timeStamp[i][1];
    if(pageFaults[i][0])
    {
      printf("Total Page Faults for process %d: %d\n", pageFaults[i][0], pageFaults[i][1]);
    }
  }
  turnAroundPromedio /= totalProcess;
  printf("Turnaround promedio: %f\n", turnAroundPromedio);
  turnAroundPromedio = 0;
////////////////////////////////////////////////////
  printf("Total Swap-out: %d\n", totalSwapOut); //Imprime el numero total de swap-outs
  printf("Total Swap-in: %d\n", totalSwapIn); //Imprime el numero total de swap-in
}

void E() //Esta funcion indica que ya no se realizaran mas operaciones y libera todo
{
  printf("Adios\n");
  end = 1;
  marcosLibres = Marcos;
  bitRef = 0;
  while(!isempty(1))
  {
    pop(entranceOrder, 1);
  }
  totalPaginas = 0;
  for(int j = 0; j < 2; j++)
  {
    for(int i = 0; i < Marcos; i++)
    {
      tablaPaginasMemoria[i][j] = 0;
      tablaPaginasSwap[i][j] = 0;
      timeStamp[i][j] = 0;
      pageFaults[i][j] = 0;
      usedTime[i][j] = 0;
    }
  }
  for(int i = 0; i < Marcos*tamanoPagina;i++)
  {
    M[i] = 0;
  }
  for(int i = 0; i < Marcos*tamanoPagina*2; i++)
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

void menu(char message[]) //Utilizado para seleccionar la funcion a utilizar
{
  if(!end)
  {
    char opr;
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

void readFile() //Utilizado para leer el archivoTrabajo-1.txt que es donde se encontraran las instrucciones a realizar
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
}

void main() //Main
{
  printf("Algoritmo FIFO\n");
  readFile();
  algRemplazo = 1; //Indica que ahora el algoritmo que se utilizara es el LRU
  end = 0;
  printf("Algoritmo LRU\n");
  readFile();
}
