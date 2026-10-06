# Certamen 1: Intérprete de DSL para Máquina de Turing

**Integrantes:** 
- Diego Peña
- Nataniel Riquelme

## Descripción del Proyecto
Este proyecto es un compilador e intérprete para un Lenguaje de Dominio Específico (DSL) que permite programar y simular Máquinas de Turing. Está construido en C utilizando **Flex** para el análisis léxico y **Bison** para el análisis sintáctico.

El programa no solo reconoce la sintaxis, sino que valida la semántica (comprobando determinismo, alfabeto y existencia de estados), permite inyectar **subrutinas dinámicas** en tiempo de compilación y ejecuta la simulación mostrando la cinta paso a paso.

## Requisitos de Instalación
Para compilar este proyecto necesitas un entorno Linux/Unix con:
- `gcc`
- `flex`
- `bison`
- `make`

## Instrucciones de Compilación y Ejecución

El proyecto incluye un `Makefile` para automatizar todo el proceso[cite: 21]. Puedes usar los siguientes comandos en tu terminal:

- **`make build`**: Solo compila el código y genera el ejecutable llamado `turing`[cite: 21].
- **`make clean`**: Borra el ejecutable y los archivos generados por Flex y Bison para limpiar la carpeta[cite: 21].
- **`make run`**: Compila todo de forma automática y ejecuta la máquina principal por defecto[cite: 21].
- **`make zip`**: Limpia la carpeta y comprime los códigos fuente en el archivo `.zip` final para la entrega[cite: 21].

**Ejecución manual:**
Si quieres correr una máquina específica o pasarle una palabra inicial a la cinta, usa el ejecutable directamente:
```bash
./turing <nombre_archivo.tm> [palabra_inicial]
```
# Máquinas de Prueba Incluidas

Hemos preparado **3 archivos `.tm`** que demuestran todas las capacidades del intérprete exigidas en el certamen:

## 1. Incrementador Binario (`maquina.tm`)

Esta máquina toma un número binario, avanza hasta el final de la cadena hacia la derecha, y luego retrocede de derecha a izquierda sumándole **1**.

Durante el retroceso, aplica la regla de acarreo: cambia los `1` por `0` hasta encontrar un `0` o un espacio en blanco `_`, el cual convierte a `1` y se detiene.
## Comando de prueba

```bash
./turing maquina.tm 1011
```
**Resultado esperado:** La máquina procesará el acarreo en cadena y la cinta final mostrará `1100`.

## 2. Generador Unario con Subrutinas (`maquina2.tm`)

Esta máquina pone a prueba la **Fase 2 del certamen (Subrutinas parametrizadas)**. Contiene la invocación `escribir_unos(5);`, la cual es detectada por Bison en tiempo de compilación. El compilador en C hace un ciclo y genera **5 estados dinámicos en la memoria RAM**, enlazándolos al estado inicial y final de la máquina principal.

### Comando de prueba

Usamos `_` para iniciar con la cinta en blanco:

```bash
./turing maquina2.tm _
```
**Resultado esperado:** El cabezal avanzará por la cinta vacía escribiendo el número `1` cinco veces consecutivas, demostrando que los estados de la subrutina se expandieron correctamente.

**Cinta final:** `11111`

## 3. Inversor de Bits / Complemento a 1 (`maquina3.tm`)

Es una máquina sencilla pero efectiva que recorre la cinta completa reemplazando todos los ceros por unos, y todos los unos por ceros. Se detiene automáticamente al encontrar el primer espacio en blanco.

### Comando de prueba

```bash
./turing maquina3.tm 110010

```
**Resultado esperado:** La máquina avanzará hacia la derecha invirtiendo cada celda. La cinta final será `001101`.

# Estructura del Código Fuente

- `lexer.l`: Analizador léxico. Reconoce palabras clave, captura identificadores y convierte las cadenas numéricas en valores enteros.

- `parser.y`: Analizador sintáctico. Define la gramática, inyecta los nodos de subrutina dinámicamente y arma la lista de reglas en memoria.

- `main.c`: El intérprete. Contiene las validaciones semánticas previas y el motor `while` que simula el movimiento del cabezal sobre la cinta de 100 caracteres.

- `turing.h`: Cabecera que define la estructura `Transicion` y las variables globales compartidas.
