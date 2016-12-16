#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include "piezas.h"

#define BUFSIZE 10
#define tp1 5.0
#define tp2 4.0
#define tp3 10.0
#define tp4 15.0
#define tp5 13.0
#define tg1 3.0
#define tg2 2.0
#define tmaxB 2.0
#define tmaxS 4.0
#define tmaxC 6.0
#define PIZENUMB 300

pthread_mutex_t buffer_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct Buffer_Circ {
  pieza values[BUFSIZE];
  int bufIN;
  int bufOUT;
  int con;
} Buffer_Circ;

void initialize(Buffer_Circ *buff){
  pthread_mutex_lock(&buffer_lock);
  buff->bufIN = 0;
  buff->bufOUT = 0;
  buff->con=0;
  pthread_mutex_unlock(&buffer_lock);
}

int get_item(pieza *x, Buffer_Circ *buff){
  /* Función que saca el valor más antiguo del buffer.
     Si funciona correctamente devuelbe 0 si no -1. */
  pthread_mutex_lock(&buffer_lock);
  if (buff->con == 0) {
    pthread_mutex_unlock(&buffer_lock);
    return -1;
  }
  else {
    *x = buff->values[buff->bufOUT];
    buff->bufOUT = (buff->bufOUT+1)%BUFSIZE;
    buff->con--;
    pthread_mutex_unlock(&buffer_lock);
    return 0;
  }
}

int put_item(pieza x, Buffer_Circ *buff){
  /* Función que introduce un valor en el buffer.
     Si funciona correctamente devuelbe 0 si no -1. */
  pthread_mutex_lock(&buffer_lock);
  if (buff->con == BUFSIZE){
    pthread_mutex_unlock(&buffer_lock);
    return -1;
  }
  else{
    buff->values[buff->bufIN] = x;
    buff->bufIN = (buff->bufIN+1)%BUFSIZE;
    buff->con++;
    pthread_mutex_unlock(&buffer_lock);
    return 0;
  }
}

bool isEmpty(Buffer_Circ *buff){
  // Función para averiguar si el buffer está bacio
  pthread_mutex_lock(&buffer_lock);
  if(buff->con == 0){
    pthread_mutex_unlock(&buffer_lock);
    return true;
  }
  else{
    pthread_mutex_unlock(&buffer_lock);
    return false;
  }
}

bool isFull(Buffer_Circ *buff){
  // Función para aberiguar si el buffer está lleno
  pthread_mutex_lock(&buffer_lock);
  if(buff->con == BUFSIZE){
    pthread_mutex_unlock(&buffer_lock);
    return true;
  }
  else{
    pthread_mutex_unlock(&buffer_lock);
    return false;
  }
}

void content(Buffer_Circ *buff){
  // Función que imprime por pantalla los elementos del buffer
  int i;
  pthread_mutex_lock(&buffer_lock);
  printf("En el almacen se encuantran las siguientes piezas: (%d)\n",buff->con);
  if (buff->con!=BUFSIZE) i = buff->bufOUT;
  else (i = buff ->bufOUT+1)%BUFSIZE;
  while(i!=buff->bufIN){
    printf("%s (%d)\t",buff->values[i].etiqueta,buff->values[i].generador);
    i = (i+1)%BUFSIZE;
  }
  printf("\n");
  pthread_mutex_unlock(&buffer_lock);
}

int num_elements(Buffer_Circ *buff){
  // Función que devuelbe el número de elemntos en el buffer
  pthread_mutex_lock(&buffer_lock);
  pthread_mutex_unlock(&buffer_lock);
  return buff->con;
}

int consult(pieza *x,Buffer_Circ *buff){
  /* Función que consulta (sin sacar) el valor más antiguo del buffer.
     Si funciona correctamente devuelbe 0 si no -1. */
  pthread_mutex_lock(&buffer_lock);
  if (buff->con == 0) {
    pthread_mutex_unlock(&buffer_lock);
    return -1;
  }
  else {
    *x = buff->values[buff->bufOUT];
    pthread_mutex_unlock(&buffer_lock);
    return 0;
  }
}
