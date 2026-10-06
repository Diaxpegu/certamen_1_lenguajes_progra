%{
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "turing.h"

int yylex(void);
void yyerror(const char *s);

// Definición de variables globales utilizadas por el parser y el main
Transicion* cabeza_lista = NULL;
char estado_inicial[50];
char estados_finales[10][50];
int total_finales = 0;

char alfabeto[20];
int total_alfabeto = 0;
char estados[50][50];
int total_estados = 0;

static int registrar_estado(const char *estado) {
    for (int i = 0; i < total_estados; i++) {
        if (strcmp(estados[i], estado) == 0) return 1;
    }
    if (total_estados >= 50) {
        fprintf(stderr, "Error: se excedio el maximo de 50 estados.\n");
        return 0;
    }
    if (snprintf(estados[total_estados], sizeof(estados[total_estados]), "%s", estado)
        >= (int)sizeof(estados[total_estados])) {
        fprintf(stderr, "Error: nombre de estado demasiado largo: '%s'.\n", estado);
        return 0;
    }
    total_estados++;
    return 1;
}

static int registrar_estados_subrutina(const char *nombre, const char *argumento) {
    char *fin = NULL;
    errno = 0;
    long cantidad = strtol(argumento, &fin, 10);
    if (errno != 0 || fin == argumento || *fin != '\0' || cantidad < 0
        || cantidad > 50 - total_estados) {
        fprintf(stderr, "Error: argumento de expansion invalido para la subrutina '%s'.\n", nombre);
        return 0;
    }

    for (int i = 0; i < cantidad; i++) {
        char estado[50];
        int longitud = snprintf(estado, sizeof(estado), "sub_%.42s", nombre);
        if (longitud < 0 || (size_t)longitud > sizeof(estado) - 4) return 0;
        estado[longitud++] = '_';
        if (i >= 10) estado[longitud++] = (char)('0' + i / 10);
        estado[longitud++] = (char)('0' + i % 10);
        estado[longitud] = '\0';
        if (!registrar_estado(estado)) return 0;
    }
    return 1;
}
%}

%union {
    char* str;
    int num;
}

%token TOKEN_MAQUINA TOKEN_TRANSICIONES TOKEN_DER TOKEN_IZQ TOKEN_QUIETO TOKEN_FLECHA
%token TOKEN_ALFABETO TOKEN_ESTADOS TOKEN_INICIAL TOKEN_FINALES 
%token TOKEN_SUBRUTINA TOKEN_USA
%token <str> TOKEN_ID
%token <num> TOKEN_NUMERO

%type <str> parametro argumento lista_argumentos
%type <num> simbolo_regla

%%
programa:
    lista_subrutinas maquina_def
    ;

lista_subrutinas:
    /* vacio */
    | lista_subrutinas declaracion_subrutina
    ;

declaracion_subrutina:
    TOKEN_SUBRUTINA TOKEN_ID '(' lista_params ')' '{' cuerpo_subrutina '}'
    {
        printf("Subrutina declarada con parametros: %s\n", $2);
    }
    | TOKEN_SUBRUTINA TOKEN_ID '{' cuerpo_subrutina '}'
    {
        printf("Subrutina declarada sin parametros: %s\n", $2);
    }
    ;

cuerpo_subrutina:
    /* vacio */
    | bloque_transiciones
    ;

lista_params:
    /* vacio */
    | parametro
    | lista_params ',' parametro
    ;

parametro:
    TOKEN_ID { $$ = $1; }
    ;

maquina_def:
    TOKEN_MAQUINA TOKEN_ID '{' bloque_alfabeto bloque_estados config_inicial config_finales contenido_maquina '}'
    {
        printf("Analisis sintactico exitoso por la maquina: %s\n", $2);
    }
    ;

contenido_maquina:
    /* vacio */
    | contenido_maquina bloque_transiciones
    | contenido_maquina invocacion_subrutina
    ;

invocacion_subrutina:
    TOKEN_USA TOKEN_ID '(' lista_argumentos ')' ';'
    {
        if (!registrar_estados_subrutina($2, $4)) YYERROR;
        printf("Invocacion parametrizada de subrutina: usa %s(...)\n", $2);
    }
    | TOKEN_USA TOKEN_ID ';'
    {
        printf("Invocacion simple de subrutina: usa %s;\n", $2);
    }
    ;

lista_argumentos:
    argumento
    | lista_argumentos ',' argumento { $$ = $1; }
    ;

argumento:
    TOKEN_ID
    {
        $$ = $1;
    }
    | TOKEN_NUMERO
    {
        char buffer[20];
        snprintf(buffer, sizeof(buffer), "%d", $1);
        $$ = strdup(buffer);
    }
    ;

bloque_alfabeto:
    TOKEN_ALFABETO '{' lista_simbolos '}'
    ;

lista_simbolos:
    simbolo_alfabeto
    | lista_simbolos ',' simbolo_alfabeto
    ;

simbolo_alfabeto:
    TOKEN_ID
    {
        alfabeto[total_alfabeto++] = $1[0];
    }
    | TOKEN_NUMERO
    {
        alfabeto[total_alfabeto++] = '0' + ($1 % 10);
    }
    ;

bloque_estados:
    TOKEN_ESTADOS '{' lista_estados_ids '}'
    ;

lista_estados_ids:
    TOKEN_ID
    {
        strcpy(estados[total_estados++], $1);
    }
    | lista_estados_ids ',' TOKEN_ID
    {
        strcpy(estados[total_estados++], $3);
    }
    ;

config_inicial:
    TOKEN_INICIAL ':' TOKEN_ID ';'
    {
        strcpy(estado_inicial, $3);
        printf("Estado inicial guardado: %s\n", estado_inicial);
    }
    ;

config_finales:
    TOKEN_FINALES ':' '{' lista_ids_finales '}' ';'
    ;

lista_ids_finales:
    TOKEN_ID
    {
        strcpy(estados_finales[total_finales], $1);
        total_finales++;
    }
    | lista_ids_finales ',' TOKEN_ID
    {
        strcpy(estados_finales[total_finales], $3);
        total_finales++;
    }
    ;

bloque_transiciones:
    TOKEN_TRANSICIONES '{' lista_reglas '}'
    ;               

lista_reglas:
    regla 
    | lista_reglas regla 
    ;

simbolo_regla:
    TOKEN_ID       { $$ = $1[0]; }
    | TOKEN_NUMERO { $$ = '0' + ($1 % 10); }
    ;

regla:
     TOKEN_ID ',' simbolo_regla TOKEN_FLECHA TOKEN_ID ',' simbolo_regla ',' TOKEN_DER ';'
     {
        Transicion* nueva = (Transicion*)malloc(sizeof(Transicion));
        strcpy(nueva->estado_actual, $1);
        nueva->lee_simbolo = $3;
        strcpy(nueva->estado_nuevo, $5);
        nueva->escribe_simbolo = $7;
        strcpy(nueva->movimiento, "DER");

        nueva->siguiente = cabeza_lista;
        cabeza_lista = nueva;

        printf("Guardado en RAM: (%s, %c) -> (%s, %c, DER)\n",
              nueva->estado_actual, nueva->lee_simbolo,
              nueva->estado_nuevo, nueva->escribe_simbolo);
     }
     |
     TOKEN_ID ',' simbolo_regla TOKEN_FLECHA TOKEN_ID ',' simbolo_regla ',' TOKEN_IZQ ';'
     {
        Transicion* nueva = (Transicion*)malloc(sizeof(Transicion));
        strcpy(nueva->estado_actual, $1);
        nueva->lee_simbolo = $3;
        strcpy(nueva->estado_nuevo, $5);
        nueva->escribe_simbolo = $7;
        strcpy(nueva->movimiento, "IZQ");

        nueva->siguiente = cabeza_lista;
        cabeza_lista = nueva;

        printf("Guardado en RAM: (%s, %c) -> (%s, %c, IZQ)\n",
              nueva->estado_actual, nueva->lee_simbolo,
              nueva->estado_nuevo, nueva->escribe_simbolo);
     }
     |
     TOKEN_ID ',' simbolo_regla TOKEN_FLECHA TOKEN_ID ',' simbolo_regla ',' TOKEN_QUIETO ';'
     {
        Transicion* nueva = (Transicion*)malloc(sizeof(Transicion));
        strcpy(nueva->estado_actual, $1);
        nueva->lee_simbolo = $3;
        strcpy(nueva->estado_nuevo, $5);
        nueva->escribe_simbolo = $7;
        strcpy(nueva->movimiento, "QUIETO");

        nueva->siguiente = cabeza_lista;
        cabeza_lista = nueva;

        printf("Guardado en RAM: (%s, %c) -> (%s, %c, QUIETO)\n",
              nueva->estado_actual, nueva->lee_simbolo,
              nueva->estado_nuevo, nueva->escribe_simbolo);
     }
     ;
%%

void yyerror(const char *s){
    fprintf(stderr, "Error en la sintaxis: %s\n", s);
}
