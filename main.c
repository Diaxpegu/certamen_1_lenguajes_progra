#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "turing.h"

extern int yyparse(void);
extern FILE *yyin;

static int es_estado_declarado_o_subrutina(const char *estado);

// Funcion para ver si la maquina tiene sentido antes de hacerla correr
int validar_semantica() {
    int blanco_ok = 0;
    for (int i = 0; i < total_alfabeto; i++) {
        if (alfabeto[i] == '_') blanco_ok = 1;
    }
    if (!blanco_ok) {
        fprintf(stderr, "Error: Falta el simbolo blanco '_' en el alfabeto.\n");
        return 0;
    }

    // 1. Revisamos si el estado inicial existe
    int inicial_ok = 0;
    for (int i = 0; i < total_estados; i++) {
        if (strcmp(estados[i], estado_inicial) == 0) inicial_ok = 1;
    }
    if (!inicial_ok) {
        fprintf(stderr, "Error: El estado inicial '%s' no esta en la lista de estados.\n", estado_inicial);
        return 0;
    }

    // 2. Revisamos si los estados finales existen
    for (int i = 0; i < total_finales; i++) {
        int final_ok = 0;
        for (int j = 0; j < total_estados; j++) {
            if (strcmp(estados[j], estados_finales[i]) == 0) final_ok = 1;
        }
        if (!final_ok) {
            fprintf(stderr, "Error: Pusiste un estado final '%s' que no esta declarado.\n", estados_finales[i]);
            return 0;
        }
    }

    // 3. Revisamos que las reglas de transicion usen cosas que existen
    Transicion* t1 = cabeza_lista;
    while (t1 != NULL) {
        
        int act_ok = es_estado_declarado_o_subrutina(t1->estado_actual);
        if (!act_ok) {
            fprintf(stderr, "Error: Una regla usa el estado actual '%s' y no existe.\n", t1->estado_actual);
            return 0;
        }

        int nue_ok = es_estado_declarado_o_subrutina(t1->estado_nuevo);
        if (!nue_ok) {
            fprintf(stderr, "Error: Una regla manda al estado nuevo '%s' y no existe.\n", t1->estado_nuevo);
            return 0;
        }

        // Revisamos que los simbolos esten en el alfabeto
        int lee_ok = 0, escribe_ok = 0;
        for (int j = 0; j < total_alfabeto; j++) {
            if (alfabeto[j] == t1->lee_simbolo) lee_ok = 1;
            if (alfabeto[j] == t1->escribe_simbolo) escribe_ok = 1;
        }
        if (!lee_ok) {
            fprintf(stderr, "Error: Tratas de leer el simbolo '%c' pero no esta en el alfabeto.\n", t1->lee_simbolo);
            return 0;
        }
        if (!escribe_ok) {
            fprintf(stderr, "Error: Tratas de escribir el simbolo '%c' pero no esta en el alfabeto.\n", t1->escribe_simbolo);
            return 0;
        }

        // 4. Verificamos que sea determinista (que no hayan dos reglas para lo mismo)
        Transicion* t2 = t1->siguiente;
        while (t2 != NULL) {
            if (strcmp(t1->estado_actual, t2->estado_actual) == 0 && t1->lee_simbolo == t2->lee_simbolo) {
                fprintf(stderr, "Error: Hay reglas duplicadas o choque de decisiones en (%s, %c).\n", 
                        t1->estado_actual, t1->lee_simbolo);
                return 0;
            }
            t2 = t2->siguiente;
        }

        t1 = t1->siguiente;
    }

    printf("Validaciones listas, todo ok con las reglas.\n");
    return 1;
}

// Revisa si llegamos a la meta
int es_estado_final(char* estado) {
    for (int i = 0; i < total_finales; i++) {
        if (strcmp(estados_finales[i], estado) == 0) return 1;
    }
    return 0;
}

// Revisa si es un estado normal o uno creado internamente por una subrutina
static int es_estado_declarado_o_subrutina(const char *estado) {
    for (int i = 0; i < total_estados; i++) {
        if (strcmp(estados[i], estado) == 0) return 1;
    }
    return strncmp(estado, "sub_", 4) == 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Te falto el archivo. Usa: %s <archivo.tm> [cinta_inicial]\n", argv[0]);
        return 1;
    }

    yyin = fopen(argv[1], "r");
    if (!yyin) {
        perror("No se pudo abrir el archivo .tm");
        return 1;
    }

    if (yyparse() != 0) {
        fclose(yyin);
        fprintf(stderr, "Fallo al leer la estructura del archivo.\n");
        return 1;
    }
    fclose(yyin);

    // Hacemos el chequeo antes de mover la maquina
    if (!validar_semantica()) {
        fprintf(stderr, "La maquina tiene errores en sus reglas. Se cancela la simulacion.\n");
        return 1;
    }

    // Inicio de la simulación
    printf("\nArrancando la maquina\n");

    // Preparamos la memoria de la cinta con blancos
    char cinta[100];
    for (int i = 0; i < 100; i++) cinta[i] = '_'; 

    // Partimos en el medio para no caernos del arreglo al tiro
    int cabezal = 50; 
    
    // Si pasaron una palabra por la terminal, la metemos a la cinta
    if (argc > 2) {
        char* entrada = argv[2];
        int len = strlen(entrada);
        if (len > 50) {
            fprintf(stderr, "La palabra es muy larga para el espacio de la cinta.\n");
            return 1;
        }
        for (int i = 0; i < len; i++) {
            cinta[cabezal + i] = entrada[i];
        }
    } else {
        // RESTAURADO: Palabra por defecto si corres solo "make run"
        cinta[cabezal] = '0';
        cinta[cabezal+1] = '1';
        cinta[cabezal+2] = '0';
    }

    char estado_actual[50];
    strcpy(estado_actual, estado_inicial);

    int paso = 0;
    printf("Paso | Estado         | Lee | Accion                 | Cinta parcial\n");
    printf("------------------------------------------------------------------------------------------------------\n");

    // Ciclo de vida de la maquina
    while (1) {
        // Control de seguridad por si la maquina se vuelve loca y se sale del arreglo
        if (cabezal < 0 || cabezal >= 100) {
            fprintf(stderr, "\nSe acabo el espacio en la cinta (posicion %d).\n", cabezal);
            break;
        }

        char simbolo_actual = cinta[cabezal];

        // Buscamos que regla nos sirve ahora
        Transicion* t = cabeza_lista;
        Transicion* regla_aplicada = NULL;

        while (t != NULL) {
            if (strcmp(t->estado_actual, estado_actual) == 0 && t->lee_simbolo == simbolo_actual) {
                regla_aplicada = t;
                break;
            }
            t = t->siguiente;
        }

        // Si no pillamos ninguna regla, la maquina se tranca
        if (regla_aplicada == NULL) {
            printf("%4d | %14s |  %c  | No hay regla aplicable | Detenida\n", 
                   paso, estado_actual, simbolo_actual);
            printf("\nFallo: La maquina se trabo sin llegar a un estado final.\n");
            break;
        }

        // Mostramos lo que va a hacer
        printf("%4d | %14s |  %c  | Escribe %c, Mueve %-6s | ", 
               paso, estado_actual, simbolo_actual, 
               regla_aplicada->escribe_simbolo, regla_aplicada->movimiento);
        
        // Printeo de la ventana que persigue al cabezal (11 celdas)
        for (int i = cabezal - 5; i <= cabezal + 5; i++) {
            if (i >= 0 && i < 100) {
                if (i == cabezal) printf("[%c]", cinta[i]);
                else printf(" %c ", cinta[i]);
            }
        }
        printf("\n");

        // Aplicamos la regla: cambiamos el simbolo y actualizamos el estado
        cinta[cabezal] = regla_aplicada->escribe_simbolo;
        strcpy(estado_actual, regla_aplicada->estado_nuevo);

        // Movemos la aguja
        if (strcmp(regla_aplicada->movimiento, "DER") == 0) {
            cabezal++;
        } else if (strcmp(regla_aplicada->movimiento, "IZQ") == 0) {
            cabezal--;
        }

        // Verificamos si tocamos la meta
        if (es_estado_final(estado_actual)) {
            printf("\nTerminó, llego al estado final '%s' en %d pasos.\n", estado_actual, paso + 1);
            printf("Cinta final: ");
            for (int i = 0; i < 100; i++) {
                if (cinta[i] != '_') printf("%c", cinta[i]);
            }
            printf("\n");
            break;
        }

        // Control para evitar bucles infinitos (maximo 1000 pasos)
        paso++;
        if (paso > 1000) {
            printf("\nSe corto por seguridad: paso los 1000 movimientos (posible bucle infinito).\n");
            break;
        }
    }

    return 0;
}
