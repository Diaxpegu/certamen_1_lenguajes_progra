/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "parser.y"

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

//  Estructuras para plantillas de subrutinas 
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

    //Generar las transiciones conectando estado_entrada -> [internos] -> estado_salida
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

#line 233 "parser.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "parser.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_TOKEN_MAQUINA = 3,              /* TOKEN_MAQUINA  */
  YYSYMBOL_TOKEN_TRANSICIONES = 4,         /* TOKEN_TRANSICIONES  */
  YYSYMBOL_TOKEN_DER = 5,                  /* TOKEN_DER  */
  YYSYMBOL_TOKEN_IZQ = 6,                  /* TOKEN_IZQ  */
  YYSYMBOL_TOKEN_QUIETO = 7,               /* TOKEN_QUIETO  */
  YYSYMBOL_TOKEN_FLECHA = 8,               /* TOKEN_FLECHA  */
  YYSYMBOL_TOKEN_ALFABETO = 9,             /* TOKEN_ALFABETO  */
  YYSYMBOL_TOKEN_ESTADOS = 10,             /* TOKEN_ESTADOS  */
  YYSYMBOL_TOKEN_INICIAL = 11,             /* TOKEN_INICIAL  */
  YYSYMBOL_TOKEN_FINALES = 12,             /* TOKEN_FINALES  */
  YYSYMBOL_TOKEN_SUBRUTINA = 13,           /* TOKEN_SUBRUTINA  */
  YYSYMBOL_TOKEN_ID = 14,                  /* TOKEN_ID  */
  YYSYMBOL_TOKEN_NUMERO = 15,              /* TOKEN_NUMERO  */
  YYSYMBOL_TOKEN_REPETIR = 16,             /* TOKEN_REPETIR  */
  YYSYMBOL_TOKEN_VECES = 17,               /* TOKEN_VECES  */
  YYSYMBOL_18_ = 18,                       /* '('  */
  YYSYMBOL_19_ = 19,                       /* ')'  */
  YYSYMBOL_20_ = 20,                       /* '{'  */
  YYSYMBOL_21_ = 21,                       /* '}'  */
  YYSYMBOL_22_ = 22,                       /* ','  */
  YYSYMBOL_23_ = 23,                       /* ';'  */
  YYSYMBOL_24_ = 24,                       /* ':'  */
  YYSYMBOL_YYACCEPT = 25,                  /* $accept  */
  YYSYMBOL_programa = 26,                  /* programa  */
  YYSYMBOL_lista_subrutinas = 27,          /* lista_subrutinas  */
  YYSYMBOL_declaracion_subrutina = 28,     /* declaracion_subrutina  */
  YYSYMBOL_cuerpo_subrutina = 29,          /* cuerpo_subrutina  */
  YYSYMBOL_bloque_repetir = 30,            /* bloque_repetir  */
  YYSYMBOL_lista_plantillas = 31,          /* lista_plantillas  */
  YYSYMBOL_transicion_plantilla = 32,      /* transicion_plantilla  */
  YYSYMBOL_lista_params = 33,              /* lista_params  */
  YYSYMBOL_parametro = 34,                 /* parametro  */
  YYSYMBOL_maquina_def = 35,               /* maquina_def  */
  YYSYMBOL_contenido_maquina = 36,         /* contenido_maquina  */
  YYSYMBOL_lista_argumentos = 37,          /* lista_argumentos  */
  YYSYMBOL_argumento = 38,                 /* argumento  */
  YYSYMBOL_bloque_alfabeto = 39,           /* bloque_alfabeto  */
  YYSYMBOL_lista_simbolos = 40,            /* lista_simbolos  */
  YYSYMBOL_simbolo_alfabeto = 41,          /* simbolo_alfabeto  */
  YYSYMBOL_bloque_estados = 42,            /* bloque_estados  */
  YYSYMBOL_lista_estados_ids = 43,         /* lista_estados_ids  */
  YYSYMBOL_config_inicial = 44,            /* config_inicial  */
  YYSYMBOL_config_finales = 45,            /* config_finales  */
  YYSYMBOL_lista_ids_finales = 46,         /* lista_ids_finales  */
  YYSYMBOL_bloque_transiciones = 47,       /* bloque_transiciones  */
  YYSYMBOL_lista_reglas = 48,              /* lista_reglas  */
  YYSYMBOL_simbolo_regla = 49,             /* simbolo_regla  */
  YYSYMBOL_direccion = 50,                 /* direccion  */
  YYSYMBOL_regla = 51                      /* regla  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   133

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  25
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  27
/* YYNRULES -- Number of rules.  */
#define YYNRULES  46
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  116

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   272


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      18,    19,     2,     2,    22,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    24,    23,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    20,     2,    21,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   189,   189,   192,   194,   202,   227,   249,   250,   251,
     255,   263,   264,   268,   284,   285,   286,   290,   294,   300,
     302,   307,   312,   320,   324,   333,   337,   338,   342,   346,
     353,   357,   361,   368,   376,   380,   385,   393,   397,   398,
     402,   403,   407,   408,   409,   413,   417
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "TOKEN_MAQUINA",
  "TOKEN_TRANSICIONES", "TOKEN_DER", "TOKEN_IZQ", "TOKEN_QUIETO",
  "TOKEN_FLECHA", "TOKEN_ALFABETO", "TOKEN_ESTADOS", "TOKEN_INICIAL",
  "TOKEN_FINALES", "TOKEN_SUBRUTINA", "TOKEN_ID", "TOKEN_NUMERO",
  "TOKEN_REPETIR", "TOKEN_VECES", "'('", "')'", "'{'", "'}'", "','", "';'",
  "':'", "$accept", "programa", "lista_subrutinas",
  "declaracion_subrutina", "cuerpo_subrutina", "bloque_repetir",
  "lista_plantillas", "transicion_plantilla", "lista_params", "parametro",
  "maquina_def", "contenido_maquina", "lista_argumentos", "argumento",
  "bloque_alfabeto", "lista_simbolos", "simbolo_alfabeto",
  "bloque_estados", "lista_estados_ids", "config_inicial",
  "config_finales", "lista_ids_finales", "bloque_transiciones",
  "lista_reglas", "simbolo_regla", "direccion", "regla", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-92)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int8 yypact[] =
{
     -92,    18,     2,   -92,    -4,    11,   -92,   -92,    20,     6,
      28,    27,     0,    22,    33,   -92,   -11,   -92,    24,    29,
      25,   -92,   -92,    13,    30,    34,    31,    27,    35,    38,
     -92,   -92,   -92,     8,   -92,    39,    32,    36,     0,   -92,
      -5,    -8,   -92,    40,   -92,    13,   -92,    10,    41,    37,
     -92,    42,    43,    19,   -92,   -92,    45,   -92,   -92,    44,
      46,    47,    -2,   -92,    48,   -92,   -92,    52,    50,   -92,
     -92,    51,   -92,   -92,    21,    54,    57,   -92,    17,   -92,
     -92,   -10,   -92,    53,    55,    -7,   -92,    49,    59,    56,
      21,    19,    19,   -92,   -92,   -92,   -92,    60,   -92,    58,
      68,    61,    16,    64,   -92,   -92,   -92,   -92,    62,    65,
     -92,    19,    66,    16,    63,   -92
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       3,     0,     0,     1,     0,     0,     4,     2,     0,     0,
       0,    14,     7,     0,     0,    17,     0,    15,     0,     0,
       0,     9,     8,     0,     0,     0,     0,     0,     0,     0,
       6,    28,    29,     0,    26,     0,     0,     0,     7,    16,
       0,     0,    38,     0,    25,     0,    31,     0,     0,     0,
      19,     0,     0,     0,    37,    39,     0,    27,    30,     0,
       0,     0,     0,     5,     0,    40,    41,     0,     0,    32,
      33,     0,    18,    20,     0,     0,     0,    35,     0,    23,
      24,     0,    21,     0,     0,     0,    11,     0,     0,     0,
       0,     0,     0,    10,    12,    34,    36,     0,    22,     0,
       0,     0,     0,     0,    46,    42,    43,    44,     0,     0,
      45,     0,     0,     0,     0,    13
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -92,   -92,   -92,   -92,    67,   -92,   -92,   -31,   -92,    69,
     -92,   -92,   -92,    -9,   -92,   -92,    70,   -92,   -92,   -92,
     -92,   -92,    71,   -92,   -91,   -34,    72
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     1,     2,     6,    20,    21,    85,    86,    16,    17,
       7,    62,    81,    82,    14,    33,    34,    25,    47,    37,
      50,    78,    22,    41,    67,   108,    42
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      99,   100,    18,    52,    18,     4,    40,    84,    26,    89,
       8,    27,    90,    54,    93,     5,    19,    53,     3,    72,
     112,   105,   106,   107,    11,     9,    12,    31,    32,    44,
      45,    58,    59,    65,    66,    79,    80,    13,    87,    88,
      10,    15,    23,    24,    28,    36,    30,    29,    49,    40,
      35,    38,    43,    46,    94,    60,    48,    64,    69,    56,
      75,    61,    68,    63,    97,    77,    74,    71,    83,    70,
      76,    84,    95,    96,   101,    91,   103,    92,   109,   114,
     102,    98,     0,     0,   104,   110,   115,   111,   113,     0,
       0,     0,     0,     0,     0,     0,    39,     0,     0,     0,
       0,     0,     0,     0,     0,    51,     0,     0,     0,     0,
       0,     0,     0,    55,     0,    57,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    73
};

static const yytype_int8 yycheck[] =
{
      91,    92,     4,     8,     4,     3,    14,    14,    19,    19,
      14,    22,    22,    21,    21,    13,    16,    22,     0,    21,
     111,     5,     6,     7,    18,    14,    20,    14,    15,    21,
      22,    21,    22,    14,    15,    14,    15,     9,    21,    22,
      20,    14,    20,    10,    20,    11,    21,    18,    12,    14,
      20,    20,    14,    14,    85,    14,    24,    14,    14,    19,
       8,    24,    17,    21,     8,    14,    18,    20,    14,    23,
      20,    14,    23,    14,    14,    22,     8,    22,    14,   113,
      22,    90,    -1,    -1,    23,    23,    23,    22,    22,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    41,    -1,    45,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    62
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    26,    27,     0,     3,    13,    28,    35,    14,    14,
      20,    18,    20,     9,    39,    14,    33,    34,     4,    16,
      29,    30,    47,    20,    10,    42,    19,    22,    20,    18,
      21,    14,    15,    40,    41,    20,    11,    44,    20,    34,
      14,    48,    51,    14,    21,    22,    14,    43,    24,    12,
      45,    29,     8,    22,    21,    51,    19,    41,    21,    22,
      14,    24,    36,    21,    14,    14,    15,    49,    17,    14,
      23,    20,    21,    47,    18,     8,    20,    14,    46,    14,
      15,    37,    38,    14,    14,    31,    32,    21,    22,    19,
      22,    22,    22,    21,    32,    23,    14,     8,    38,    49,
      49,    14,    22,     8,    23,     5,     6,     7,    50,    14,
      23,    22,    49,    22,    50,    23
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    25,    26,    27,    27,    28,    28,    29,    29,    29,
      30,    31,    31,    32,    33,    33,    33,    34,    35,    36,
      36,    37,    37,    38,    38,    39,    40,    40,    41,    41,
      42,    43,    43,    44,    45,    46,    46,    47,    48,    48,
      49,    49,    50,    50,    50,    51,    51
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     0,     2,     8,     5,     0,     1,     1,
       8,     1,     2,    10,     0,     1,     3,     1,     9,     0,
       2,     1,     3,     1,     1,     4,     1,     3,     1,     1,
       4,     1,     3,     4,     6,     1,     3,     4,     1,     2,
       1,     1,     1,     1,     1,    10,     9
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 5: /* declaracion_subrutina: TOKEN_SUBRUTINA TOKEN_ID '(' lista_params ')' '{' cuerpo_subrutina '}'  */
#line 203 "parser.y"
    {
        // Si hay bloque repetir, su variable debe coincidir con el parámetro declarado
        if (sub_temp.tiene_repetir) {
            if ((yyvsp[-4].str) == NULL || strcmp(sub_temp.param_repetir, (yyvsp[-4].str)) != 0) {
                fprintf(stderr, "Error: en '%s', repetir(%s) no coincide con el parametro declarado '%s'.\n",
                        (yyvsp[-6].str), sub_temp.param_repetir, (yyvsp[-4].str) ? (yyvsp[-4].str) : "ninguno");
                memset(&sub_temp, 0, sizeof(SubrutinaDef));
                YYERROR;
            }
        }

        if (total_subrutinas < 20) {
            strcpy(sub_temp.nombre, (yyvsp[-6].str));
            strcpy(sub_temp.param, (yyvsp[-4].str) ? (yyvsp[-4].str) : "");
            tabla_subrutinas[total_subrutinas++] = sub_temp;
            printf("Subrutina declarada con parametros: %s(%s)\n", sub_temp.nombre, sub_temp.param);
        } else {
            fprintf(stderr, "Error: limite de subrutinas alcanzado.\n");
            YYERROR;
        }
        memset(&sub_temp, 0, sizeof(SubrutinaDef));
    }
#line 1366 "parser.tab.c"
    break;

  case 6: /* declaracion_subrutina: TOKEN_SUBRUTINA TOKEN_ID '{' cuerpo_subrutina '}'  */
#line 228 "parser.y"
    {
        if (sub_temp.tiene_repetir) {
            fprintf(stderr, "Error: subrutina '%s' usa repetir(%s) pero no declara parametros.\n",
                    (yyvsp[-3].str), sub_temp.param_repetir);
            memset(&sub_temp, 0, sizeof(SubrutinaDef));
            YYERROR;
        }

        if (total_subrutinas < 20) {
            strcpy(sub_temp.nombre, (yyvsp[-3].str));
            sub_temp.param[0] = '\0';
            tabla_subrutinas[total_subrutinas++] = sub_temp;
            printf("Subrutina declarada sin parametros: %s\n", sub_temp.nombre);
        } else {
            fprintf(stderr, "Error: limite de subrutinas alcanzado.\n");
            YYERROR;
        }
        memset(&sub_temp, 0, sizeof(SubrutinaDef));
    }
#line 1390 "parser.tab.c"
    break;

  case 10: /* bloque_repetir: TOKEN_REPETIR '(' TOKEN_ID ')' TOKEN_VECES '{' lista_plantillas '}'  */
#line 256 "parser.y"
    {
        sub_temp.tiene_repetir = 1;
        snprintf(sub_temp.param_repetir, sizeof(sub_temp.param_repetir), "%s", (yyvsp[-5].str));
    }
#line 1399 "parser.tab.c"
    break;

  case 13: /* transicion_plantilla: TOKEN_ID ',' simbolo_regla TOKEN_FLECHA TOKEN_ID ',' simbolo_regla ',' direccion ';'  */
#line 269 "parser.y"
    {
        if (sub_temp.total_reglas < 10) {
            int idx = sub_temp.total_reglas++;
            sub_temp.reglas[idx].lee_simbolo = (yyvsp[-7].num);
            sub_temp.reglas[idx].escribe_simbolo = (yyvsp[-3].num);
            strcpy(sub_temp.reglas[idx].movimiento, (yyvsp[-1].str));
            sub_temp.reglas[idx].es_bucle_actual = (strcmp((yyvsp[-5].str), "actual") == 0);
        } else {
            fprintf(stderr, "Error: maximo 10 reglas por bloque repetir.\n");
            YYERROR;
        }
    }
#line 1416 "parser.tab.c"
    break;

  case 14: /* lista_params: %empty  */
#line 284 "parser.y"
                                 { (yyval.str) = NULL; }
#line 1422 "parser.tab.c"
    break;

  case 15: /* lista_params: parametro  */
#line 285 "parser.y"
                                 { (yyval.str) = (yyvsp[0].str); }
#line 1428 "parser.tab.c"
    break;

  case 16: /* lista_params: lista_params ',' parametro  */
#line 286 "parser.y"
                                 { (yyval.str) = (yyvsp[-2].str); }
#line 1434 "parser.tab.c"
    break;

  case 17: /* parametro: TOKEN_ID  */
#line 290 "parser.y"
             { (yyval.str) = (yyvsp[0].str); }
#line 1440 "parser.tab.c"
    break;

  case 18: /* maquina_def: TOKEN_MAQUINA TOKEN_ID '{' bloque_alfabeto bloque_estados config_inicial config_finales contenido_maquina '}'  */
#line 295 "parser.y"
    {
        printf("Analisis sintactico exitoso por la maquina: %s\n", (yyvsp[-7].str));
    }
#line 1448 "parser.tab.c"
    break;

  case 21: /* lista_argumentos: argumento  */
#line 308 "parser.y"
    {
        snprintf((yyval.args).valor, sizeof((yyval.args).valor), "%s", (yyvsp[0].str));
        (yyval.args).cantidad = 1;
    }
#line 1457 "parser.tab.c"
    break;

  case 22: /* lista_argumentos: lista_argumentos ',' argumento  */
#line 313 "parser.y"
    {
        (yyval.args) = (yyvsp[-2].args);
        (yyval.args).cantidad++;
    }
#line 1466 "parser.tab.c"
    break;

  case 23: /* argumento: TOKEN_ID  */
#line 321 "parser.y"
    {
        (yyval.str) = (yyvsp[0].str);
    }
#line 1474 "parser.tab.c"
    break;

  case 24: /* argumento: TOKEN_NUMERO  */
#line 325 "parser.y"
    {
        char buffer[20];
        snprintf(buffer, sizeof(buffer), "%d", (yyvsp[0].num));
        (yyval.str) = strdup(buffer);
    }
#line 1484 "parser.tab.c"
    break;

  case 28: /* simbolo_alfabeto: TOKEN_ID  */
#line 343 "parser.y"
    {
        alfabeto[total_alfabeto++] = (yyvsp[0].str)[0];
    }
#line 1492 "parser.tab.c"
    break;

  case 29: /* simbolo_alfabeto: TOKEN_NUMERO  */
#line 347 "parser.y"
    {
        alfabeto[total_alfabeto++] = '0' + ((yyvsp[0].num) % 10);
    }
#line 1500 "parser.tab.c"
    break;

  case 31: /* lista_estados_ids: TOKEN_ID  */
#line 358 "parser.y"
    {
        registrar_estado((yyvsp[0].str));
    }
#line 1508 "parser.tab.c"
    break;

  case 32: /* lista_estados_ids: lista_estados_ids ',' TOKEN_ID  */
#line 362 "parser.y"
    {
        registrar_estado((yyvsp[0].str));
    }
#line 1516 "parser.tab.c"
    break;

  case 33: /* config_inicial: TOKEN_INICIAL ':' TOKEN_ID ';'  */
#line 369 "parser.y"
    {
        strcpy(estado_inicial, (yyvsp[-1].str));
        printf("Estado inicial guardado: %s\n", estado_inicial);
    }
#line 1525 "parser.tab.c"
    break;

  case 35: /* lista_ids_finales: TOKEN_ID  */
#line 381 "parser.y"
    {
        strcpy(estados_finales[total_finales], (yyvsp[0].str));
        total_finales++;
    }
#line 1534 "parser.tab.c"
    break;

  case 36: /* lista_ids_finales: lista_ids_finales ',' TOKEN_ID  */
#line 386 "parser.y"
    {
        strcpy(estados_finales[total_finales], (yyvsp[0].str));
        total_finales++;
    }
#line 1543 "parser.tab.c"
    break;

  case 40: /* simbolo_regla: TOKEN_ID  */
#line 402 "parser.y"
                   { (yyval.num) = (yyvsp[0].str)[0]; }
#line 1549 "parser.tab.c"
    break;

  case 41: /* simbolo_regla: TOKEN_NUMERO  */
#line 403 "parser.y"
                   { (yyval.num) = '0' + ((yyvsp[0].num) % 10); }
#line 1555 "parser.tab.c"
    break;

  case 42: /* direccion: TOKEN_DER  */
#line 407 "parser.y"
                   { (yyval.str) = "DER"; }
#line 1561 "parser.tab.c"
    break;

  case 43: /* direccion: TOKEN_IZQ  */
#line 408 "parser.y"
                   { (yyval.str) = "IZQ"; }
#line 1567 "parser.tab.c"
    break;

  case 44: /* direccion: TOKEN_QUIETO  */
#line 409 "parser.y"
                   { (yyval.str) = "QUIETO"; }
#line 1573 "parser.tab.c"
    break;

  case 45: /* regla: TOKEN_ID ',' simbolo_regla TOKEN_FLECHA TOKEN_ID ',' simbolo_regla ',' direccion ';'  */
#line 414 "parser.y"
    {
        insertar_transicion((yyvsp[-9].str), (yyvsp[-7].num), (yyvsp[-5].str), (yyvsp[-3].num), (yyvsp[-1].str));
    }
#line 1581 "parser.tab.c"
    break;

  case 46: /* regla: TOKEN_ID TOKEN_FLECHA TOKEN_ID '(' lista_argumentos ')' TOKEN_FLECHA TOKEN_ID ';'  */
#line 418 "parser.y"
    {
        if (!expandir_subrutina((yyvsp[-6].str), (yyvsp[-4].args).valor, (yyvsp[-4].args).cantidad, (yyvsp[-8].str), (yyvsp[-1].str))) YYERROR;
    }
#line 1589 "parser.tab.c"
    break;


#line 1593 "parser.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 422 "parser.y"


void yyerror(const char *s){
    fprintf(stderr, "Error en la sintaxis: %s\n", s);
}
