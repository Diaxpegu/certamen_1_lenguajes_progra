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
## 2. Generador unario con subrutinas (`maquina2.tm`)

Esta máquina demuestra cómo se declara y se invoca una subrutina parametrizada. `escribir_unos(n)` recorre `n` celdas consecutivas desde la posición del cabezal. En cada celda, lee `0` o `1`, escribe `1` y avanza a la derecha.

La máquina principal hace la llamada dentro del bloque `transiciones`:

```text
q_inicio -> escribir_unos(3) -> q_final;
```

La llamada parte en `q_inicio`, usa `3` como valor del parámetro `n` y pasa a `q_final` después de la tercera escritura. Antes de ejecutar la máquina, Bison expande esta llamada en **dos estados intermedios y seis reglas de transición**. En cada paso hay una regla para leer `0` y otra para leer `1`. Durante la ejecución se aplica solo la que coincide con el símbolo bajo el cabezal.

### Comando de prueba

```bash
./turing maquina2.tm 010
```

La máquina alcanza `q_final` en tres pasos y muestra `Cinta final: 111`. La entrada `010` permite probar ambas reglas de la plantilla. Si se usa `_` como entrada, la máquina se detiene en `q_inicio` porque la subrutina no tiene una regla para leer el símbolo blanco.

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
