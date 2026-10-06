#ifndef TURING_H
#define TURING_H

//Estructura clasica de C para modelar cada regla de la maquina de Turing 
typedef struct Transicion {
  char estado_actual[50];
  char lee_simbolo;
  char estado_nuevo[50];
  char escribe_simbolo;
  char movimiento[10];
  struct Transicion* siguiente; // Puntero para armar la lista enlazada dinámica
} Transicion;

//Variables globales para el parser y el main
extern Transicion* cabeza_lista;
extern char estado_inicial[50];
// Simplificar los estados iniciales en un arreglo o lista simple 
extern char estados_finales[10][50];
extern int total_finales;


// Validacion semantica
extern char alfabeto[20];
extern int total_alfabeto;
extern char estados [50][50];
extern int total_estados;

// Prototipo de validacion semantica
int validar_semantica();

#endif
