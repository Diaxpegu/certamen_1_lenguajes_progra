%{
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "turing.h"

int yylex(void);
void yyerror(const char *s);

// Variables globales compartidas con main.c
Transicion* cabeza_lista = NULL;
char estado_inicial[50];
char estados_finales[10][50];
int total_finales = 0;

char alfabeto[20];
int total_alfabeto = 0;
char estados[50][50];
int total_estados = 0;

// --- Estructuras para plantillas de subrutinas ---
typedef struct {
    char lee_simbolo;
    char escribe_simbolo;
    char movimiento[10];
    int es_bucle_actual; // 1 si el destino es "actual", 0 si avanza a "siguiente"
} ReglaPlantilla;

typedef struct {
    char nombre[50];
    char param[50];
    char param_repetir[50];
    ReglaPlantilla reglas[10];
    int total_reglas;
    int tiene_repetir;
} SubrutinaDef;

SubrutinaDef tabla_subrutinas[20];
int total_subrutinas = 0;

// Buffer temporal mientras Bison analiza el bloque repetir de una subrutina
SubrutinaDef sub_temp;

// Contador global para garantizar nombres únicos por cada llamada
static int contador_llamadas = 0;

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

static SubrutinaDef* buscar_subrutina(const char *nombre) {
    for (int i = 0; i < total_subrutinas; i++) {
        if (strcmp(tabla_subrutinas[i].nombre, nombre) == 0) {
            return &tabla_subrutinas[i];
        }
    }
    return NULL;
}

static void insertar_transicion(const char *origen, char lee, const char *destino, char escribe, const char *mov) {
    Transicion* nueva = (Transicion*)malloc(sizeof(Transicion));
    strcpy(nueva->estado_actual, origen);
    nueva->lee_simbolo = lee;
    strcpy(nueva->estado_nuevo, destino);
    nueva->escribe_simbolo = escribe;
    strcpy(nueva->movimiento, mov);

    nueva->siguiente = cabeza_lista;
    cabeza_lista = nueva;

    printf("Guardado en RAM: (%s, %c) -> (%s, %c, %s)\n",
           nueva->estado_actual, nueva->lee_simbolo,
           nueva->estado_nuevo, nueva->escribe_simbolo, nueva->movimiento);
}

static void formatear_estado_sub(char *dest, size_t tam, const char *nombre, int id_llamada, int indice) {
    snprintf(dest, tam, "sub_%.28s_%d_%02d", nombre, id_llamada % 1000, indice % 100);
}

static int expandir_subrutina(const char *nombre, const char *argumento, int cant_args,
                              const char *estado_entrada, const char *estado_salida) {
    SubrutinaDef *sub = buscar_subrutina(nombre);
    if (!sub) {
        fprintf(stderr, "Error: subrutina '%s' no declarada.\n", nombre);
        return 0;
    }

    // Validación: Rechazar subrutinas sin bloque repetir
    if (!sub->tiene_repetir || sub->total_reglas == 0) {
        fprintf(stderr, "Error: la subrutina '%s' no tiene un bloque 'repetir' valido.\n", nombre);
        return 0;
    }

    // Validación: Cantidad exacta de argumentos (debe ser 1)
    if (cant_args != 1) {
        fprintf(stderr, "Error: la subrutina '%s' espera 1 argumento, pero recibio %d.\n", nombre, cant_args);
        return 0;
    }

    char *fin = NULL;
    errno = 0;
    long cantidad = strtol(argumento, &fin, 10);
    if (errno != 0 || fin == argumento || *fin != '\0' || cantidad <= 0
        || (cantidad - 1) > 50 - total_estados) {
        fprintf(stderr, "Error: argumento de expansion invalido ('%s') para la subrutina '%s'.\n", argumento, nombre);
        return 0;
    }

    // Identificador único para esta llamada específica
    int id_llamada = contador_llamadas++;

    // 1. Registrar exactamente (n - 1) estados internos únicos para esta llamada
    for (int i = 0; i < cantidad - 1; i++) {
        char estado[50];
        formatear_estado_sub(estado, sizeof(estado), nombre, id_llamada, i);
        if (!registrar_estado(estado)) return 0;
    }

    // 2. Generar las transiciones conectando estado_entrada -> [internos] -> estado_salida
    for (int i = 0; i < cantidad; i++) {
        char origen[50], siguiente_estado[50];

        if (i == 0) {
            snprintf(origen, sizeof(origen), "%s", estado_entrada);
        } else {
            formatear_estado_sub(origen, sizeof(origen), nombre, id_llamada, i - 1);
        }

        if (i == cantidad - 1) {
            snprintf(siguiente_estado, sizeof(siguiente_estado), "%s", estado_salida);
        } else {
            formatear_estado_sub(siguiente_estado, sizeof(siguiente_estado), nombre, id_llamada, i);
        }

        for (int r = 0; r < sub->total_reglas; r++) {
            const char *destino = sub->reglas[r].es_bucle_actual ? origen : siguiente_estado;
            insertar_transicion(origen,
                                sub->reglas[r].lee_simbolo,
                                destino,
                                sub->reglas[r].escribe_simbolo,
                                sub->reglas[r].movimiento);
        }
    }

    return 1;
}
%}

%code requires {
    typedef struct {
        char valor[50];
        int cantidad;
    } ListaArgs;
}

%union {
    char* str;
    int num;
    ListaArgs args;
}

%token TOKEN_MAQUINA TOKEN_TRANSICIONES TOKEN_DER TOKEN_IZQ TOKEN_QUIETO TOKEN_FLECHA
%token TOKEN_ALFABETO TOKEN_ESTADOS TOKEN_INICIAL TOKEN_FINALES 
%token TOKEN_SUBRUTINA TOKEN_USA
%token <str> TOKEN_ID
%token <num> TOKEN_NUMERO
%token TOKEN_REPETIR TOKEN_VECES

%type <str> parametro lista_params argumento direccion
%type <args> lista_argumentos
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
        // Si hay bloque repetir, su variable debe coincidir con el parámetro declarado
        if (sub_temp.tiene_repetir) {
            if ($4 == NULL || strcmp(sub_temp.param_repetir, $4) != 0) {
                fprintf(stderr, "Error: en '%s', repetir(%s) no coincide con el parametro declarado '%s'.\n",
                        $2, sub_temp.param_repetir, $4 ? $4 : "ninguno");
                memset(&sub_temp, 0, sizeof(SubrutinaDef));
                YYERROR;
            }
        }

        if (total_subrutinas < 20) {
            strcpy(sub_temp.nombre, $2);
            strcpy(sub_temp.param, $4 ? $4 : "");
            tabla_subrutinas[total_subrutinas++] = sub_temp;
            printf("Subrutina declarada con parametros: %s(%s)\n", sub_temp.nombre, sub_temp.param);
        } else {
            fprintf(stderr, "Error: limite de subrutinas alcanzado.\n");
            YYERROR;
        }
        memset(&sub_temp, 0, sizeof(SubrutinaDef));
    }
    | TOKEN_SUBRUTINA TOKEN_ID '{' cuerpo_subrutina '}'
    {
        if (sub_temp.tiene_repetir) {
            fprintf(stderr, "Error: subrutina '%s' usa repetir(%s) pero no declara parametros.\n",
                    $2, sub_temp.param_repetir);
            memset(&sub_temp, 0, sizeof(SubrutinaDef));
            YYERROR;
        }

        if (total_subrutinas < 20) {
            strcpy(sub_temp.nombre, $2);
            sub_temp.param[0] = '\0';
            tabla_subrutinas[total_subrutinas++] = sub_temp;
            printf("Subrutina declarada sin parametros: %s\n", sub_temp.nombre);
        } else {
            fprintf(stderr, "Error: limite de subrutinas alcanzado.\n");
            YYERROR;
        }
        memset(&sub_temp, 0, sizeof(SubrutinaDef));
    }
    ;

cuerpo_subrutina:
    /* vacio */
    | bloque_transiciones
    | bloque_repetir
    ;

bloque_repetir:
    TOKEN_REPETIR '(' TOKEN_ID ')' TOKEN_VECES '{' lista_plantillas '}'
    {
        sub_temp.tiene_repetir = 1;
        snprintf(sub_temp.param_repetir, sizeof(sub_temp.param_repetir), "%s", $3);
    }
    ;

lista_plantillas:
    transicion_plantilla
    | lista_plantillas transicion_plantilla
    ;

transicion_plantilla:
    TOKEN_ID ',' simbolo_regla TOKEN_FLECHA TOKEN_ID ',' simbolo_regla ',' direccion ';'
    {
        if (sub_temp.total_reglas < 10) {
            int idx = sub_temp.total_reglas++;
            sub_temp.reglas[idx].lee_simbolo = $3;
            sub_temp.reglas[idx].escribe_simbolo = $7;
            strcpy(sub_temp.reglas[idx].movimiento, $9);
            sub_temp.reglas[idx].es_bucle_actual = (strcmp($5, "actual") == 0);
        } else {
            fprintf(stderr, "Error: maximo 10 reglas por bloque repetir.\n");
            YYERROR;
        }
    }
    ;

lista_params:
    /* vacio */                  { $$ = NULL; }
    | parametro                  { $$ = $1; }
    | lista_params ',' parametro { $$ = $1; }
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
        if (!buscar_subrutina($2)) {
            fprintf(stderr, "Error: subrutina '%s' no declarada.\n", $2);
            YYERROR;
        }
        printf("Declaracion de uso validada: usa %s(...);\n", $2);
    }
    | TOKEN_USA TOKEN_ID ';'
    {
        if (!buscar_subrutina($2)) {
            fprintf(stderr, "Error: subrutina '%s' no declarada.\n", $2);
            YYERROR;
        }
        printf("Declaracion de uso validada: usa %s;\n", $2);
    }
    ;

lista_argumentos:
    argumento
    {
        snprintf($$.valor, sizeof($$.valor), "%s", $1);
        $$.cantidad = 1;
    }
    | lista_argumentos ',' argumento
    {
        $$ = $1;
        $$.cantidad++;
    }
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
        registrar_estado($1);
    }
    | lista_estados_ids ',' TOKEN_ID
    {
        registrar_estado($3);
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

direccion:
    TOKEN_DER      { $$ = "DER"; }
    | TOKEN_IZQ    { $$ = "IZQ"; }
    | TOKEN_QUIETO { $$ = "QUIETO"; }
    ;

regla:
    TOKEN_ID ',' simbolo_regla TOKEN_FLECHA TOKEN_ID ',' simbolo_regla ',' direccion ';'
    {
        insertar_transicion($1, $3, $5, $7, $9);
    }
    | TOKEN_ID TOKEN_FLECHA TOKEN_ID '(' lista_argumentos ')' TOKEN_FLECHA TOKEN_ID ';'
    {
        if (!expandir_subrutina($3, $5.valor, $5.cantidad, $1, $8)) YYERROR;
    }
    ;
%%

void yyerror(const char *s){
    fprintf(stderr, "Error en la sintaxis: %s\n", s);
}