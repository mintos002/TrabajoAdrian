#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include "sBufferCirc.h"

#define MS2S 1.0E6

sem_t huecos1, prod1, huecos2, prod2, huecos3, prod3, huecos4, prod4, huecos5, prod5;
Buffer_Circ almacen1, almacen2, almacen3, almacen4, almacen5;
int conEntrada = 0;
int conSalida = 0;
int conDescar = 0;
float tMax=0.0F, tMin=9E9, tAberage=0.0;
pthread_mutex_t conEntrada_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t conSalida_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t conDescar_lock = PTHREAD_MUTEX_INITIALIZER;

long getCurrentMicroseconds(){
  struct timespec currentTime;
  clock_gettime(CLOCK_MONOTONIC, &currentTime);
  return (currentTime.tv_sec)*1000000 + (currentTime.tv_nsec) / 1000;
}

void *generador1(){
  pieza pieza = {.generador = 1};
  while(conEntrada < PIZENUMB){
    pieza.tIN = getCurrentMicroseconds();
    pieza.tGen = pieza.tIN;
    pthread_mutex_lock(&conEntrada_lock);
    sprintf(pieza.etiqueta,"pieza%d",conEntrada);
    conEntrada++;
    pthread_mutex_unlock(&conEntrada_lock);
    sem_wait(&huecos1);
    put_item(pieza, &almacen1);
    sem_post(&prod1);
    sleep(tg1);
  }
}

void *generador2(void *arg){
  pieza pieza = {.generador = 2};

  while(conEntrada <= PIZENUMB){
    pieza.tIN = getCurrentMicroseconds();
    pieza.tGen = pieza.tIN;
    pthread_mutex_lock(&conEntrada_lock);
    sprintf(pieza.etiqueta,"pieza%d",conEntrada);
    conEntrada++;
    pthread_mutex_unlock(&conEntrada_lock);
    sem_wait(&huecos2);
    put_item(pieza, &almacen2);
    sem_post(&prod2);
    sleep(tg2);
  }
}

void *barnizadoSecado1(void *arg){
  pieza pieza;
  float tBarnizado;

  while(1){
    sem_wait(&prod1);
    consult(&pieza,&almacen1);
    sem_post(&prod1);
    tBarnizado = (getCurrentMicroseconds()-pieza.tIN)/MS2S;
    if (tBarnizado >= tp1) {
      sem_wait(&prod1);
      get_item(&pieza,&almacen1);
      sem_post(&huecos1);
      if (tBarnizado > tp1+tmaxB){
	pthread_mutex_lock(&conEntrada_lock);
	conEntrada--;
	pthread_mutex_unlock(&conEntrada_lock);
	pthread_mutex_lock(&conDescar_lock);
	conDescar++;
	pthread_mutex_unlock(&conDescar_lock);
      }
      else{
	pieza.tBarnizado = tBarnizado;
	pieza.tIN = getCurrentMicroseconds();
	sem_wait(&huecos3);
	put_item(pieza,&almacen3);
	sem_post(&prod3);
      }
    }
    usleep(500000);
  }
}

void *barnizadoSecado2(void *arg){
  pieza pieza;
  float tBarnizado;

  while(1){
    sem_wait(&prod2);
    consult(&pieza,&almacen2);
    sem_post(&prod2);
    tBarnizado = (getCurrentMicroseconds()-pieza.tIN)/MS2S;
    if (tBarnizado >= tp2) {
      sem_wait(&prod2);
      get_item(&pieza,&almacen2);
      sem_post(&huecos2);
      if (tBarnizado > tp2 + tmaxB){
	pthread_mutex_lock(&conEntrada_lock);
	conEntrada--;
	pthread_mutex_unlock(&conEntrada_lock);
	pthread_mutex_lock(&conDescar_lock);
	conDescar++;
	pthread_mutex_unlock(&conDescar_lock);
      }
      else {
	pieza.tBarnizado = tBarnizado;
	pieza.tIN = getCurrentMicroseconds();
	sem_wait(&huecos3);
	put_item(pieza,&almacen3);
	sem_post(&prod3);
      }
    }
    usleep(500000);
  }
}

void *secadoCocido(void *arg){
  pieza pieza;
  float tSecado;

  while(1){
    sem_wait(&prod3);
    consult(&pieza,&almacen3);
    sem_post(&prod3);
    tSecado = (getCurrentMicroseconds()-pieza.tIN)/MS2S;
    if (tSecado >= tp3) {
      sem_wait(&prod3);
      get_item(&pieza,&almacen3);
      sem_post(&huecos3);
      pieza.tSecado = tSecado;
      pieza.tIN = getCurrentMicroseconds();
      if (tSecado > tp3 + tmaxS){
	pthread_mutex_lock(&conEntrada_lock);
	conEntrada--;
	pthread_mutex_unlock(&conEntrada_lock);
	pthread_mutex_lock(&conDescar_lock);
	conDescar++;
	pthread_mutex_unlock(&conDescar_lock);
      }
      else{
	if (pieza.generador == 1){
	  sem_wait(&huecos4);
	  put_item(pieza,&almacen4);
	  sem_post(&prod4);
	}
	else if (pieza.generador == 2){
	  sem_wait(&huecos5);
	  put_item(pieza,&almacen5);
	  sem_post(&prod5);
	}
      }
    }
    usleep(500000);
  }
}

void *consumidor1(void *arg){
  pieza pieza;
  float tCocido;

  while(conSalida <= PIZENUMB-1){
    sem_wait(&prod4);
    consult(&pieza,&almacen4);
    sem_post(&prod4);
    tCocido = (getCurrentMicroseconds() - pieza.tIN)/MS2S;
    if (tCocido >= tp4){
      sem_wait(&prod4);
      get_item(&pieza,&almacen4);
      sem_post(&huecos4);
      if (tCocido > tp4 + tmaxC){
	pthread_mutex_lock(&conEntrada_lock);
	conEntrada--;
	pthread_mutex_unlock(&conEntrada_lock);
	pthread_mutex_lock(&conDescar_lock);
	conDescar++;
	pthread_mutex_unlock(&conDescar_lock);
      }
      else{
	pieza.tCocido = tCocido;
	pieza.tTotal = (getCurrentMicroseconds()-pieza.tGen)/MS2S;
	pthread_mutex_lock(&conSalida_lock);
	conSalida++;
	pthread_mutex_unlock(&conSalida_lock);
	tAberage += pieza.tTotal;
	if (pieza.tTotal < tMin) tMin = pieza.tTotal;
	if (pieza.tTotal > tMax) tMax = pieza.tTotal;
      }
    }
  }
}

void *consumidor2(void *arg){
  pieza pieza;
  float tCocido;
  
  while(conSalida <= PIZENUMB-1){
    sem_wait(&prod5);
    consult(&pieza,&almacen5);
    sem_post(&prod5);
    tCocido = (getCurrentMicroseconds()-pieza.tIN)/MS2S;
    if (tCocido >= tp5){
      sem_wait(&prod5);
      get_item(&pieza,&almacen5);
      sem_post(&huecos5);
      if (tCocido > tp5 + tmaxC){
	pthread_mutex_lock(&conEntrada_lock);
	conEntrada--;
	pthread_mutex_unlock(&conEntrada_lock);
	pthread_mutex_lock(&conDescar_lock);
	conDescar++;
	pthread_mutex_unlock(&conDescar_lock);
      }
      else {
	pieza.tCocido = tCocido;
	pieza.tTotal = (getCurrentMicroseconds()-pieza.tGen)/MS2S;
	pthread_mutex_lock(&conSalida_lock);
	conSalida++;
	pthread_mutex_unlock(&conSalida_lock);
	tAberage += pieza.tTotal;
	if (pieza.tTotal < tMin) tMin = pieza.tTotal;
	if (pieza.tTotal > tMax) tMax = pieza.tTotal;
      }
    }
  }
}

void *monitor(void *arg){
  while(1){
    printf("=================================================================\n");
    printf("(Almacén1) ");
    content(&almacen1);
    printf("-----------------------------------------------------------------\n");
    printf("(Almacén2) ");
    content(&almacen2);
    printf("-----------------------------------------------------------------\n");
    printf("(Almacén3) ");
    content(&almacen3);
    printf("-----------------------------------------------------------------\n");
    printf("(Almacén4) ");
    content(&almacen4);
    printf("-----------------------------------------------------------------\n");
    printf("(Almacén5) ");
    content(&almacen5);
    printf("-----------------------------------------------------------------\n");
    sleep(1);
  }
}

int main(){
  pthread_t hilo_generador1, hilo_generador2, hilo_barnizadoSecado1, hilo_barnizadoSecado2, hilo_secadoCocido, hilo_consumidor1, hilo_consumidor2, hilo_monitor;
  pthread_attr_t attr;

  pthread_attr_init(&attr);

  initialize(&almacen1);
  initialize(&almacen2);
  initialize(&almacen3);
  initialize(&almacen4);
  initialize(&almacen5);

  sem_init(&huecos1,0,BUFSIZE);
  sem_init(&huecos2,0,BUFSIZE);
  sem_init(&huecos3,0,BUFSIZE);
  sem_init(&huecos4,0,BUFSIZE);
  sem_init(&huecos5,0,BUFSIZE);
  sem_init(&prod1,0,0);
  sem_init(&prod2,0,0);
  sem_init(&prod3,0,0);
  sem_init(&prod4,0,0);
  sem_init(&prod5,0,0);
  
  pthread_create(&hilo_generador1, &attr, generador1, NULL);
  pthread_create(&hilo_generador2,&attr,generador2,NULL);
  pthread_create(&hilo_barnizadoSecado1,&attr,barnizadoSecado1,NULL);
  pthread_create(&hilo_barnizadoSecado2,&attr,barnizadoSecado2,NULL);
  pthread_create(&hilo_secadoCocido,&attr,secadoCocido,NULL);
  pthread_create(&hilo_consumidor1,&attr,consumidor1,NULL);
  pthread_create(&hilo_consumidor2,&attr,consumidor2,NULL);
  pthread_create(&hilo_monitor,&attr,monitor,NULL);

  pthread_join(hilo_consumidor1,NULL);
  pthread_join(hilo_consumidor2,NULL);

  printf("\n\n\n\n");
  printf("-----------------------------------------------------------------\n");
  printf("------------------RESULTADPS DE LA SIMULACIÓN--------------------\n");
  printf("-----------------------------------------------------------------\n\n");
  printf("Tiempo mínimo: %f s\n",tMin);
  printf("Tiempo máximo: %f s\n",tMax);
  printf("Tiempo medio:  %f s\n",tAberage/(float)PIZENUMB);
  printf("Número de piezas descartadas: %d\n", conDescar);
  printf("\n--------------------FINAL DE LA APLICACIÓN-----------------------\n");
  return 0;
}
