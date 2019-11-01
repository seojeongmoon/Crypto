/******************************************************************************/
/*                                                                            */
/*  Arithmetik- und Zahlentheorie-Funktionen fuer grosse Zahlen in C          */
/*  Software zum Buch "Kryptographie in C und C++"                            */
/*  Autor: Michael Welschenbach                                               */
/*                                                                            */
/*  Quelldatei flint.h      Stand: 29.04.2001                                 */
/*                                                                            */
/*  Copyright (C) 1998-2001 by Michael Welschenbach                           */
/*  Copyright (C) 1998-2001 by Springer-Verlag Berlin, Heidelberg             */
/*                                                                            */
/*  All Rights Reserved                                                       */
/*                                                                            */
/*  Die Software darf fuer nichtkommerzielle Zwecke frei verwendet und        */
/*  veraendert werden, solange die folgenden Bedingungen anerkannt werden:    */
/*                                                                            */
/*  (1) Alle Aenderungen muessen als solche kenntlich gemacht werden, so dass */
/*      nicht der Eindruck erweckt wird, es handle sich um die Original-      */
/*      Software.                                                             */
/*                                                                            */
/*  (2) Die Copyright-Hinweise und die uebrigen Angaben in diesem Feld        */
/*      duerfen weder entfernt noch veraendert werden.                        */
/*                                                                            */
/*  (3) Weder der Verlag noch der Autor uebernehmen irgendeine Haftung fuer   */
/*      eventuelle Schaeden, die aus der Verwendung oder aus der Nicht-       */
/*      verwendbarkeit der Software, gleich fuer welchen Zweck, entstehen     */
/*      sollten, selbst dann nicht, wenn diese Schaeden durch Fehler verur-   */
/*      sacht wurden, die in der Software enthalten sein sollten.             */
/*                                                                            */
/******************************************************************************/


/* Lese flint.h nur einmal */

#ifndef __FLINTH__
#define __FLINTH__

/* Einschalten des FLINT-Sicherheitsmodus */
#if !(defined FLINT_SECURE || defined FLINT_UNSECURE)
#define FLINT_UNSECURE
#endif

#ifdef  __cplusplus
extern "C" {
#endif

#include <time.h>


/******************************************************************************/
/* Macros                                                                     */
/******************************************************************************/
/* Einfache Makros */
/* Fehlercodes     */


#define E_CLINT_OK     0        /* Alles O.K.                                 */
#define E_CLINT_DBZ   -1        /* Division durch Null                        */
#define E_CLINT_OFL   -2        /* Overflow                                   */
#define E_CLINT_UFL   -3        /* Underflow                                  */
#define E_CLINT_MAL   -4        /* Fehler bei Memory Allocation               */
#define E_CLINT_NOR   -5        /* Register nicht vorhanden                   */
#define E_CLINT_BOR   -6        /* Ungueltige Basis in str2clint_l()          */
#define E_CLINT_MOD   -7        /* Modulus nicht ungerade in ?mexp?m_l()      */
#define E_CLINT_NPT   -8        /* Null-Pointer uebergeben                    */


#define E_VCHECK_OK    0        /* Zahlformat O.K.                            */
#define E_VCHECK_LDZ   1        /* vcheck_l-Warnung: Fuehrende Nullen         */
#define E_VCHECK_MEM  -1        /* vcheck_l-Fehler: Null-Pointer              */
#define E_VCHECK_OFL  -2        /* vcheck_l-Fehler: Ueberlauf                 */


/***********************************************************/
/* Konstanten mit Bezug zur Basis der CLINT-Darstellung    */
/***********************************************************/

#define BASE            0x10000UL
#define BASEMINONE      0xffffU
#define BASEMINONEL     0xffffUL
#define DBASEMINONE     0xffffffffUL
#define BASEDIV2        0x8000U
#define DBASEDIV2       0x80000000U
#define BITPERDGT       16UL
#define LDBITPERDGT     4U


/***********************************************************/
/* Anzahl der Stellen zur Basis 0x10000 von CLINT-Objekten */

#define CLINTMAXDIGIT   64U
/***********************************************************/

#define CLINTMAXSHORT   (CLINTMAXDIGIT + 1)
#define CLINTMAXLONG    ((CLINTMAXDIGIT >> 1) + 1)
#define CLINTMAXBYTE    (CLINTMAXSHORT << 1)
#define CLINTMAXBIT     (CLINTMAXDIGIT << 4)

/* Anzahl kleiner Primzahlen in smallprimes[] */
#define NOOFSMALLPRIMES 6542

/* Default-Anzahl von Registern in Registerbank */
#define NOOFREGS        16U

/* FLINT/C-Versionskennung */
#define FLINT_VERMAJ        2       /* Major-Versionsnummer */
#define FLINT_VERMIN        2       /* Minor-Versionsnummer */
/* FLINT/C-Version als USHORT-Wert 0xhhll, hh=FLINT_VERMAJ, ll=FLINT_VERMIN */
#define FLINT_VERSION   ((FLINT_VERMAJ << 8) + FLINT_VERMIN)

#ifdef FLINT_COMPATIBILITY
/* Kompatibilitaetsmakros zu Versionen 1.xx */
#define E_OK     0                  /* Alles O.K.                             */
#define E_DBZ   -1                  /* Division durch Null                    */
#define E_OFL   -2                  /* Overflow                               */
#define E_UFL   -3                  /* Underflow                              */
#define E_MAL   -4                  /* Fehler bei Memory Allocation           */
#define E_NOR   -5                  /* Register nicht vorhanden               */
#define E_BOR   -6                  /* Ungueltige Basis in str2clint_l()      */
#define E_MOD   -7                  /* Modulus nicht ungerade in ?mexp?m_l()  */
#define E_NPT   -8                  /* Null-Pointer uebergeben                */
#endif /* FLINT_COMPAT */

/* Internationalisierung */
#define ggT_l            gcd_l
#define xggT_l           xgcd_l
#define kgV_l            lcm_l
#define zweiantei_l      twofact_l
#define chinrest_l       chinrem_l
#define primwurz_l       primroot_l


/* LINT_ASM -> FLINT_ASM und LINT_ANSI -> FLINT_ANSI */
#ifdef LINT_ASM
#ifndef FLINT_ASM
#define FLINT_ASM
#endif /* !FLINT_ASM */
#endif /* LINT_ASM */

#ifdef LINT_ANSI
#ifndef FLINT_ANSI
#define FLINT_ANSI
#endif /* !LINT_ANSI */
#endif /* LINT_ANSI */


#ifdef FLINT_ASM
#define _FLINT_ASM       0x61       /* ASCII 'a': Kennung fuer     */
#else                               /*  Assembler-Unterstuetzung   */
#define _FLINT_ASM          0
#endif

#ifdef FLINT_SECURE
#define _FLINT_SECMOD    0x73       /* ASCII 's': Kennung fuer     */
#else                               /*  Sicherheits-Modus, in dem  */
#define _FLINT_SECMOD       0       /*  alle CLINT-Variablen durch */
#endif                              /*  Ueberschreiben geloescht   */
                                    /*  werden.                    */


/* Parametrisierte Makros */

/* Definition von Standard-CLINT-Registern */

#define r0_l  get_reg_l(0)
#define r1_l  get_reg_l(1)
#define r2_l  get_reg_l(2)
#define r3_l  get_reg_l(3)
#define r4_l  get_reg_l(4)
#define r5_l  get_reg_l(5)
#define r6_l  get_reg_l(6)
#define r7_l  get_reg_l(7)
#define r8_l  get_reg_l(8)
#define r9_l  get_reg_l(9)
#define r10_l get_reg_l(10)
#define r11_l get_reg_l(11)
#define r12_l get_reg_l(12)
#define r13_l get_reg_l(13)
#define r14_l get_reg_l(14)
#define r15_l get_reg_l(15)


/* MIN, MAX etc. */

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#define MIN_L(a,b) (lt_l ((a), (b)) ? (a) : (b))
#define min_l(a,b) (lt_l ((a), (b)) ? (a) : (b))

#define MAX_L(a,b) (gt_l ((a), (b)) ? (a) : (b))
#define max_l(a,b) (gt_l ((a), (b)) ? (a) : (b))


#ifndef SWAP
#define SWAP(a,b) ((a) ^= (b), (b) ^= (a), (a) ^= (b))
#endif
#ifndef swap
#define swap(a,b) ((a) ^= (b), (b) ^= (a), (a) ^= (b))
#endif

#define SWAP_L(a,b) (xor_l((a),(b),(a)),xor_l((b),(a),(b)),xor_l((a),(b),(a)))
#define swap_l(a,b) (xor_l((a),(b),(a)),xor_l((b),(a),(b)),xor_l((a),(b),(a)))


/* ReMoveLeaDingZeRoS von CLINT-Typ */
#define RMLDZRS_L(n_l) \
    while ((DIGITS_L (n_l) > 0) && (*MSDPTR_L (n_l) == 0)) {DECDIGITS_L (n_l);}

#define rmldzrs_l(n_l) \
    while ((DIGITS_L (n_l) > 0) && (*MSDPTR_L (n_l) == 0)) {DECDIGITS_L (n_l);}


/* Kopieren von CLINT-Typ, und Entfernen fuehrender Nullen */
#define ZCPY_L(dest_l,src_l)\
    cpy_l ((dest_l), (src_l));\
    RMLDZRS_L ((dest_l))

#define zcpy_l(dest_l,src_l)\
    cpy_l ((dest_l), (src_l));\
    RMLDZRS_L ((dest_l))


/* Restbildung modulo Nmax + 1 */
#define ANDMAX_L(a_l)\
    SETDIGITS_L ((a_l), MIN (DIGITS_L (a_l), (USHORT)CLINTMAXDIGIT));\
    RMLDZRS_L ((a_l))

#define andmax_l(a_l)\
    SETDIGITS_L ((a_l), MIN (DIGITS_L (a_l), (USHORT)CLINTMAXDIGIT));\
    RMLDZRS_L ((a_l))


/* Setzen eines CLINT-Objektes auf die Zahlwerte 0, 1, 2 */
#define SETZERO_L(n_l)\
    (*(n_l) = 0)

#define setzero_l(n_l)\
    (*(n_l) = 0)

#define SETONE_L(n_l)\
    (u2clint_l ((n_l), 1U))

#define setone_l(n_l)\
    (u2clint_l ((n_l), 1U))

#define SETTWO_L(n_l)\
    (u2clint_l ((n_l), 2U))

#define settwo_l(n_l)\
    (u2clint_l ((n_l), 2U))


/* Lesen der Stellenzahl eines CLINT-Objekts */
#define DIGITS_L(n_l)\
    ((unsigned short)*(n_l))

#define digits_l(n_l)\
    ((unsigned short)*(n_l))


/* Setzen der Stellenzahl eines CLINT-Objekts */
#define SETDIGITS_L(n_l, l)\
    (*(n_l) = (unsigned short)(l))

#define setdigits_l(n_l, l)\
    (*(n_l) = (unsigned short)(l))


/* Inkrementieren der Stellenzahl eines CLINT-Objekts */
#define INCDIGITS_L(n_l)\
    (++*(n_l))

#define incdigits_l(n_l)\
    (++*(n_l))


/* Dekrementieren der Stellenzahl eines CLINT-Objekts, 0 ist Minimum */
#define DECDIGITS_L(n_l)\
    Assert (DIGITS_L (n_l) > 0);\
    (--*(n_l))

#define decdigits_l(n_l)\
    Assert (DIGITS_L (n_l) > 0);\
    (--*(n_l))


/* Zeiger auf hoechstwertige Stelle eines CLINT-Objekts */
#define MSDPTR_L(n_l)\
    ((n_l) + DIGITS_L (n_l))

#define msdptr_l(n_l)\
    ((n_l) + DIGITS_L (n_l))


/* Zeiger auf niedrigstwertige Stelle eines CLINT-Objekts */
#define LSDPTR_L(n_l)\
    ((n_l) + 1)

#define lsdptr_l(n_l)\
    ((n_l) + 1)


/* Vergleiche, set_l und assign_l, gerade, ungerade */

#define LT_L(a_l,b_l) \
    (cmp_l ((a_l), (b_l)) == -1)        /* a_l < b_l        */

#define lt_l(a_l,b_l) \
    (cmp_l ((a_l), (b_l)) == -1)        /* a_l < b_l        */


#define LE_L(a_l,b_l) \
    (cmp_l ((a_l), (b_l)) < 1)          /* a_l <= b_l       */

#define le_l(a_l,b_l) \
    (cmp_l ((a_l), (b_l)) < 1)          /* a_l <= b_l       */


#define GT_L(a_l,b_l) \
    (cmp_l ((a_l), (b_l)) == 1)         /* a_l > b_l        */

#define gt_l(a_l,b_l) \
    (cmp_l ((a_l), (b_l)) == 1)         /* a_l > b_l        */


#define GE_L(a_l,b_l) \
    (cmp_l ((a_l), (b_l)) > -1)         /* a_l >= b_l       */

#define ge_l(a_l,b_l) \
    (cmp_l ((a_l), (b_l)) > -1)         /* a_l >= b_l       */


#define GTZ_L(a_l) \
    (cmp_l ((a_l), nul_l) == 1)         /* a_l > 0          */

#define gtz_l(a_l) \
    (cmp_l ((a_l), nul_l) == 1)         /* a_l > 0          */


#define EQZ_L(a_l) \
    (equ_l ((a_l), nul_l) == 1)         /* a_l == 0         */

#define eqz_l(a_l) \
    (equ_l ((a_l), nul_l) == 1)         /* a_l == 0         */


#define EQONE_L(a_l) \
    (equ_l ((a_l), one_l) == 1)         /* a_l == 1         */

#define eqone_l(a_l) \
    (equ_l ((a_l), one_l) == 1)         /* a_l == 1         */


#define SET_L(a_l,ul)\
    ul2clint_l ((a_l), (ul))            /* a_l <-- unsigned long ul */

#define set_l(a_l,ul)\
    ul2clint_l ((a_l), (ul))            /* a_l <-- unsigned long ul */


#define ASSIGN_L(dest_l,src_l)\
    cpy_l ((dest_l), (src_l))           /* src_l <-- dest_l */

#define assign_l(dest_l,src_l)\
    cpy_l ((dest_l), (src_l))           /* src_l <-- dest_l */


#define ISEVEN_L(a_l)\
    (DIGITS_L (a_l) == 0 || (DIGITS_L (a_l) > 0 && (*LSDPTR_L (a_l) & 1U) == 0))
                                        /* a_l ist gerade   */

#define iseven_l(a_l)\
    (DIGITS_L (a_l) == 0 || (DIGITS_L (a_l) > 0 && (*LSDPTR_L (a_l) & 1U) == 0))
                                        /* a_l ist gerade   */

#define ISODD_L(a_l)\
    (DIGITS_L (a_l) > 0 && (*LSDPTR_L (a_l) & 1U) == 1)
                                        /* a_l ist ungerade */

#define isodd_l(a_l)\
    (DIGITS_L (a_l) > 0 && (*LSDPTR_L (a_l) & 1U) == 1)
                                        /* a_l ist ungerade */



/* Standard-Ausgabe von CLINT-Objekten */
#define DISP_L(S,A) printf ("%s%s\n%u bit\n\n", (S), hexstr_l(A), ld_l(A))
#define disp_l(S,A) printf ("%s%s\n%u bit\n\n", (S), hexstr_l(A), ld_l(A))


/* Initialisierung der Zufallszahlengeneratoren mit der Zeit */
#define INITRAND_LT()\
        ulseed64_l ((unsigned long)time(NULL))

#define initrand_lt()\
        ulseed64_l ((unsigned long)time(NULL))


#define INITRAND64_LT()\
        ulseed64_l ((unsigned long)time(NULL))

#define initrand64_lt()\
        ulseed64_l ((unsigned long)time(NULL))


#define INITRANDBBS_LT()\
        ulseedBBS_l ((unsigned long)time(NULL))

#define initrandBBS_lt()\
        ulseedBBS_l ((unsigned long)time(NULL))


/* Kompatibilitaet zu Version 1.0 : Makro clint2str_l */

#define CLINT2STR_L(n_l,b) xclint2str_l ((n_l), (b), 0)
#define clint2str_l(n_l,b) xclint2str_l ((n_l), (b), 0)


/* Implementation der xxxstr_l-Funktionen als Macros */

#define HEXSTR_L(n_l) xclint2str_l ((n_l), 16, 0)
#define hexstr_l(n_l) xclint2str_l ((n_l), 16, 0)

#define DECSTR_L(n_l) xclint2str_l ((n_l), 10, 0)
#define decstr_l(n_l) xclint2str_l ((n_l), 10, 0)

#define OCTSTR_L(n_l) xclint2str_l ((n_l), 8, 0)
#define octstr_l(n_l) xclint2str_l ((n_l), 8, 0)

#define BINSTR_L(n_l) xclint2str_l ((n_l), 2, 0)
#define binstr_l(n_l) xclint2str_l ((n_l), 2, 0)


/* Faktorisierung mittels sieve_l() */

#define SFACTOR_L(n_l) sieve_l ((n_l), NOOFSMALLPRIMES);
#define sfactor_l(n_l) sieve_l ((n_l), NOOFSMALLPRIMES);


/* Miller-Rabin-Primzahltest, Parameter nach Lenstra und [MOV] */

#define ISPRIME_L(n) prime_l ((n), 302, 0)
#define isprime_l(n) prime_l ((n), 302, 0)


/* Makros zur Bestimmung der Potenzierungsfunktion                    */
/* Eine automatische Auswahl von mexpk_l oder mexpkm_l uebernimmt die */
/* Funktion mexp_l.                                                   */

/*
 * define MEXP_L(a,e,p,n) mexp5_l ((a), (e), (p), (n))
 */

#define MEXP_L(a,e,p,n) mexpk_l ((a), (e), (p), (n))

/*
 * #define MEXP_L(a,e,p,n) mexp5m_l ((a), (e), (p), (n))
 */

/*
 * #define MEXP_L(a,e,p,n) mexpkm_l ((a), (e), (p), (n))
 */


/* Loeschen von CLINT-Variablen durch Ueberschreiben.                   */

#ifdef FLINT_SECURE
  #define ZEROCLINT_L(A)  Assert (sizeof(A) >= CLINTMAXBYTE);\
                          purge_l (A)

  #define ZEROCLINTD_L(A) Assert (sizeof(A) >= sizeof(CLINTD));\
                          purged_l (A)

  #define ZEROCLINTQ_L(A) Assert (sizeof(A) >= sizeof(CLINTQ));\
                          purgeq_l (A)
#else
  #define ZEROCLINT_L(A)  (void)0
  #define ZEROCLINTD_L(A) (void)0
  #define ZEROCLINTQ_L(A) (void)0
#endif


/***********************************************************/
/* Typedefs                                                */
/***********************************************************/

typedef unsigned short clint;
typedef unsigned long clintd;
typedef clint CLINT[CLINTMAXSHORT];
typedef clint CLINTD[1 + (CLINTMAXDIGIT << 1)];
typedef clint CLINTQ[1 + (CLINTMAXDIGIT << 2)];
typedef clint *CLINTPTR;
#ifndef UCHAR
typedef unsigned char  UCHAR;
#endif  /* UCHAR */
#ifndef USHORT
typedef unsigned short USHORT;
#endif  /* USHORT */
#ifndef ULONG
typedef unsigned long  ULONG;
#endif  /* ULONG */


/***********************************************************/
/* Funktions-Prototypen                                    */
/***********************************************************/

#ifndef __FLINT_API
#ifdef FLINT_USEDLL
#define __FLINT_API                   __cdecl
#else
#define __FLINT_API                   /**/
#endif /* FLINT_USEDLL */
#endif /* !defined __FLINT_API */

#if !defined __FLINT_API_A
#if defined __GNUC__ && !defined __cdecl
#define __FLINT_API_A                 /**/
#else
#define __FLINT_API_A                 __cdecl
#endif /* !defined __GNUC__ */
#endif /* !defined __FLINT_API_A */


/* Falls das FLINT/C-Paket unter MS Visual C/C++ als DLL eingesetzt wird,    */
/* muessen Module, die von aussen auf die Daten nul_l, one_l, two_l und      */
/* smallprimes zugreifen, mit -D__FLINT_API_DATA=__declspec(dllimport)       */
/* uebersetzt werden.                                                        */

#ifndef __FLINT_API_DATA
#if (defined _MSC_VER && _MSC_VER >= 11) && defined FLINT_USEDLL
#define __FLINT_API_DATA              __declspec(dllimport)
#else
#define __FLINT_API_DATA              /**/
#endif /* MSC_VER && FLINT_USEDLL */
#endif /* !defined __FLINT_API_DATA */


extern int      __FLINT_API  add_l         (CLINT, CLINT, CLINT);
extern int      __FLINT_API  chinrem_l     (unsigned int, clint**, CLINT);
extern int      __FLINT_API  cmp_l         (CLINT, CLINT);
extern void     __FLINT_API  cpy_l         (CLINT, CLINT);
extern clint *  __FLINT_API  create_l      (void);
extern int      __FLINT_API  create_reg_l  (void);
extern int      __FLINT_API  dec_l         (CLINT);

#if !defined FLINT_ASM
extern int      __FLINT_API  div_l         (CLINT, CLINT, CLINT, CLINT);
#else
extern int      __FLINT_API_A div_l        (CLINT, CLINT, CLINT, CLINT);
#endif /* FLINT_ASM */


extern void     __FLINT_API  and_l         (CLINT, CLINT, CLINT);
extern int      __FLINT_API  byte2clint_l  (CLINT, UCHAR *, int);
extern int      __FLINT_API  clearbit_l    (CLINT, unsigned int);
extern UCHAR *  __FLINT_API  clint2byte_l  (CLINT, int *);
extern char *   __FLINT_API  xclint2str_l  (CLINT, USHORT, int);
extern int      __FLINT_API  equ_l         (CLINT, CLINT);
extern char *   __FLINT_API  fbinstr_l     (CLINT);
extern char *   __FLINT_API  fdecstr_l     (CLINT);
extern char *   __FLINT_API  fhexstr_l     (CLINT);
extern char *   __FLINT_API  foctstr_l     (CLINT);
extern void     __FLINT_API  free_l        (CLINT);
extern void     __FLINT_API  free_reg_l    (void);
extern void     __FLINT_API  fswap_l       (CLINT, CLINT);
extern ULONG *  __FLINT_API  genprimes     (ULONG);
extern clint *  __FLINT_API  get_reg_l     (unsigned int);
extern void     __FLINT_API  gcd_l         (CLINT, CLINT, CLINT);
extern int      __FLINT_API  inc_l         (CLINT);
extern void     __FLINT_API  inv_l         (CLINT, CLINT, CLINT, CLINT);
extern USHORT   __FLINT_API  invmon_l      (CLINT);
extern void     __FLINT_API  iroot_l       (CLINT, CLINT);
extern unsigned __FLINT_API  issqr_l       (CLINT, CLINT);
extern int      __FLINT_API  jacobi_l      (CLINT, CLINT);
extern int      __FLINT_API  lcm_l         (CLINT, CLINT, CLINT);
extern unsigned __FLINT_API  ld_l          (CLINT);
extern int      __FLINT_API  madd_l        (CLINT, CLINT, CLINT, CLINT);
extern int      __FLINT_API  mequ_l        (CLINT, CLINT, CLINT);
extern int      __FLINT_API  mexp_l        (CLINT, CLINT, CLINT, CLINT);
extern int      __FLINT_API  mexp2_l       (CLINT, USHORT, CLINT, CLINT);
extern int      __FLINT_API  mexp5_l       (CLINT, CLINT, CLINT, CLINT);
extern int      __FLINT_API  mexp5m_l      (CLINT, CLINT, CLINT, CLINT);
extern int      __FLINT_API  mexpk_l       (CLINT, CLINT, CLINT, CLINT);
extern int      __FLINT_API  mexpkm_l      (CLINT, CLINT, CLINT, CLINT);
extern int      __FLINT_API  mmul_l        (CLINT, CLINT, CLINT, CLINT);
extern int      __FLINT_API  mod2_l        (CLINT, ULONG, CLINT);
extern int      __FLINT_API  mod_l         (CLINT, CLINT, CLINT);
extern int      __FLINT_API  msqr_l        (CLINT, CLINT, CLINT);
extern int      __FLINT_API  msub_l        (CLINT, CLINT, CLINT, CLINT);
extern int      __FLINT_API  mul_l         (CLINT, CLINT, CLINT);
extern void     __FLINT_API  mulmon_l      (CLINT, CLINT, CLINT, USHORT, USHORT, CLINT);
extern void     __FLINT_API  or_l          (CLINT, CLINT, CLINT);
extern int      __FLINT_API  prime_l       (CLINT, unsigned int, unsigned int);
extern int      __FLINT_API  primroot_l    (CLINT, unsigned int, clint*[]);
extern int      __FLINT_API  proot_l       (CLINT, CLINT, CLINT);
extern void     __FLINT_API  purge_l       (CLINT);
extern void     __FLINT_API  purged_l      (CLINTD);
extern void     __FLINT_API  purgeq_l      (CLINTQ);
extern int      __FLINT_API  purge_reg_l   (unsigned int);
extern int      __FLINT_API  purgeall_reg_l(void);
extern clint *  __FLINT_API  rand64_l      (void);
extern void     __FLINT_API  rand_l        (CLINT, int);
extern void     __FLINT_API  randBBS_l     (CLINT, int);
extern int      __FLINT_API  randbit_l     (void);
extern int      __FLINT_API  root_l        (CLINT, CLINT, CLINT, CLINT);
extern clint *  __FLINT_API  seed64_l      (CLINT);
extern int      __FLINT_API  seedBBS_l     (CLINT);
extern void     __FLINT_API  set_noofregs_l(unsigned int);
extern int      __FLINT_API  setbit_l      (CLINT, unsigned int);
extern clint *  __FLINT_API  setmax_l      (CLINT);
extern int      __FLINT_API  shift_l       (CLINT, long int);
extern int      __FLINT_API  shl_l         (CLINT);
extern int      __FLINT_API  shr_l         (CLINT);
extern USHORT   __FLINT_API  sieve_l       (CLINT, unsigned int);
extern int      __FLINT_API  sqr_l         (CLINT, CLINT);
extern void     __FLINT_API  sqrmon_l      (CLINT, CLINT, USHORT, USHORT, CLINT);
extern int      __FLINT_API  str2clint_l   (CLINT, char *, USHORT);
extern char *   __FLINT_API  strlwr_l      (char *);
extern char *   __FLINT_API  strrev_l      (char *);
extern char *   __FLINT_API  strupr_l      (char *);
extern int      __FLINT_API  sub_l         (CLINT, CLINT, CLINT);
extern int      __FLINT_API  testbit_l     (CLINT, unsigned int);
extern int      __FLINT_API  tolower_l     (int);
extern int      __FLINT_API  toupper_l     (int);
extern void     __FLINT_API  u2clint_l     (CLINT, USHORT);
extern int      __FLINT_API  uadd_l        (CLINT, USHORT, CLINT);
extern UCHAR    __FLINT_API  ucrand64_l    (void);
extern UCHAR    __FLINT_API  ucrandBBS_l   (void);
extern int      __FLINT_API  udiv_l        (CLINT, USHORT, CLINT, CLINT);
extern void     __FLINT_API  ul2clint_l    (CLINT, ULONG);
extern ULONG    __FLINT_API  ulrand64_l    (void);
extern ULONG    __FLINT_API  ulrandBBS_l   (void);
extern clint *  __FLINT_API  ulseed64_l    (ULONG);
extern void     __FLINT_API  ulseedBBS_l   (ULONG);
extern char *   __FLINT_API  ultoa_l       (char *, ULONG, int);
extern int      __FLINT_API  umadd_l       (CLINT, USHORT, CLINT, CLINT);
extern int      __FLINT_API  umexp_l       (CLINT, USHORT, CLINT, CLINT);
extern int      __FLINT_API  umexpm_l      (CLINT, USHORT, CLINT, CLINT);
extern int      __FLINT_API  ummul_l       (CLINT, USHORT, CLINT, CLINT);
extern USHORT   __FLINT_API  umod_l        (CLINT, USHORT);
extern int      __FLINT_API  umsub_l       (CLINT, USHORT, CLINT, CLINT);
extern int      __FLINT_API  umul_l        (CLINT, USHORT, CLINT);
extern USHORT   __FLINT_API  usrand64_l    (void);
extern USHORT   __FLINT_API  usrandBBS_l   (void);
extern int      __FLINT_API  usub_l        (CLINT, USHORT, CLINT);
extern int      __FLINT_API  vcheck_l      (CLINT);
extern ULONG    __FLINT_API  version_l     (void);
extern char *   __FLINT_API  verstr_l      (void);
extern int      __FLINT_API  wmexp_l       (USHORT, CLINT, CLINT, CLINT);
extern int      __FLINT_API  wmexpm_l      (USHORT, CLINT, CLINT, CLINT);
extern void     __FLINT_API  xgcd_l        (CLINT, CLINT, CLINT, CLINT, int *, CLINT, int *);
extern void     __FLINT_API  xor_l         (CLINT, CLINT, CLINT);
extern int      __FLINT_API  twofact_l     (CLINT, CLINT);


/* Kernfunktionen ohne Overflow-Detection */
#if defined FLINT_ASM
extern void     __FLINT_API_A mult         (CLINT, CLINT, CLINT);
extern void     __FLINT_API_A umul         (CLINT, USHORT, CLINT);
extern void     __FLINT_API_A sqr          (CLINT, CLINT);
#else
extern void     __FLINT_API  mult          (CLINT, CLINT, CLINT);
extern void     __FLINT_API  umul          (CLINT, USHORT, CLINT);
extern void     __FLINT_API  sqr           (CLINT, CLINT);
#endif
extern void     __FLINT_API  add           (CLINT, CLINT, CLINT);
extern void     __FLINT_API  sub           (CLINT, CLINT, CLINT);


/* Funktionen mit Vorzeichen */
extern int      __FLINT_API  sadd          (CLINT, int, CLINT, int, CLINT);
extern int      __FLINT_API  ssub          (CLINT, int, CLINT, int, CLINT);
extern int      __FLINT_API  smod          (CLINT, int, CLINT, CLINT);


/* Konstanten */
extern clint  __FLINT_API_DATA nul_l[];
extern clint  __FLINT_API_DATA one_l[];
extern clint  __FLINT_API_DATA two_l[];
extern USHORT __FLINT_API_DATA smallprimes[];

#ifdef  __cplusplus
}
#endif

#endif /* #defined __FLINTH__ */
