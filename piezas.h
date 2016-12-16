#include <time.h>

typedef struct pieza {
  long tIN;
  long tGen;
  float tBarnizado;
  float tSecado;
  float tCocido;
  float tTotal;
  int generador;
  char etiqueta[20];
} pieza;
