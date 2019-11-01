/******************************************************************************/
/*                                                                            */
/*  Arithmetik- und Zahlentheorie-Funktionen fuer grosse Zahlen in C          */
/*  Software zum Buch "Kryptographie in C und C++"                            */
/*  Autor: Michael Welschenbach                                               */
/*                                                                            */
/*  Quelldatei flint.c      Stand: 09.05.2001                                 */
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
/*                                                                            */
/*  Voraussetzung an das Zielsystem:                                          */
/*                                                                            */
/*  sizeof (ULONG)  == 4                                                      */
/*  sizeof (USHORT) == 2                                                      */
/*                                                                            */
/******************************************************************************/

#ifndef FLINT_ANSI
#define FLINT_ANSI
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "flint.h"

#define NO_ASSERTS 1

#define FLINTCVMAJ  2
#define FLINTCVMIN  2

//#if ((FLINTCVMIN != FLINT_VERMIN) || (FLINTCVMAJ != FLINT_VERMAJ))
//#error Versionsfehler: FLINT.C inkompatibel zu FLINT.H
//#endif

#ifdef FLINT_DEBUG
#undef NO_ASSERTS
#define ASSERT_LOG_AND_QUIT
#include "_assert.h"
#include "_alloc.h"
#ifdef COVERAGE
#include "utclog.h"
#endif
#endif

#ifdef NO_ASSERTS
#define Assert(a) (void)0
#endif


/* Blende Global Wrap Up-Fehlermeldungen von PC-lint aus */
/*lint -esym(14,add,sub,mul,umul,sqr)   */
/*lint -esym(15,add,sub,mul,umul,sqr)   */
/*lint -esym(515,add,sub,mul,umul,sqr)  */
/*lint -esym(516,add,sub,mul,umul,sqr)  */
/*lint -esym(532,add,sub,mul,umul,sqr)  */
/*lint -esym(533,add,sub,mul,umul,sqr)  */
/*lint -esym(1066,add,sub,mul,umul,sqr) */
/*lint -esym(534,add_l,sub_l,mul_l,sqr_l,div_l,mmul_l,msub_l,dec_l,madd_l) */
/*lint -esym(534,msqr_l,mexp_l,mexp5_l,mexpk_l,mod_l,mod2_l,mexp2_l) */


/***********  Prototypen lokaler Funktionen ********************************/

/* Private Register-Funktionen */
static void
destroy_reg_l (void);
static int
allocate_reg_l (void);
/* Ganzzahlige Quadratwurzeln von ULONG-Werten */
static ULONG
ul_iroot (unsigned long n);

#ifdef FLINT_SECURE
#define PURGEVARS_L(X) purgevars_l X
/* Funktion zum Loeschen von Variablen */
static void purgevars_l (int noofvars, ...);
#ifdef FLINT_DEBUG
#define ISPURGED_L(X) Assert(ispurged_l X)
/* Funktion zum Pruefen, ob Variablen geloescht wurden */
static int ispurged_l (int noofvars, ...);
#else
#define ISPURGED_L(X) (void)0
#endif /* FLINT_DEBUG */
#else
#define PURGEVARS_L(X) (void)0
#define ISPURGED_L(X) (void)0
#endif /* FLINT_SECURE */
/******************************************************************************/

/* CLINT-Konstanten */
clint __FLINT_API_DATA
nul_l[] = {0, 0, 0, 0, 0};
clint __FLINT_API_DATA
one_l[] = {1, 1, 0, 0, 0};
clint __FLINT_API_DATA
two_l[] = {1, 2, 0, 0, 0};



/******************************************************************************/
/*                                                                            */
/*  Funktion:   Initialisierung der FLINT/C-Library                           */
/*              Falls die FLINT/C-Funktionen in einer DLL bereitgestellt      */
/*              werden, sollte die DLL-Initialisierungsroutine, z. B.         */
/*              DllMain(), diese Initalisierungsfunktion aufrufen.            */
/*                                                                            */
/*  Syntax:     FLINTInit_l() (void);                                         */
/*  Eingabe:    -                                                             */
/*  Ausgabe:    -                                                             */
/*  Rueckgabe:  E_CLINT_OK falls alles O.K.                                   */
/*              -1 sonst                                                      */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
FLINTInit_l (void)
{
  int error;
  initrand64_lt();
  initrandBBS_lt();
  error = create_reg_l();

  if (!error)
    return E_CLINT_OK;
  else
    return -1;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:   Verlassen der FLINT/C-Library                                 */
/*  Syntax:     FLINTExit_l (void);                                           */
/*  Eingabe:    -                                                             */
/*  Ausgabe:    -                                                             */
/*  Rueckgabe:  E_CLINT_OK falls alles O.K.                                   */
/*              -1 sonst                                                      */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
FLINTExit_l (void)
{
  free_reg_l();

  return E_CLINT_OK;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:   Kopieren CLINT nach CLINT                                     */
/*  Syntax:     void cpy_l (CLINT dest_l, CLINT src_l);                       */
/*  Eingabe:    CLINT src_l                                                   */
/*  Ausgabe:    CLINT dest_l                                                  */
/*  Rueckgabe:  -                                                             */
/*                                                                            */
/******************************************************************************/
void __FLINT_API
cpy_l (CLINT dest_l, CLINT src_l)
{
  clint *lastsrc_l = MSDPTR_L (src_l);
  *dest_l = *src_l;

  while ((*lastsrc_l == 0) && (*dest_l > 0))
    {
      --lastsrc_l;
      --*dest_l;
    }

  while (src_l < lastsrc_l)
    {
      *++dest_l = *++src_l;
    }
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Vertauschen zweier CLINT-Werte                                 */
/*  Syntax:    void fswap_l (CLINT a_l, CLINT b_l);                           */
/*  Eingabe:   CLINT a_l, b_l                                                 */
/*  Ausgabe:   CLINT a_l, b_l                                                 */
/*  Rueckgabe: -                                                              */
/*                                                                            */
/******************************************************************************/
void __FLINT_API
fswap_l (CLINT a_l, CLINT b_l)
{
  CLINT tmp_l;

  cpy_l (tmp_l, a_l);
  cpy_l (a_l, b_l);
  cpy_l (b_l, tmp_l);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((1, sizeof (tmp_l), tmp_l));
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Test auf Gleichheit                                            */
/*  Syntax:    int equ_l (CLINT a_l, CLINT b_l);                              */
/*  Eingabe:   CLINT a_l, b_l                                                 */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: 1 falls a_l und b_l gleichwertig sind, 0 sonst                 */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
equ_l (CLINT a_l, CLINT b_l)
{
  clint *msdptra_l, *msdptrb_l;
  int la = (int)DIGITS_L (a_l);
  int lb = (int)DIGITS_L (b_l);

  if (la == 0 && lb == 0)
    {
      return 1;
    }

  while (a_l[la] == 0 && la > 0)
    {
      --la;
    }

  while (b_l[lb] == 0 && lb > 0)
    {
      --lb;
    }

  if (la == 0 && lb == 0)
    {
      return 1;
    }

  if (la != lb)
    {
      return 0;
    }

  msdptra_l = a_l + la;
  msdptrb_l = b_l + lb;

  while ((*msdptra_l == *msdptrb_l) && (msdptra_l > a_l))
    {
      msdptra_l--;
      msdptrb_l--;
    }

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((2, sizeof (la), &la,
                   sizeof (lb), &lb));

  ISPURGED_L ((2, sizeof (la), &la,
                  sizeof (lb), &lb));

  return (msdptra_l > a_l ? 0 : 1);
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Test auf Gleichheit modulo m                                   */
/*  Syntax:    int mequ_l (CLINT a_l, CLINT b_l, CLINT m_l);                  */
/*  Eingabe:   CLINT a_l, b_l (zu vergleichende Werte), CLINT m_l (Modulus)   */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: 1 falls a_l = b_l mod m_l                                      */
/*             0 falls a_l != b_l mod m_l                                     */
/*             E_CLINT_DBZ bei Division durch 0                               */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
mequ_l (CLINT a_l, CLINT b_l, CLINT m_l)
{
  CLINT r_l;
  int res;

  if (EQZ_L (m_l))
    {
      return E_CLINT_DBZ;          /* Division durch Null? */
    }

  msub_l (a_l, b_l, r_l, m_l);

  res = (0 == DIGITS_L (r_l))?1:0;

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((1, sizeof (r_l), r_l));
  ISPURGED_L ((1, sizeof (r_l), r_l));

  return res;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Vergleich zweier CLINT-Objekte                                 */
/*  Syntax:    int cmp_l (CLINT a_l, CLINT b_l);                              */
/*  Eingabe:   CLINT a_l, b_l                                                 */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: -1 falls a_l < b_l, 0 falls a_l == b_l, 1 falls a_l > b_l      */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
cmp_l (CLINT a_l, CLINT b_l)
{
  clint *msdptra_l, *msdptrb_l;
  int la = (int)DIGITS_L (a_l);
  int lb = (int)DIGITS_L (b_l);

  if (la == 0 && lb == 0)
    {
      return 0;
    }

  while (a_l[la] == 0 && la > 0)
    {
      --la;
    }

  while (b_l[lb] == 0 && lb > 0)
    {
      --lb;
    }

  if (la == 0 && lb == 0)
    {
      return 0;
    }

  if (la > lb)
    {
      PURGEVARS_L ((2, sizeof (la), &la,
                       sizeof (lb), &lb));
      ISPURGED_L  ((2, sizeof (la), &la,
                       sizeof (lb), &lb));
      return 1;
    }

  if (la < lb)
    {
      PURGEVARS_L ((2, sizeof (la), &la,
                       sizeof (lb), &lb));
      ISPURGED_L  ((2, sizeof (la), &la,
                       sizeof (lb), &lb));
      return -1;
    }

  msdptra_l = a_l + la;
  msdptrb_l = b_l + lb;

  while ((*msdptra_l == *msdptrb_l) && (msdptra_l > a_l))
    {
      msdptra_l--;
      msdptrb_l--;
    }

  PURGEVARS_L ((2, sizeof (la), &la,
                   sizeof (lb), &lb));
  ISPURGED_L  ((2, sizeof (la), &la,
                   sizeof (lb), &lb));

  if (msdptra_l == a_l)
    {
      return 0;
    }

  if (*msdptra_l > *msdptrb_l)
    {
      return 1;
    }
  else
    {
      return -1;
    }
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Erzeugen des groessten CLINT-Wertes 2^CLINTMAXBIT - 1          */
/*  Syntax:    clint * setmax_l (CLINT a_l);                                  */
/*  Eingabe:   a_l CLINT-Objekt                                               */
/*  Ausgabe:   a_l erhaelt den Wert 2^CLINTMAXBIT - 1 = Nmax                  */
/*  Rueckgabe: Adresse der CLINT-Variablen a_l                                */
/*                                                                            */
/******************************************************************************/
clint * __FLINT_API
setmax_l (CLINT a_l)
{
  clint *aptr_l = a_l;
  clint *msdptra_l = a_l + CLINTMAXDIGIT;

  while (++aptr_l <= msdptra_l)
    {
      *aptr_l = BASEMINONE;
    }

  SETDIGITS_L (a_l, CLINTMAXDIGIT);
  return (clint *)a_l;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Addition                                                       */
/*  Syntax:    int add_l (CLINT a_l, CLINT b_l, CLINT s_l);                   */
/*  Eingabe:   a_l, b_l (Summanden)                                           */
/*  Ausgabe:   s_l (Summe)                                                    */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_OFL: Ueberlauf                                         */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
add_l (CLINT a_l, CLINT b_l, CLINT s_l)
{
  clint ss_l[CLINTMAXSHORT + 1];
  int OFL = 0;

  add (a_l, b_l, ss_l);

  if (DIGITS_L (ss_l) > (USHORT)CLINTMAXDIGIT)  /* Overflow ? */
    {
      ANDMAX_L (ss_l);             /* Reduziere modulo Nmax+1 */
      OFL = E_CLINT_OFL;
    }

  cpy_l (s_l, ss_l);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((1, sizeof (s_l), ss_l));
  ISPURGED_L  ((1, sizeof (s_l), ss_l));

  return OFL;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Subtraktion                                                    */
/*  Syntax:    int sub_l (CLINT aa_l, CLINT bb_l, CLINT d_l);                 */
/*  Eingabe:   aa_l (Minuend), bb_l (Subtrahend)                              */
/*  Ausgabe:   d_l (Differenz)                                                */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_UFL: Unterlauf                                         */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
sub_l (CLINT aa_l, CLINT bb_l, CLINT d_l)
{
  CLINT b_l;
  clint a_l[CLINTMAXSHORT + 1], t_l[CLINTMAXSHORT + 1], tmp_l[CLINTMAXSHORT + 1];
  int UFL = 0;

  cpy_l (b_l, bb_l);

  if (LT_L (aa_l, b_l))            /* Underflow ? */
    {
      setmax_l (a_l);              /* Wir rechnen mit Nmax */
      cpy_l (t_l, aa_l);           /* aa_l wird noch einmal benoetigt */
      UFL = E_CLINT_UFL;           /* das wird am Ende korrigiert  */
    }
  else
    {
      cpy_l (a_l, aa_l);
    }

  sub (a_l, b_l, tmp_l);

  if (UFL)
    {                              /* Underflow ? */
      add_l (tmp_l, t_l, d_l);     /* Korrektur erforderlich */
      inc_l (d_l);                 /* Einer fehlt noch */
    }
  else
    {
      cpy_l (d_l, tmp_l);
    }

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((4, sizeof (a_l), a_l,
                   sizeof (b_l), b_l,
                   sizeof (t_l), t_l,
                   sizeof (tmp_l), tmp_l));

  ISPURGED_L  ((4, sizeof (a_l), a_l,
                   sizeof (b_l), b_l,
                   sizeof (t_l), t_l,
                   sizeof (tmp_l), tmp_l));

  return UFL;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Inkrement um 1                                                 */
/*  Syntax:    int inc_l (CLINT a_l);                                         */
/*  Eingabe:   a_l                                                            */
/*  Ausgabe:   a_l, Inkrementiert um 1                                        */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_OFL: Ueberlauf                                         */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
inc_l (CLINT a_l)
{
  clint *msdptra_l, *aptr_l = LSDPTR_L (a_l);
  ULONG carry = BASE;
  int OFL = 0;

  msdptra_l = MSDPTR_L (a_l);
  while ((aptr_l <= msdptra_l) && (carry & BASE))
    {
      *aptr_l = (USHORT)(carry = 1UL + (ULONG)(*aptr_l));
      aptr_l++;
    }

  if ((aptr_l > msdptra_l) && (carry & BASE))
    {
      *aptr_l = 1;
      INCDIGITS_L (a_l);
      if (DIGITS_L (a_l) > (USHORT)CLINTMAXDIGIT)    /* Overflow ? */
        {
          SETZERO_L (a_l);              /* Reduziere modulo Nmax+1 */
          OFL = E_CLINT_OFL;
        }
    }

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((1, sizeof (carry), &carry));
  ISPURGED_L  ((1, sizeof (carry), &carry));

  return OFL;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Dekrement um 1                                                 */
/*  Syntax:    int dec_l (CLINT a_l);                                         */
/*  Eingabe:   a_l                                                            */
/*  Ausgabe:   a_l, Dekrementiert um 1                                        */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_UFL: Unterlauf                                         */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
dec_l (CLINT a_l)
{
  clint *msdptra_l, *aptr_l = LSDPTR_L (a_l);
  ULONG carry = DBASEMINONE;

  if (DIGITS_L (a_l) == 0)                     /* Underflow ? */
    {
      setmax_l (a_l);              /* Reduziere modulo Nmax+1 */
      return E_CLINT_UFL;
    }

  msdptra_l = MSDPTR_L (a_l);
  while ((aptr_l <= msdptra_l) && (carry & (BASEMINONEL << BITPERDGT)))
    {
      *aptr_l = (USHORT)(carry = (ULONG)*aptr_l - 1L);
      aptr_l++;
    }

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((1, sizeof (carry), &carry));
  ISPURGED_L  ((1, sizeof (carry), &carry));

  RMLDZRS_L (a_l);
  return E_CLINT_OK;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Multiplikation                                                 */
/*  Syntax:    int mul_l (CLINT f1_l, CLINT f2_l, CLINT pp_l);                */
/*  Eingabe:   f1_l, f2_l (Faktoren)                                          */
/*  Ausgabe:   p_l (Produkt)                                                  */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
mul_l (CLINT f1_l, CLINT f2_l, CLINT pp_l)
{
  CLINT a_l, b_l;
  CLINTD p_l;
  int OFL = 0;

  cpy_l (a_l, f1_l);
  cpy_l (b_l, f2_l);

  mult (a_l, b_l, p_l);

  if (DIGITS_L (p_l) > (USHORT)CLINTMAXDIGIT)   /* Overflow ? */
    {
      ANDMAX_L (p_l);              /* Reduziere modulo Nmax+1 */
      OFL = E_CLINT_OFL;
    }

  cpy_l (pp_l, p_l);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((3, sizeof (a_l), a_l,
                   sizeof (b_l), b_l,
                   sizeof (p_l), p_l));

  ISPURGED_L  ((3, sizeof (a_l), a_l,
                   sizeof (b_l), b_l,
                   sizeof (p_l), p_l));

  return OFL;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Quadrierung                                                    */
/*  Syntax:    int sqr_l (CLINT f_l, CLINT pp_l);                             */
/*  Eingabe:   f_l (Faktor)                                                   */
/*  Ausgabe:   pp_l (Quadrat)                                                 */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_OFL: Ueberlauf                                         */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
sqr_l (CLINT f_l, CLINT pp_l)
{
  CLINT a_l;
  CLINTD p_l;
  int OFL = 0;

  cpy_l (a_l, f_l);

  sqr (a_l, p_l);

  if (DIGITS_L (p_l) > (USHORT)CLINTMAXDIGIT)   /* Overflow ? */
    {
      ANDMAX_L (p_l);              /* Reduziere modulo Nmax+1 */
      OFL = E_CLINT_OFL;
    }

  cpy_l (pp_l, p_l);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((2, sizeof (a_l), a_l,
                   sizeof (p_l), p_l));

  ISPURGED_L  ((2, sizeof (a_l), a_l,
                   sizeof (p_l), p_l));

  return OFL;
}


#if !defined FLINT_ASM
/******************************************************************************/
/*                                                                            */
/*  Funktion:  Division mit Rest                                              */
/*  Syntax:    int div_l (CLINT d1_l, CLINT d2_l, CLINT quot_l, CLINT rest_l);*/
/*  Eingabe:   d1_l (Dividend), d2_l (Divisor)                                */
/*  Ausgabe:   quot_l (Quotient), rest_l (Rest)                               */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_DBZ: Division durch Null                               */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
div_l (CLINT d1_l, CLINT d2_l, CLINT quot_l, CLINT rest_l)
{
  register clint *rptr_l, *bptr_l;
  CLINT b_l;
  clint r_l[2 + (CLINTMAXDIGIT << 1)];  /* Erlaube doppelt langen Rest + 1 Stelle */
  clint *qptr_l, *msdptrb_l, *msdptrr_l, *lsdptrr_l;
  USHORT bv, rv, qdach, ri, ri_1, ri_2, bn, bn_1;
  ULONG right, left, rdach, borrow, carry, sbitsminusd;
  unsigned int d = 0;
  int i;

  cpy_l (r_l, d1_l);
  cpy_l (b_l, d2_l);

  if (EQZ_L (b_l))
    {
      PURGEVARS_L ((1, sizeof (r_l), r_l));
      ISPURGED_L  ((1, sizeof (r_l), r_l));

      return E_CLINT_DBZ;           /* Division durch Null */
    }

  if (EQZ_L (r_l))
    {
      SETZERO_L (quot_l);
      SETZERO_L (rest_l);

      PURGEVARS_L ((1, sizeof (b_l), b_l));
      ISPURGED_L  ((1, sizeof (b_l), b_l));

      return E_CLINT_OK;
    }

  i = cmp_l (r_l, b_l);

  if (i == -1)
    {
      cpy_l (rest_l, r_l);
      SETZERO_L (quot_l);

      PURGEVARS_L ((2, sizeof (b_l), b_l,
                       sizeof (r_l), r_l));
      ISPURGED_L  ((2, sizeof (b_l), b_l,
                       sizeof (r_l), r_l));
      return E_CLINT_OK;
    }
  else if (i == 0)
    {
      SETONE_L (quot_l);
      SETZERO_L (rest_l);

      PURGEVARS_L ((2, sizeof (b_l), b_l,
                       sizeof (r_l), r_l));
      ISPURGED_L  ((2, sizeof (b_l), b_l,
                       sizeof (r_l), r_l));
      return E_CLINT_OK;
    }

  if (DIGITS_L (b_l) == 1)
    {
      goto shortdiv;
    }

  /* Schritt 1 */
  msdptrb_l = MSDPTR_L (b_l);

  bn = *msdptrb_l;
  while (bn < BASEDIV2)
    {
      d++;
      bn <<= 1;
    }

  sbitsminusd = (int)BITPERDGT - d;

  if (d > 0)
    {
      bn += *(msdptrb_l - 1) >> sbitsminusd;

      if (DIGITS_L (b_l) > 2)
        {
          bn_1 = (USHORT)((*(msdptrb_l - 1) << d) + (*(msdptrb_l - 2) >> sbitsminusd));
        }
      else
        {
          bn_1 = (USHORT)(*(msdptrb_l - 1) << d);
        }
    }
  else
    {
      bn_1 = (USHORT)(*(msdptrb_l - 1));
    }

  /* Schritte 2 und 3 */
  msdptrr_l = MSDPTR_L (r_l) + 1;
  lsdptrr_l = MSDPTR_L (r_l) - DIGITS_L (b_l) + 1;
  *msdptrr_l = 0;

  qptr_l = quot_l + DIGITS_L (r_l) - DIGITS_L (b_l) + 1;

  /* Schritt 4 */
  while (lsdptrr_l >= LSDPTR_L (r_l))
    {
      ri = (USHORT)((*msdptrr_l << d) + (*(msdptrr_l - 1) >> sbitsminusd));

      ri_1 = (USHORT)((*(msdptrr_l - 1) << d) + (*(msdptrr_l - 2) >> sbitsminusd));

      if (msdptrr_l - 3 > r_l)
        {
          ri_2 = (USHORT)((*(msdptrr_l - 2) << d) + (*(msdptrr_l - 3) >> sbitsminusd));
        }
      else
        {
          ri_2 = (USHORT)(*(msdptrr_l - 2) << d);
        }

      if (ri != bn)               /* fast immer */
        {
          qdach = (USHORT)((rdach = ((ULONG)ri << BITPERDGT) + (ULONG)ri_1) / bn);
          right = ((rdach = (rdach - (ULONG)bn * qdach)) << BITPERDGT) + ri_2;

          /* test qdach */

          if ((left = (ULONG)bn_1 * qdach) > right)
            {
              qdach--;
              if ((rdach + bn) < BASE)
                  /* sonst bn_1 * qdach < rdach * b_l */
                {
                  if ((left - bn_1) > (right + ((ULONG)bn << BITPERDGT)))
                    {
                      qdach--;
                    }
                }
            }
        }
      else                        /* ri == bn, seltenerer Fall */
        {
          qdach = BASEMINONE;
          right = ((ULONG)(rdach = (ULONG)bn + (ULONG)ri_1) << BITPERDGT) + ri_2;
          if (rdach < BASE)       /* sonst ist bn_1 * qdach < rdach * b_l */
            {
              /* test qdach */

              if ((left = (ULONG)bn_1 * qdach) > right)
                {
                  qdach--;
                  if ((rdach + bn) < BASE)
                      /* sonst ist bn_1 * qdach < rdach * b_l */
                    {
                      if ((left - bn_1) > (right + ((ULONG)bn << BITPERDGT)))
                        {
                          qdach--;
                        }
                    }
                }
            }
        }

      /* Schritt 5 */
      borrow = BASE;
      carry = 0;
      for (bptr_l = LSDPTR_L (b_l), rptr_l = lsdptrr_l; bptr_l <= msdptrb_l; bptr_l++, rptr_l++)
        {
          if (borrow >= BASE)
            {
              *rptr_l = (USHORT)(borrow = ((ULONG)(*rptr_l) + BASE -
                         (ULONG)(USHORT)(carry = (ULONG)(*bptr_l) *
                          qdach + (ULONG)(USHORT)(carry >> BITPERDGT))));
            }
          else
            {
              *rptr_l = (USHORT)(borrow = ((ULONG)(*rptr_l) + BASEMINONEL -
                                (ULONG)(USHORT)(carry = (ULONG)(*bptr_l) *
                               qdach + (ULONG)(USHORT)(carry >> BITPERDGT))));
            }
        }

      if (borrow >= BASE)
        {
          *rptr_l = (USHORT)(borrow = ((ULONG)(*rptr_l) + BASE -
                             (ULONG)(USHORT)(carry >> BITPERDGT)));
        }
      else
        {
          *rptr_l = (USHORT)(borrow = ((ULONG)(*rptr_l) + BASEMINONEL -
                                    (ULONG)(USHORT)(carry >> BITPERDGT)));
        }

      /* Schritt 6 */
      *qptr_l = qdach;

      if (borrow < BASE)
        {
          carry = 0;
          for (bptr_l = LSDPTR_L (b_l), rptr_l = lsdptrr_l; bptr_l <= msdptrb_l; bptr_l++, rptr_l++)
            {
              *rptr_l = (USHORT)(carry = ((ULONG)(*rptr_l) + (ULONG)(*bptr_l) +
                                          (ULONG)(USHORT)(carry >> BITPERDGT)));
            }
          *rptr_l += (USHORT)(carry >> BITPERDGT);
          (*qptr_l)--;
        }

      /* Schritt 7 */
      msdptrr_l--;
      lsdptrr_l--;
      qptr_l--;
    }

  /* Schritt 8 */
  SETDIGITS_L (quot_l, DIGITS_L (r_l) - DIGITS_L (b_l) + 1);
  RMLDZRS_L (quot_l);

  SETDIGITS_L (r_l, DIGITS_L (b_l));
  cpy_l (rest_l, r_l);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((17, sizeof (bv), &bv,
                    sizeof (rv), &rv,
                    sizeof (qdach), &qdach,
                    sizeof (ri), &ri,
                    sizeof (ri_1), &ri_1,
                    sizeof (ri_2), &ri_2,
                    sizeof (bn), &bn,
                    sizeof (bn_1), &bn_1,
                    sizeof (right), &right,
                    sizeof (left), &left,
                    sizeof (rdach), &rdach,
                    sizeof (borrow), &borrow,
                    sizeof (carry), &carry,
                    sizeof (sbitsminusd), &sbitsminusd,
                    sizeof (d), &d,
                    sizeof (b_l), b_l,
                    sizeof (r_l), r_l));

  ISPURGED_L  ((17, sizeof (bv), &bv,
                    sizeof (rv), &rv,
                    sizeof (qdach), &qdach,
                    sizeof (ri), &ri,
                    sizeof (ri_1), &ri_1,
                    sizeof (ri_2), &ri_2,
                    sizeof (bn), &bn,
                    sizeof (bn_1), &bn_1,
                    sizeof (right), &right,
                    sizeof (left), &left,
                    sizeof (rdach), &rdach,
                    sizeof (borrow), &borrow,
                    sizeof (carry), &carry,
                    sizeof (sbitsminusd), &sbitsminusd,
                    sizeof (d), &d,
                    sizeof (b_l), b_l,
                    sizeof (r_l), r_l));

  return E_CLINT_OK;

  /* Kurze Division */
  shortdiv:

  rv = 0;
  bv = *LSDPTR_L (b_l);
  for (rptr_l = MSDPTR_L (r_l), qptr_l = quot_l + DIGITS_L (r_l); rptr_l >= LSDPTR_L (r_l); rptr_l--, qptr_l--)
    {
      *qptr_l = (USHORT)((rdach = ((((ULONG)rv) << BITPERDGT) +
                                           (ULONG)*rptr_l)) / bv);
      rv = (USHORT)(rdach - (ULONG)bv * (ULONG)*qptr_l);
    }

  SETDIGITS_L (quot_l, DIGITS_L (r_l));

  RMLDZRS_L (quot_l);
  u2clint_l (rest_l, rv);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((4, sizeof (rv), &rv,
                   sizeof (bv), &bv,
                   sizeof (b_l), b_l,
                   sizeof (r_l), r_l));

  ISPURGED_L  ((4, sizeof (rv), &rv,
                   sizeof (bv), &bv,
                   sizeof (b_l), b_l,
                   sizeof (r_l), r_l));

  return E_CLINT_OK;
}
#endif /* FLINT_ASM */


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Rechtsschieben um 1 Bit                                        */
/*  Syntax:    int shr_l (CLINT a_l);                                         */
/*  Eingabe:   a_l (Argument)                                                 */
/*  Ausgabe:   a_l (Rechtsgeschobener Wert)                                   */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_UFL: Unterlauf                                         */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
shr_l (CLINT a_l)
{
  clint *ap_l;
  USHORT help, carry = 0;

  if (DIGITS_L (a_l) == 0)
    {
      return E_CLINT_UFL;          /* Underflow */
    }

  for (ap_l = MSDPTR_L (a_l); ap_l > a_l; ap_l--)
    {
      help = (USHORT)((USHORT)(*ap_l >> 1) | (USHORT)(carry << (BITPERDGT - 1)));
      carry = (USHORT)(*ap_l & 1U);
      *ap_l = help;
    }

  RMLDZRS_L (a_l);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((2, sizeof (help), &help,
                   sizeof (carry), &carry));

  ISPURGED_L  ((2, sizeof (help), &help,
                   sizeof (carry), &carry));

  return E_CLINT_OK;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Linksschieben um 1 Bit                                         */
/*  Syntax:    int shl_l (CLINT a_l);                                         */
/*  Eingabe:   a_l (Argument)                                                 */
/*  Ausgabe:   a_l (Linksgeschobener Wert)                                    */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_OFL: Ueberlauf                                         */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
shl_l (CLINT a_l)
{
  clint *ap_l, *msdptra_l;
  ULONG carry = 0L;
  int error = E_CLINT_OK;

  RMLDZRS_L (a_l);
  if (ld_l (a_l) >= (USHORT)CLINTMAXBIT)
    {
      SETDIGITS_L (a_l, CLINTMAXDIGIT);
      error = E_CLINT_OFL;         /* Overflow */
    }

  msdptra_l = MSDPTR_L (a_l);
  for (ap_l = LSDPTR_L (a_l); ap_l <= msdptra_l; ap_l++)
    {
      *ap_l = (USHORT)(carry = ((ULONG)(*ap_l) << 1) | (carry >> BITPERDGT));
    }

  if (carry >> BITPERDGT)
    {
      if (DIGITS_L (a_l) < CLINTMAXDIGIT)
        {
          *ap_l = 1;
          INCDIGITS_L (a_l);
          error = E_CLINT_OK;
        }
      else
        {
          error = E_CLINT_OFL;
        }
    }

  RMLDZRS_L (a_l);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((1, sizeof (carry), &carry));
  ISPURGED_L  ((1, sizeof (carry), &carry));

  return error;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Links-/Rechtsschieben um n Bit                                 */
/*  Syntax:    int shift_l (CLINT n_l, long int noofbits);                    */
/*  Eingabe:   n_l (zu schiebendes Argument),                                 */
/*             noofbits (Anzahl zu schiebender Binaerstellen)                 */
/*             negatives Vorzeichen: Schieberichtung rechts                   */
/*             positives Vorzeichen: Schieberichtung links                    */
/*  Ausgabe:   a_l (Geschobener Wert)                                         */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_OFL: Ueberlauf                                         */
/*             E_CLINT_UFL: Unterlauf                                         */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
shift_l (CLINT n_l, long int noofbits)
{
  USHORT shorts = (USHORT)((ULONG)(noofbits < 0 ? -noofbits : noofbits) / BITPERDGT);
  USHORT bits = (USHORT)((ULONG)(noofbits < 0 ? -noofbits : noofbits) % BITPERDGT);
  long int resl;
  USHORT i;
  int error = E_CLINT_OK;

  clint *nptr_l;
  clint *msdptrn_l;

  RMLDZRS_L (n_l);
  resl = (int)ld_l (n_l) + noofbits;

  if (DIGITS_L (n_l) == 0)
    {
      shorts = bits = 0;
      return ((resl < 0) ? E_CLINT_UFL : E_CLINT_OK);
    }

  if (noofbits == 0)
    {
      return E_CLINT_OK;
    }

  if ((resl < 0) || (resl > (long)CLINTMAXBIT))
    {
      error = ((resl < 0) ? E_CLINT_UFL : E_CLINT_OFL);    /* Under-/Overflow */
    }

  SETDIGITS_L (n_l, MIN (DIGITS_L (n_l), CLINTMAXDIGIT));

  if (noofbits < 0)
    {

      /* Shift Right */

      shorts = (USHORT)MIN (DIGITS_L (n_l), shorts);
      msdptrn_l = MSDPTR_L (n_l) - shorts;
      for (nptr_l = LSDPTR_L (n_l); nptr_l <= msdptrn_l; nptr_l++)
        {
          *nptr_l = *(nptr_l + shorts);
        }
      SETDIGITS_L (n_l, DIGITS_L (n_l) - shorts);

      for (i = 0; i < bits; i++)
        {
          shr_l (n_l);
        }
    }
  else
    {

      /* Shift Left   */

      if (shorts < CLINTMAXDIGIT)
        {
          SETDIGITS_L (n_l, MIN ((USHORT)(DIGITS_L (n_l) + shorts), CLINTMAXDIGIT));
          nptr_l = n_l + DIGITS_L (n_l);
          msdptrn_l = n_l + shorts;
          while (nptr_l > msdptrn_l)
            {
              *nptr_l = *(nptr_l - shorts);
              --nptr_l;
            }

          while (nptr_l > n_l)
            {
              *nptr_l-- = 0;
            }

          RMLDZRS_L (n_l);
          for (i = 0; i < bits; i++)
            {
              shl_l (n_l);
            }
        }
      else
        {
          SETZERO_L (n_l);
        }
    }

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((3, sizeof (resl), &resl,
                   sizeof (shorts), &shorts,
                   sizeof (bits), &bits));

  ISPURGED_L  ((3, sizeof (resl), &resl,
                   sizeof (shorts), &shorts,
                   sizeof (bits), &bits));

  return error;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Restbildung                                                    */
/*  Syntax:    int mod_l (CLINT dv_l, CLINT ds_l, CLINT r_l);                 */
/*  Eingabe:   dv_l (Dividend), ds_l (Divisor)                                */
/*  Ausgabe:   r_l (Rest von dv_l mod ds_l)                                   */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_DBZ: Division durch Null                               */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
mod_l (CLINT dv_l, CLINT ds_l, CLINT r_l)
{
  CLINTD junk_l;
  int err;

  err = div_l (dv_l, ds_l, junk_l, r_l);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((1, sizeof (junk_l), junk_l));
  ISPURGED_L  ((1, sizeof (junk_l), junk_l));

  return err;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Restbildung mod Zweierpotenz 2^k                               */
/*  Syntax:    int mod2_l (CLINT d_l, ULONG k, CLINT r_l);                    */
/*  Eingabe:   d_l (Dividend), k (Exponent des Divisors zur Basis 2)          */
/*  Ausgabe:   r_l (Rest)                                                     */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_DBZ: Division durch Null                               */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
mod2_l (CLINT d_l, ULONG k, CLINT r_l)
{
  int i;

  cpy_l (r_l, d_l);

  if (k > CLINTMAXBIT)
    {
      return E_CLINT_OK;
    }

  i = 1 + (k >> LDBITPERDGT);

  if (i > (int)DIGITS_L (r_l))
    {
      return E_CLINT_OK;
    }

  r_l[i] &= (1U << (k & (BITPERDGT - 1UL))) - 1U;
  SETDIGITS_L (r_l, i);            /* r_l[i] = 2^(k mod BITPERDGT) - 1 */

  RMLDZRS_L (r_l);
  return E_CLINT_OK;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Modulare Addition                                              */
/*  Syntax:    int madd_l (CLINT aa_l, CLINT bb_l, CLINT c_l, CLINT m_l);     */
/*  Eingabe:   aa_l, bb_l (Summanden), m_l (Modulus)                          */
/*  Ausgabe:   c_l (Rest von aa_l + bb_l mod m_l)                             */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_DBZ: Division durch Null                               */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
madd_l (CLINT aa_l, CLINT bb_l, CLINT c_l, CLINT m_l)
{
  CLINT a_l, b_l;
  clint tmp_l[CLINTMAXSHORT + 1];

  if (EQZ_L (m_l))
    {
      return E_CLINT_DBZ;          /* Division durch Null */
    }

  cpy_l (a_l, aa_l);
  cpy_l (b_l, bb_l);

  if (GE_L (a_l, m_l) || GE_L (b_l, m_l))
    {
      add (a_l, b_l, tmp_l);
      mod_l (tmp_l, m_l, c_l);
    }
  else
    {
      add (a_l, b_l, tmp_l);
      if (GE_L (tmp_l, m_l))
        {
          sub_l (tmp_l, m_l, tmp_l);    /* Underflow ausgeschlossen */
        }
      cpy_l (c_l, tmp_l);
    }

  Assert(DIGITS_L (c_l) <= CLINTMAXDIGIT);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((3, sizeof (a_l), a_l,
                   sizeof (b_l), b_l,
                   sizeof (tmp_l), tmp_l));

  ISPURGED_L  ((3, sizeof (a_l), a_l,
                   sizeof (b_l), b_l,
                   sizeof (tmp_l), tmp_l));

  return E_CLINT_OK;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Modulare Subtraktion                                           */
/*  Syntax:    int msub_l (CLINT aa_l, CLINT bb_l, CLINT c_l, CLINT m_l);     */
/*  Eingabe:   aa_l (Minuend), bb_l (Subtrahend), m_l (Modulus)               */
/*  Ausgabe:   c_l (Rest von aa_l - bb_l mod m_l)                             */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_DBZ: Division durch Null                               */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
msub_l (CLINT aa_l, CLINT bb_l, CLINT c_l, CLINT m_l)
{
  CLINT a_l, b_l, tmp_l;

  if (EQZ_L (m_l))
    {
      return E_CLINT_DBZ;          /* Division durch Null */
    }

  cpy_l (a_l, aa_l);
  cpy_l (b_l, bb_l);

  if (GE_L (a_l, b_l))
    {
      sub (a_l, b_l, tmp_l);
      mod_l (tmp_l, m_l, c_l);
    }
  else
    {
      sub (b_l, a_l, tmp_l);       /* Vorzeichen tmp_l = -1 */
      mod_l (tmp_l, m_l, tmp_l);
      if (GTZ_L (tmp_l))
        {
          sub (m_l, tmp_l, c_l);
        }
      else
        {
          SETZERO_L (c_l);
        }
    }

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((3, sizeof (a_l), a_l,
                   sizeof (b_l), b_l,
                   sizeof (tmp_l), tmp_l));

  ISPURGED_L  ((3, sizeof (a_l), a_l,
                   sizeof (b_l), b_l,
                   sizeof (tmp_l), tmp_l));

  return E_CLINT_OK;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Modulare Multiplikation                                        */
/*  Syntax:    int mmul_l (CLINT aa_l, CLINT bb_l, CLINT c_l, CLINT m_l);     */
/*  Eingabe:   aa_l, bb_l (Faktoren),  m_l (Modulus)                          */
/*  Ausgabe:   c_l (Rest von aa_l * bb_l mod m_l)                             */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_DBZ: Division durch Null                               */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
mmul_l (CLINT aa_l, CLINT bb_l, CLINT c_l, CLINT m_l)
{
  CLINT a_l, b_l;
  CLINTD tmp_l;

  if (EQZ_L (m_l))
    {
      return E_CLINT_DBZ;          /* Division durch Null */
    }

  cpy_l (a_l, aa_l);
  cpy_l (b_l, bb_l);

  mult (a_l, b_l, tmp_l);
  mod_l (tmp_l, m_l, c_l);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((3, sizeof (a_l), a_l,
                   sizeof (b_l), b_l,
                   sizeof (tmp_l), tmp_l));

  ISPURGED_L  ((3, sizeof (a_l), a_l,
                   sizeof (b_l), b_l,
                   sizeof (tmp_l), tmp_l));

  return E_CLINT_OK;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Modulare Quadrierung                                           */
/*  Syntax:    int msqr_l (CLINT aa_l, CLINT c_l, CLINT m_l);                 */
/*  Eingabe:   aa_l (Faktor),  m_l (Modulus)                                  */
/*  Ausgabe:   c_l (Rest von aa_l * aa_l mod m_l)                             */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_DBZ: Division durch Null                               */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
msqr_l (CLINT aa_l, CLINT c_l, CLINT m_l)
{
  CLINT a_l;
  CLINTD tmp_l;

  if (EQZ_L (m_l))
    {
      return E_CLINT_DBZ;          /* Division durch Null */
    }

  cpy_l (a_l, aa_l);

  sqr (a_l, tmp_l);
  mod_l (tmp_l, m_l, c_l);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((2, sizeof (a_l), a_l,
                   sizeof (tmp_l), tmp_l));

  ISPURGED_L  ((2, sizeof (a_l), a_l,
                   sizeof (tmp_l), tmp_l));

  return E_CLINT_OK;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Montgomery-Multiplikation                                      */
/*  Syntax:    void mulmon_l (CLINT a_l, CLINT b_l, CLINT n_l, USHORT nprime, */
/*                                                 USHORT logB_r, CLINT p_l); */
/*  Eingabe:   a_l, b_l (Faktoren)                                            */
/*             n_l (Modulus, ungerade, n_l > a_l, b_l)                        */
/*             nprime (-n_l^(-1) mod B)                                       */
/*             logB_r (Ganzzahliger Anteil des Logarithmus von r zur Basis B) */
/*             (Zur Bedeutung der Operanden vgl. Kap. 6)                      */
/*  Ausgabe:   p_l (Rest a_l * b_l * r^(-1) mod n_l)                          */
/*             mit r := B^logB_r, B^(logB_r-1) <= n_l < B^logB_r)             */
/*  Rueckgabe: -                                                              */
/*                                                                            */
/******************************************************************************/
void __FLINT_API
mulmon_l(CLINT a_l, CLINT b_l, CLINT n_l, USHORT nprime, USHORT logB_r, CLINT p_l)
{
  clint t_l[2 + (CLINTMAXDIGIT << 1)];
  clint *tptr_l, *nptr_l, *tiptr_l, *lasttnptr, *lastnptr;
  ULONG carry;
  USHORT mi;
  int i;

  mult (a_l, b_l, t_l);
  Assert (DIGITS_L (t_l) <= (1 + (CLINTMAXDIGIT << 1)));

  lasttnptr = t_l + DIGITS_L (n_l);
  lastnptr = MSDPTR_L (n_l);

  for (i = (int)DIGITS_L (t_l) + 1; i <= (int)(DIGITS_L (n_l) << 1); i++)
    {
      Assert (i < sizeof (t_l));
      t_l[i] = 0;
    }

  SETDIGITS_L (t_l, MAX(DIGITS_L (t_l), DIGITS_L (n_l) << 1));

  Assert (DIGITS_L (t_l) <= (CLINTMAXDIGIT << 1));

  for (tptr_l = LSDPTR_L (t_l); tptr_l <= lasttnptr; tptr_l++)
    {
      carry = 0;
      mi = (USHORT)((ULONG)nprime * (ULONG)*tptr_l);
      for (nptr_l = LSDPTR_L (n_l), tiptr_l = tptr_l; nptr_l <= lastnptr; nptr_l++, tiptr_l++)
        {
          Assert (tiptr_l <= t_l + (CLINTMAXDIGIT << 1));
          *tiptr_l = (USHORT)(carry = (ULONG)mi * (ULONG)*nptr_l +
                     (ULONG)*tiptr_l + (ULONG)(USHORT)(carry >> BITPERDGT));
        }

      for (; ((carry >> BITPERDGT) > 0) && tiptr_l <= MSDPTR_L (t_l); tiptr_l++)
        {
          Assert (tiptr_l <= t_l + (CLINTMAXDIGIT << 1));
          *tiptr_l = (USHORT)(carry = (ULONG)*tiptr_l + (ULONG)(USHORT)(carry >> BITPERDGT));
        }

      if (((carry >> BITPERDGT) > 0))
        {
          Assert (tiptr_l <= t_l + 1 + (CLINTMAXDIGIT << 1));
          *tiptr_l = (USHORT)(carry >> BITPERDGT);
          INCDIGITS_L (t_l);
        }
    }

  tptr_l = t_l + logB_r;
  SETDIGITS_L (tptr_l, DIGITS_L (t_l) - logB_r);
  Assert (DIGITS_L (tptr_l) <= (CLINTMAXDIGIT + 1));

  if (GE_L (tptr_l, n_l))
    {
      sub_l (tptr_l, n_l, p_l);
    }
  else
    {
      cpy_l (p_l, tptr_l);
    }

  Assert (DIGITS_L (p_l) <= CLINTMAXDIGIT);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((3, sizeof (mi), &mi,
                   sizeof (carry), &carry,
                   sizeof (t_l), t_l));

  ISPURGED_L  ((3, sizeof (mi), &mi,
                   sizeof (carry), &carry,
                   sizeof (t_l), t_l));
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Montgomery-Quadrierung                                         */
/*  Syntax:    void sqrmon_l (CLINT a_l, CLINT n_l, USHORT nprime,            */
/*                                                 USHORT logB_r, CLINT p_l); */
/*  Eingabe:   a_l (Faktor),  n_l (Modulus, ungerade)                         */
/*             nprime (n' mod B), logB_r (Logarithmus von r zur Basis B)      */
/*             (Zur Bedeutung der Operanden vgl. Kap. 6)                      */
/*  Ausgabe:   p_l (Rest a_l * a_l * r^(-1) mod n_l)                          */
/*             mit r := B^logB_r, B^(logB_r-1) <= n_l < B^logB_r)             */
/*  Rueckgabe: -                                                              */
/*                                                                            */
/******************************************************************************/
void __FLINT_API
sqrmon_l(CLINT a_l, CLINT n_l, USHORT nprime, USHORT logB_r, CLINT p_l)
{
  clint t_l[2 + (CLINTMAXDIGIT << 1)];
  clint *tptr_l, *nptr_l, *tiptr_l, *lasttnptr, *lastnptr;
  ULONG carry;
  USHORT mi;
  int i;

  sqr (a_l, t_l);

  lasttnptr = t_l + DIGITS_L (n_l);
  lastnptr = MSDPTR_L (n_l);

  for (i = (int)DIGITS_L (t_l) + 1; i <= (int)(DIGITS_L (n_l) << 1); i++)
    {
      t_l[i] = 0;
    }

  SETDIGITS_L (t_l, MAX(DIGITS_L (t_l), DIGITS_L (n_l) << 1));

  for (tptr_l = LSDPTR_L (t_l); tptr_l <= lasttnptr; tptr_l++)
    {
      carry = 0;
      mi = (USHORT)((ULONG)nprime * (ULONG)*tptr_l);
      for (nptr_l = LSDPTR_L (n_l), tiptr_l = tptr_l; nptr_l <= lastnptr; nptr_l++, tiptr_l++)
        {
          Assert (tiptr_l <= t_l + (CLINTMAXDIGIT << 1));
          *tiptr_l = (USHORT)(carry = (ULONG)mi * (ULONG)*nptr_l +
                     (ULONG)*tiptr_l + (ULONG)(USHORT)(carry >> BITPERDGT));
        }

      for (; ((carry >> BITPERDGT) > 0) && tiptr_l <= MSDPTR_L (t_l); tiptr_l++)
        {
          Assert (tiptr_l <= t_l + (CLINTMAXDIGIT << 1));
          *tiptr_l = (USHORT)(carry = (ULONG)*tiptr_l + (ULONG)(USHORT)(carry >> BITPERDGT));
        }

      if (((carry >> BITPERDGT) > 0) && tiptr_l > MSDPTR_L (t_l))
        {
          Assert (tiptr_l <= t_l + 1 + (CLINTMAXDIGIT << 1));
          *tiptr_l = (USHORT)(carry >> BITPERDGT);
          INCDIGITS_L (t_l);
        }
    }

  tptr_l = t_l + logB_r;
  SETDIGITS_L (tptr_l, DIGITS_L (t_l) - logB_r);

  if (GE_L (tptr_l, n_l))
    {
      sub_l (tptr_l, n_l, p_l);
    }
  else
    {
      cpy_l (p_l, tptr_l);
    }

  Assert (DIGITS_L (p_l) <= CLINTMAXDIGIT);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((3, sizeof (mi), &mi,
                   sizeof (carry), &carry,
                   sizeof (t_l), t_l));

  ISPURGED_L ((3,  sizeof (mi), &mi,
                   sizeof (carry), &carry,
                   sizeof (t_l), t_l));
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Inverses -n^(-1) mod B fuer ungerade n                         */
/*  Syntax:    USHORT invmon_l (CLINT n_l);                                   */
/*  Eingabe:   n_l (Modulus)                                                  */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: -n^(-1) mod B                                                  */
/*                                                                            */
/******************************************************************************/
USHORT __FLINT_API
invmon_l (CLINT n_l)
{
  unsigned int i;
  ULONG x = 2, y = 1;

  if (ISEVEN_L (n_l))
    {
      return (USHORT)E_CLINT_MOD;
    }

  for (i = 2; i <= BITPERDGT; i++, x <<= 1)
    {
      if (x < (((ULONG)((ULONG)(*LSDPTR_L (n_l)) * (ULONG)y)) & ((x << 1) - 1)))
        {
          y += x;
        }
    }

  return (USHORT)(x - y);
}


/******************************************************************************/

static int twotab[] =
{0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0,
 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0,
 3, 0, 1, 0, 2, 0, 1, 0, 7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6, 0, 1, 0, 2, 0, 1, 0,
 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0};


static USHORT oddtab[] =
{0, 1, 1, 3, 1, 5, 3, 7, 1, 9, 5, 11, 3, 13, 7, 15, 1, 17, 9, 19, 5, 21, 11, 23, 3, 25, 13, 27, 7, 29, 15, 31, 1,
 33, 17, 35, 9, 37, 19, 39, 5, 41, 21, 43, 11, 45, 23, 47, 3, 49, 25, 51, 13, 53, 27, 55, 7, 57, 29, 59, 15,
 61, 31, 63, 1, 65, 33, 67, 17, 69, 35, 71, 9, 73, 37, 75, 19, 77, 39, 79, 5, 81, 41, 83, 21, 85, 43, 87, 11,
 89, 45, 91, 23, 93, 47, 95, 3, 97, 49, 99, 25, 101, 51, 103, 13, 105, 53, 107, 27, 109, 55, 111, 7, 113,
 57, 115, 29, 117, 59, 119, 15, 121, 61, 123, 31, 125, 63, 127, 1, 129, 65, 131, 33, 133, 67, 135, 17,
 137, 69, 139, 35, 141, 71, 143, 9, 145, 73, 147, 37, 149, 75, 151, 19, 153, 77, 155, 39, 157, 79, 159,
 5, 161, 81, 163, 41, 165, 83, 167, 21, 169, 85, 171, 43, 173, 87, 175, 11, 177, 89, 179, 45, 181, 91,
 183, 23, 185, 93, 187, 47, 189, 95, 191, 3, 193, 97, 195, 49, 197, 99, 199, 25, 201, 101, 203, 51, 205,
 103, 207, 13, 209, 105, 211, 53, 213, 107, 215, 27, 217, 109, 219, 55, 221, 111, 223, 7, 225, 113,
 227, 57, 229, 115, 231, 29, 233, 117, 235, 59, 237, 119, 239, 15, 241, 121, 243, 61, 245, 123, 247, 31,
 249, 125, 251, 63, 253, 127, 255};


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Modulare Potenzierung mit automatischer Anwendung der          */
/*             Montgomery-Potenzierung mexpkm_l, falls Modulus ungerade ist,  */
/*             ansonsten wird mexpk_l verwendet.                              */
/*  Syntax:    int mexp_l (CLINT bas_l, CLINT exp_l, CLINT p_l, CLINT m_l);   */
/*  Eingabe:   bas_l (Basis), exp_l (Exponent), m_l (Modulus)                 */
/*  Ausgabe:   p_l (Rest der Potenz bas_l^exp_l mod m_l)                      */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_DBZ: Division durch Null                               */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
mexp_l (CLINT bas_l, CLINT exp_l, CLINT p_l, CLINT m_l)
{
  if (ISODD_L (m_l))              /* Montgomery-Potenzierung moeglich */
    {
      return mexpkm_l (bas_l, exp_l, p_l, m_l);
    }
  else
    {
      return mexpk_l (bas_l, exp_l, p_l, m_l);
    }
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Modulare Potenzierung (Darstellung d. Exponenten z. Basis 2^5) */
/*  Syntax:    int mexp5_l (CLINT bas_l, CLINT exp_l, CLINT p_l, CLINT m_l);  */
/*  Eingabe:   bas_l (Basis), exp_l (Exponent), m_l (Modulus)                 */
/*  Ausgabe:   p_l (Rest der Potenz bas_l^exp_l mod m_l)                      */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_DBZ: Division durch Null                               */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
mexp5_l (CLINT bas_l, CLINT exp_l, CLINT p_l, CLINT m_l)
{
  CLINT a_l;
  clint e_l[CLINTMAXSHORT + 1];
  CLINTD acc_l;
  CLINT a2_l, a3_l, a5_l, a7_l, a9_l, a11_l, a13_l, a15_l, a17_l, a19_l,
      a21_l, a23_l, a25_l, a27_l, a29_l, a31_l;
  clint *aptr_l[32];
  int i, noofdigits, s, t;
  unsigned int bit, digit, f5, word;

  if (EQZ_L (m_l))
    {
      return E_CLINT_DBZ;          /* Division durch Null */
    }

  if (EQONE_L (m_l))
    {
      SETZERO_L (p_l);             /* Modulus = 1 ==> Rest = 0 */
      return E_CLINT_OK;
    }

  cpy_l (a_l, bas_l);
  cpy_l (e_l, exp_l);

  if (DIGITS_L (e_l) == 0)
    {
      SETONE_L (p_l);

      PURGEVARS_L ((1, sizeof (a_l), a_l));
      ISPURGED_L ((1, sizeof (a_l), a_l));

      return E_CLINT_OK;
    }

  if (DIGITS_L (a_l) == 0)
    {
      SETZERO_L (p_l);

      PURGEVARS_L ((1, sizeof (e_l), e_l));
      ISPURGED_L ((1, sizeof (e_l), e_l));

      return E_CLINT_OK;
    }

  mod_l (a_l, m_l, a_l);

  aptr_l[1] = a_l;
  aptr_l[3] = a3_l;
  aptr_l[5] = a5_l;
  aptr_l[7] = a7_l;
  aptr_l[9] = a9_l;
  aptr_l[11] = a11_l;
  aptr_l[13] = a13_l;
  aptr_l[15] = a15_l;
  aptr_l[17] = a17_l;
  aptr_l[19] = a19_l;
  aptr_l[21] = a21_l;
  aptr_l[23] = a23_l;
  aptr_l[25] = a25_l;
  aptr_l[27] = a27_l;
  aptr_l[29] = a29_l;
  aptr_l[31] = a31_l;

  msqr_l (a_l, a2_l, m_l);
  for (i = 3; i <= 31; i += 2)
    {
      mmul_l (a2_l, aptr_l[i - 2], aptr_l[i], m_l);
    }

  *(MSDPTR_L (e_l) + 1) = 0;     /* Null folgt hoechstwertiger Stelle von e_l */

  noofdigits = (ld_l (e_l) - 1)/5;                               /*lint !e713 */
  f5 = (unsigned)(noofdigits * 5);        /* >>loss of precision<< unkritisch */

      word = (unsigned int)(f5 >> LDBITPERDGT);     /* f5 div 16 */
      bit = (unsigned int)(f5 & (BITPERDGT - 1U));  /* f5 mod 16 */

      digit = ((ULONG)(e_l[word + 1] | ((ULONG)e_l[word + 2] << BITPERDGT)) >> bit) & 0x1f;

  if (digit != 0)                  /* 5-digit > 0 */
    {
      cpy_l (acc_l, aptr_l[oddtab[digit]]);

      t = twotab[digit];
      for (; t > 0; t--)
        {
          msqr_l (acc_l, acc_l, m_l);
        }
    }
  else
    {
      SETONE_L (acc_l);
    }

  for (noofdigits--, f5 -= 5; noofdigits >= 0; noofdigits--, f5 -= 5)
    {
      word = (unsigned int)f5 >> LDBITPERDGT;    /* f5 div 16 */
      bit = f5 & (BITPERDGT - 1UL);              /* f5 mod 16 */

      digit = ((ULONG)(e_l[word + 1] | ((ULONG)e_l[word + 2] << BITPERDGT)) >> bit) & 0x1f;

      if (digit != 0)              /* 5-digit > 0 */
        {
          t = twotab[digit];

          for (s = 5 - t; s > 0; s--)
            {
              msqr_l (acc_l, acc_l, m_l);
            }

          mmul_l (acc_l, aptr_l[oddtab[digit]], acc_l, m_l);

          for (; t > 0; t--)
            {
              msqr_l (acc_l, acc_l, m_l);
            }
        }
      else                         /* 5-digit > 0 */
        {
          for (s = 5; s > 0; s--)
            {
              msqr_l (acc_l, acc_l, m_l);
            }
        }
    }

  cpy_l (p_l, acc_l);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((8,  sizeof (i), &i,
                    sizeof (noofdigits), &noofdigits,
                    sizeof (s), &s,
                    sizeof (t), &t,
                    sizeof (bit), &bit,
                    sizeof (digit), &digit,
                    sizeof (f5), &f5,
                    sizeof (word), &word));
  PURGEVARS_L ((19, sizeof (acc_l), acc_l,
                    sizeof (a_l), a_l,
                    sizeof (e_l), e_l,
                    sizeof (a2_l), a2_l,
                    sizeof (a3_l), a3_l,
                    sizeof (a5_l), a5_l,
                    sizeof (a7_l), a7_l,
                    sizeof (a9_l), a9_l,
                    sizeof (a11_l), a11_l,
                    sizeof (a13_l), a13_l,
                    sizeof (a15_l), a15_l,
                    sizeof (a17_l), a17_l,
                    sizeof (a19_l), a19_l,
                    sizeof (a21_l), a21_l,
                    sizeof (a23_l), a23_l,
                    sizeof (a25_l), a25_l,
                    sizeof (a27_l), a27_l,
                    sizeof (a29_l), a29_l,
                    sizeof (a31_l), a31_l));

  ISPURGED_L  ((27, sizeof (i), &i,
                    sizeof (noofdigits), &noofdigits,
                    sizeof (s), &s,
                    sizeof (t), &t,
                    sizeof (bit), &bit,
                    sizeof (digit), &digit,
                    sizeof (f5), &f5,
                    sizeof (word), &word,
                    sizeof (acc_l), acc_l,
                    sizeof (a_l), a_l,
                    sizeof (e_l), e_l,
                    sizeof (a2_l), a2_l,
                    sizeof (a3_l), a3_l,
                    sizeof (a5_l), a5_l,
                    sizeof (a7_l), a7_l,
                    sizeof (a9_l), a9_l,
                    sizeof (a11_l), a11_l,
                    sizeof (a13_l), a13_l,
                    sizeof (a15_l), a15_l,
                    sizeof (a17_l), a17_l,
                    sizeof (a19_l), a19_l,
                    sizeof (a21_l), a21_l,
                    sizeof (a23_l), a23_l,
                    sizeof (a25_l), a25_l,
                    sizeof (a27_l), a27_l,
                    sizeof (a29_l), a29_l,
                    sizeof (a31_l), a31_l));

  return E_CLINT_OK;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Modulare Potenzierung (Darstellung d. Exponenten z. Basis 2^k) */
/*  Syntax:    int mexpk_l (CLINT bas_l, CLINT exp_l, CLINT p_l, CLINT m_l);  */
/*  Eingabe:   bas_l (Basis), exp_l (Exponent), m_l (Modulus)                 */
/*  Ausgabe:   p_l (Rest der Potenz bas_l^exp_l mod m_l)                      */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_DBZ: Division durch Null                               */
/*             E_CLINT_MAL: Fehler bei malloc()                               */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
mexpk_l (CLINT bas_l, CLINT exp_l, CLINT p_l, CLINT m_l)
{
  CLINT a_l, a2_l;
  clint e_l[CLINTMAXSHORT + 1];
  CLINTD acc_l;
  clint **aptr_l, *ptr_l = NULL;
  int noofdigits, s, t, i;
  unsigned int k, lge, bit, digit, fk, word, pow2k, k_mask;

  if (EQZ_L (m_l))
    {
      return E_CLINT_DBZ;          /* Division durch Null */
    }

  if (EQONE_L (m_l))
    {
      SETZERO_L (p_l);             /* Modulus = 1 ==> Rest = 0 */
      return E_CLINT_OK;
    }

  cpy_l (a_l, bas_l);
  cpy_l (e_l, exp_l);

  if (DIGITS_L (e_l) == 0)
    {
      SETONE_L (p_l);

      PURGEVARS_L ((1, sizeof (a_l), a_l));
      ISPURGED_L  ((1, sizeof (a_l), a_l));

      return E_CLINT_OK;
    }

  if (DIGITS_L (a_l) == 0)
    {
      SETZERO_L (p_l);

      PURGEVARS_L ((1, sizeof (e_l), e_l));
      ISPURGED_L  ((1, sizeof (e_l), e_l));

      return E_CLINT_OK;
    }

  lge = ld_l (e_l);

  k = 8;

  while (k > 1 && ((k - 1) * (k << ((k - 1) << 1)) / ((1 << k) - k - 1)) >= lge - 1)
    {
      --k;
    }

  pow2k = 1U << k;                 /*lint !e644*/

#if defined FLINT_DEBUG && defined FLINT_VERBOSE
  printf ("ld(e) = %d, k = %ld, pow2k = %u\n", lge, k, pow2k);
#endif

  k_mask = pow2k - 1U;

  if ((aptr_l = (clint **)malloc (sizeof (clint *) * pow2k)) == NULL)
    {
      PURGEVARS_L ((2, sizeof (a_l), a_l,
                       sizeof (e_l), e_l));
      ISPURGED_L  ((2, sizeof (a_l), a_l,
                       sizeof (e_l), e_l));
      return E_CLINT_MAL;
    }

  mod_l (a_l, m_l, a_l);
  aptr_l[1] = a_l;

  if (k > 1)
    {
      if ((ptr_l = (clint *)malloc (sizeof (CLINT) * ((pow2k >> 1) - 1))) == NULL)
        {
          free (aptr_l);
          PURGEVARS_L ((2, sizeof (a_l), a_l,
                           sizeof (e_l), e_l));
          ISPURGED_L  ((2, sizeof (a_l), a_l,
                           sizeof (e_l), e_l));
          return E_CLINT_MAL;
        }
      aptr_l[2] = a2_l;
      msqr_l (a_l, aptr_l[2], m_l);

      for (aptr_l[3] = ptr_l, i = 5; i < (int)pow2k; i += 2)
        {
          aptr_l[i] = aptr_l[i - 2] + CLINTMAXSHORT;   /*lint !e661 !e662 */
        }

      for (i = 3; i < (int)pow2k; i += 2)
        {
          mmul_l (aptr_l[2], aptr_l[i - 2], aptr_l[i], m_l);
        }
    }

  *(MSDPTR_L (e_l) + 1) = 0;      /* 0 folgt hoechstwertiger Stelle von e_l */

  noofdigits = (lge - 1)/k;       /*lint !e713 */
  fk = noofdigits * k;            /*  >>loss of precision<< ist unkritisch */

  word = (unsigned int)(fk >> LDBITPERDGT);         /* fk div 16 */
  bit = (unsigned int)(fk & (BITPERDGT - 1UL));     /* fk mod 16 */

  switch (k)
    {
      case 1:
      case 2:
      case 4:
      case 8:
        digit = ((ULONG)(e_l[word + 1]) >> bit) & k_mask;
        break;
      default:
        digit = ((ULONG)(e_l[word + 1] | ((ULONG)e_l[word + 2]
                                 << BITPERDGT)) >> bit) & k_mask;
    }

  if (digit != 0)                  /* k-digit > 0 */
    {
      cpy_l (acc_l, aptr_l[oddtab[digit]]);

      t = twotab[digit];
      for (; t > 0; t--)
        {
          msqr_l (acc_l, acc_l, m_l);
        }
    }
  else
    {
      SETONE_L (acc_l);
    }

  for (noofdigits--, fk -= k; noofdigits >= 0; noofdigits--, fk -= k)
    {
      word = (unsigned int)(fk >> LDBITPERDGT);       /* fk div 16 */
      bit = (unsigned int)(fk & (BITPERDGT - 1UL));   /* fk mod 16 */

      switch (k)
        {
          case 1:
          case 2:
          case 4:
          case 8:
            digit = ((ULONG)(e_l[word + 1]) >> bit) & k_mask;
            break;
          default:
            digit = ((ULONG)(e_l[word + 1] | ((ULONG)e_l[word + 2]
                                     << BITPERDGT)) >> bit) & k_mask;
        }

      if (digit != 0)              /* k-digit > 0 */
        {
          t = twotab[digit];

          for (s = (int)(k - t); s > 0; s--)
            {
              msqr_l (acc_l, acc_l, m_l);
            }

          mmul_l (acc_l, aptr_l[oddtab[digit]], acc_l, m_l);

          for (; t > 0; t--)
            {
              msqr_l (acc_l, acc_l, m_l);
            }
        }
      else                         /* k-digit == 0 */
        {
          for (s = (int)k; s > 0; s--)
            {
              msqr_l (acc_l, acc_l, m_l);
            }
        }
    }

  cpy_l (p_l, acc_l);

  free (aptr_l);
  if (ptr_l != NULL)
    {

#ifdef FLINT_SECURE
      memset (ptr_l, 0, sizeof (CLINT) * ((pow2k >> 1) - 1));   /*lint !e668*/
#endif

      free (ptr_l);                /*lint !e644 */
    }

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((12, sizeof (i), &i,
                    sizeof (noofdigits), &noofdigits,
                    sizeof (s), &s,
                    sizeof (t), &t,
                    sizeof (bit), &bit,
                    sizeof (digit), &digit,
                    sizeof (k), &k,
                    sizeof (lge), &lge,
                    sizeof (fk), &fk,
                    sizeof (word), &word,
                    sizeof (pow2k), &pow2k,
                    sizeof (k_mask), &k_mask));
  PURGEVARS_L ((4,  sizeof (a_l), a_l,
                    sizeof (a2_l), a2_l,
                    sizeof (e_l), e_l,
                    sizeof (acc_l), acc_l));

  ISPURGED_L  ((16, sizeof (i), &i,
                    sizeof (noofdigits), &noofdigits,
                    sizeof (s), &s,
                    sizeof (t), &t,
                    sizeof (bit), &bit,
                    sizeof (digit), &digit,
                    sizeof (k), &k,
                    sizeof (lge), &lge,
                    sizeof (fk), &fk,
                    sizeof (word), &word,
                    sizeof (pow2k), &pow2k,
                    sizeof (k_mask), &k_mask,
                    sizeof (a_l), a_l,
                    sizeof (a2_l), a2_l,
                    sizeof (e_l), e_l,
                    sizeof (acc_l), acc_l));

  return E_CLINT_OK;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Modulare Potenzierung (Darstellung d. Exponenten z. Basis 2^5) */
/*             fuer ungerade Moduli, Montgomery-Version                       */
/*  Syntax:    int mexp5m_l (CLINT bas_l, CLINT exp_l, CLINT p_l, CLINT m_l); */
/*  Eingabe:   bas_l (Basis), exp_l (Exponent), m_l (Modulus)                 */
/*  Ausgabe:   p_l (Rest der Potenz bas_l^exp_l mod m_l)                      */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_DBZ: Division durch Null                               */
/*             E_CLINT_MOD: Modulus nicht ungerade                            */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
mexp5m_l (CLINT bas_l, CLINT exp_l, CLINT p_l, CLINT m_l)
{
  CLINT a_l, md_l;
  clint e_l[CLINTMAXSHORT + 1];
  clint r_l[CLINTMAXSHORT + 1];
  CLINTD acc_l;
  CLINT a2_l, a3_l, a5_l, a7_l, a9_l, a11_l, a13_l, a15_l, a17_l, a19_l,
      a21_l, a23_l, a25_l, a27_l, a29_l, a31_l;
  clint *aptr_l[32];
  int i, noofdigits, s, t;
  unsigned int bit, digit, f5, word;
  USHORT logB_r, mprime;

  if (EQZ_L (m_l))
    {
      return E_CLINT_DBZ;          /* Division durch Null */
    }

  if (ISEVEN_L (m_l))
    {
      return E_CLINT_MOD;          /* Modulus nicht ungerade */
    }

  if (EQONE_L (m_l))
    {
      SETZERO_L (p_l);             /* Modulus = 1 ==> Rest = 0 */
      return E_CLINT_OK;
    }

  cpy_l (a_l, bas_l);
  cpy_l (e_l, exp_l);
  cpy_l (md_l, m_l);

  if (DIGITS_L (e_l) == 0)
    {
      SETONE_L (p_l);

      PURGEVARS_L ((2, sizeof (a_l), a_l,
                       sizeof (md_l), md_l));
      ISPURGED_L  ((2, sizeof (a_l), a_l,
                       sizeof (md_l), md_l));
      return E_CLINT_OK;
    }

  if (DIGITS_L (a_l) == 0)
    {
      SETZERO_L (p_l);

      PURGEVARS_L ((2, sizeof (e_l), e_l,
                       sizeof (md_l), md_l));
      ISPURGED_L  ((2, sizeof (e_l), e_l,
                       sizeof (md_l), md_l));
      return E_CLINT_OK;
    }

  aptr_l[1] = a_l;
  aptr_l[3] = a3_l;
  aptr_l[5] = a5_l;
  aptr_l[7] = a7_l;
  aptr_l[9] = a9_l;
  aptr_l[11] = a11_l;
  aptr_l[13] = a13_l;
  aptr_l[15] = a15_l;
  aptr_l[17] = a17_l;
  aptr_l[19] = a19_l;
  aptr_l[21] = a21_l;
  aptr_l[23] = a23_l;
  aptr_l[25] = a25_l;
  aptr_l[27] = a27_l;
  aptr_l[29] = a29_l;
  aptr_l[31] = a31_l;

  SETZERO_L (r_l);
  logB_r = DIGITS_L (md_l);
  setbit_l (r_l, logB_r << LDBITPERDGT);
  if (DIGITS_L (r_l) > CLINTMAXDIGIT)
    {
      mod_l (r_l, md_l, r_l);
    }

  mprime = invmon_l (md_l);

  mmul_l (a_l, r_l, a_l, md_l);

  sqrmon_l (a_l, md_l, mprime, logB_r, a2_l);

  for (i = 3; i <= 31; i += 2)
    {
      mulmon_l (a2_l, aptr_l[i - 2], md_l, mprime, logB_r, aptr_l[i]);
    }

  *(MSDPTR_L (e_l) + 1) = 0;      /* 0 folgt hoechstwertiger Stelle von e_l */

  noofdigits = (ld_l (e_l) - 1)/5;                             /*lint !e713 */
  f5 = (unsigned)(noofdigits * 5);  /* >>loss of precision<< ist unkritisch */

  word = (unsigned int)(f5 >> LDBITPERDGT);         /* f5 div 16 */
  bit = (unsigned int)(f5 & (BITPERDGT - 1UL));     /* f5 mod 16 */

  digit = ((ULONG)(e_l[word + 1] | ((ULONG)e_l[word + 2] << BITPERDGT)) >> bit) & 0x1f;

  if (digit != 0)                  /* 5-digit > 0 */
    {
      cpy_l (acc_l, aptr_l[oddtab[digit]]);

      t = twotab[digit];
      for (; t > 0; t--)
        {
          sqrmon_l (acc_l, md_l, mprime, logB_r, acc_l);
        }
    }
  else
    {
      mod_l (r_l, md_l, acc_l);
    }

  for (noofdigits--, f5 -= 5; noofdigits >= 0; noofdigits--, f5 -= 5)
    {
      word = (unsigned int)f5 >> LDBITPERDGT;       /* f5 div 16 */
      bit = (unsigned int)f5 & (BITPERDGT - 1UL);   /* f5 mod 16 */

      digit = ((ULONG)(e_l[word + 1] | ((ULONG)e_l[word + 2] << BITPERDGT)) >> bit) & 0x1f;

      if (digit != 0)              /* 5-digit == 0 */
        {
          t = twotab[digit];
          for (s = 5 - t; s > 0; s--)
            {
              sqrmon_l (acc_l, md_l, mprime, logB_r, acc_l);
            }

          mulmon_l (acc_l, aptr_l[oddtab[digit]], md_l, mprime, logB_r, acc_l);

          for (; t > 0; t--)
            {
              sqrmon_l (acc_l, md_l, mprime, logB_r, acc_l);
            }
        }
      else                         /* 5-digit == 0 */
        {
          for (s = 5; s > 0; s--)
            {
              sqrmon_l (acc_l, md_l, mprime, logB_r, acc_l);
            }
        }
    }

  mulmon_l (acc_l, one_l, md_l, mprime, logB_r, p_l);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((8,  sizeof (i), &i,
                    sizeof (noofdigits), &noofdigits,
                    sizeof (s), &s,
                    sizeof (t), &t,
                    sizeof (bit), &bit,
                    sizeof (digit), &digit,
                    sizeof (f5), &f5,
                    sizeof (word), &word));
  PURGEVARS_L ((21, sizeof (acc_l), acc_l,
                    sizeof (a_l), a_l,
                    sizeof (md_l), md_l,
                    sizeof (e_l), e_l,
                    sizeof (r_l), r_l,
                    sizeof (a2_l), a2_l,
                    sizeof (a3_l), a3_l,
                    sizeof (a5_l), a5_l,
                    sizeof (a7_l), a7_l,
                    sizeof (a9_l), a9_l,
                    sizeof (a11_l), a11_l,
                    sizeof (a13_l), a13_l,
                    sizeof (a15_l), a15_l,
                    sizeof (a17_l), a17_l,
                    sizeof (a19_l), a19_l,
                    sizeof (a21_l), a21_l,
                    sizeof (a23_l), a23_l,
                    sizeof (a25_l), a25_l,
                    sizeof (a27_l), a27_l,
                    sizeof (a29_l), a29_l,
                    sizeof (a31_l), a31_l));

  ISPURGED_L  ((29, sizeof (i), &i,
                    sizeof (noofdigits), &noofdigits,
                    sizeof (s), &s,
                    sizeof (t), &t,
                    sizeof (bit), &bit,
                    sizeof (digit), &digit,
                    sizeof (f5), &f5,
                    sizeof (word), &word,
                    sizeof (acc_l), acc_l,
                    sizeof (a_l), a_l,
                    sizeof (md_l), md_l,
                    sizeof (e_l), e_l,
                    sizeof (r_l), r_l,
                    sizeof (a2_l), a2_l,
                    sizeof (a3_l), a3_l,
                    sizeof (a5_l), a5_l,
                    sizeof (a7_l), a7_l,
                    sizeof (a9_l), a9_l,
                    sizeof (a11_l), a11_l,
                    sizeof (a13_l), a13_l,
                    sizeof (a15_l), a15_l,
                    sizeof (a17_l), a17_l,
                    sizeof (a19_l), a19_l,
                    sizeof (a21_l), a21_l,
                    sizeof (a23_l), a23_l,
                    sizeof (a25_l), a25_l,
                    sizeof (a27_l), a27_l,
                    sizeof (a29_l), a29_l,
                    sizeof (a31_l), a31_l));
  return E_CLINT_OK;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Modulare Potenzierung fuer ungerade Moduli (Montgomery-Version)*/
/*  Syntax:    int mexpkm_l (CLINT bas_l, CLINT exp_l, CLINT p_l, CLINT m_l); */
/*  Eingabe:   bas_l (Basis), exp_l (Exponent), m_l (Modulus)                 */
/*  Ausgabe:   p_l (Potenzrest)                                               */
/*  Rueckgabe: E_CLINT_OK falls alles O.K.                                    */
/*             E_CLINT_DBZ: Division durch 0                                  */
/*             E_CLINT_MAL: Fehler bei malloc()                               */
/*             E_CLINT_MOD: Modulus nicht ungerade                            */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
mexpkm_l (CLINT bas_l, CLINT exp_l, CLINT p_l, CLINT m_l)
{
  CLINT a_l, a2_l, md_l;
  clint e_l[CLINTMAXSHORT + 1];
  clint r_l[CLINTMAXSHORT + 1];
  CLINTD acc_l;
  clint **aptr_l, *ptr_l = NULL;
  int noofdigits, s, t, i;
  unsigned int k, lge, bit, digit, fk, word, pow2k, k_mask;
  USHORT logB_r, mprime;

#ifdef FLINT_DEBUG
  int sign_rmin1, sign_mprime;
  CLINTD d_l, mprime_l, rmin1_l;
#endif

  if (EQZ_L (m_l))
    {
      return E_CLINT_DBZ;          /* Division durch Null */
    }

  if (ISEVEN_L (m_l))
    {
      return E_CLINT_MOD;          /* Modulus nicht ungerade */
    }

  if (EQONE_L (m_l))
    {
      SETZERO_L (p_l);             /* Modulus = 1 ==> Rest = 0 */
      return E_CLINT_OK;
    }

  cpy_l (a_l, bas_l);
  cpy_l (e_l, exp_l);
  cpy_l (md_l, m_l);

  if (DIGITS_L (e_l) == 0)
    {
      SETONE_L (p_l);
      PURGEVARS_L ((2, sizeof (a_l), a_l,
                       sizeof (md_l), md_l));
      ISPURGED_L  ((2, sizeof (a_l), a_l,
                       sizeof (md_l), md_l));
      return E_CLINT_OK;
    }

  if (DIGITS_L (a_l) == 0)
    {
      SETZERO_L (p_l);

      PURGEVARS_L ((2, sizeof (e_l), e_l,
                       sizeof (md_l), md_l));
      ISPURGED_L  ((2, sizeof (e_l), e_l,
                       sizeof (md_l), md_l));
      return E_CLINT_OK;
    }

  lge = ld_l (e_l);

  k = 8;

  while (k > 1 && ((k - 1) * (k << ((k - 1) << 1)) / ((1 << k) - k - 1)) >= lge - 1)
    {
      --k;
    }

  pow2k = 1U << k;                 /*lint !e644 */

#if defined FLINT_DEBUG && defined FLINT_VERBOSE
  printf ("ld(e) = %d, k = %ld, pow2k = %u\n", lge, k, pow2k);
#endif

  k_mask = pow2k - 1;

  if ((aptr_l = (clint **)malloc (sizeof (clint *) * pow2k)) == NULL)
    {
      PURGEVARS_L ((3, sizeof (a_l), a_l,
                       sizeof (e_l), e_l,
                       sizeof (md_l), md_l));
      ISPURGED_L  ((3, sizeof (a_l), a_l,
                       sizeof (e_l), e_l,
                       sizeof (md_l), md_l));
      return E_CLINT_MAL;
    }

  aptr_l[1] = a_l;
  SETZERO_L (r_l);
  logB_r = DIGITS_L (md_l);
  setbit_l (r_l, logB_r << LDBITPERDGT);
  if (DIGITS_L (r_l) > CLINTMAXDIGIT)
    {
      mod_l (r_l, md_l, r_l);
    }

  mprime = invmon_l (md_l);

#ifdef FLINT_DEBUG
  if (logB_r < CLINTMAXDIGIT)
    {
      xgcd_l (r_l, md_l, d_l, rmin1_l, &sign_rmin1, mprime_l, &sign_mprime);
      if (sign_mprime > 0)
      {
        msub_l (r_l, mprime_l, mprime_l, r_l);
      }

      Assert(EQONE_L (d_l));
      Assert(*LSDPTR_L (mprime_l) == mprime);
    }
#endif /* FLINT_DEBUG */

  mmul_l (a_l, r_l, a_l, md_l);

  if (k > 1)
    {
      if ((ptr_l = (clint *)malloc (sizeof (CLINT) * ((pow2k >> 1) - 1))) == NULL)
        {
          free (aptr_l);
          PURGEVARS_L ((3, sizeof (a_l), a_l,
                           sizeof (e_l), e_l,
                           sizeof (md_l), md_l));
          ISPURGED_L  ((3, sizeof (a_l), a_l,
                           sizeof (e_l), e_l,
                           sizeof (md_l), md_l));
          return E_CLINT_MAL;
        }

      aptr_l[2] = a2_l;
      sqrmon_l (a_l, md_l, mprime, logB_r, aptr_l[2]);

      for (aptr_l[3] = ptr_l, i = 5; i < (int)pow2k; i += 2)
        {
          aptr_l[i] = aptr_l[i - 2] + CLINTMAXSHORT;   /*lint !e661 !e662 */
        }

      for (i = 3; i < (int)pow2k; i += 2)
        {
          mulmon_l (aptr_l[2], aptr_l[i - 2], md_l, mprime, logB_r, aptr_l[i]);
        }
    }

  *(MSDPTR_L (e_l) + 1) = 0;      /* 0 folgt hoechstwertiger Stelle von e_l */

  noofdigits = (lge - 1)/k;                                    /*lint !e713 */
  fk = noofdigits * k;              /* >>loss of precision<< ist unkritisch */

  word = (unsigned int)(fk >> LDBITPERDGT);        /* fk div 16 */
  bit = (unsigned int)(fk & (BITPERDGT - 1UL));    /* fk mod 16 */

  switch (k)
    {
      case 1:
      case 2:
      case 4:
      case 8:
        digit = ((ULONG)(e_l[word + 1]) >> bit) & k_mask;
        break;
      default:
        digit = ((ULONG)(e_l[word + 1] | ((ULONG)e_l[word + 2]
                                 << BITPERDGT)) >> bit) & k_mask;
    }

  if (digit != 0)                  /* k-digit > 0 */
    {
      cpy_l (acc_l, aptr_l[oddtab[digit]]);

      t = twotab[digit];
      for (; t > 0; t--)
        {
          sqrmon_l (acc_l, md_l, mprime, logB_r, acc_l);
        }
    }
  else
    {
      mod_l (r_l, md_l, acc_l);
    }

  for (noofdigits--, fk -= k; noofdigits >= 0; noofdigits--, fk -= k)
    {
      word = (unsigned int)fk >> LDBITPERDGT;       /* fk div 16 */
      bit = (unsigned int)fk & (BITPERDGT - 1UL);   /* fk mod 16 */

      switch (k)
        {
          case 1:
          case 2:
          case 4:
          case 8:
            digit = ((ULONG)(e_l[word + 1]) >> bit) & k_mask;
            break;
          default:
            digit = ((ULONG)(e_l[word + 1] | ((ULONG)e_l[word + 2]
                                     << BITPERDGT)) >> bit) & k_mask;
        }

      if (digit != 0)              /* k-digit > 0 */
        {
          t = twotab[digit];

          for (s = (int)(k - t); s > 0; s--)
            {
              sqrmon_l (acc_l, md_l, mprime, logB_r, acc_l);
            }

          mulmon_l (acc_l, aptr_l[oddtab[digit]], md_l, mprime, logB_r, acc_l);

          for (; t > 0; t--)
            {
              sqrmon_l (acc_l, md_l, mprime, logB_r, acc_l);
            }
        }
      else                         /* k-digit == 0 */
        {
          for (s = (int)k; s > 0; s--)
            {
              sqrmon_l (acc_l, md_l, mprime, logB_r, acc_l);
            }
        }
    }

  mulmon_l (acc_l, one_l, md_l, mprime, logB_r, p_l);

#ifdef FLINT_SECURE
  memset (aptr_l, 0, sizeof (clint *) * pow2k);
#endif

  free (aptr_l);
  if (ptr_l != NULL)
    {

#ifdef FLINT_SECURE
      memset (ptr_l, 0, sizeof (CLINT) * ((pow2k >> 1) - 1));   /*lint !e668*/
#endif

      free (ptr_l);                /*lint !e644 */
    }

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((14, sizeof (i), &i,
                    sizeof (noofdigits), &noofdigits,
                    sizeof (s), &s,
                    sizeof (t), &t,
                    sizeof (bit), &bit,
                    sizeof (digit), &digit,
                    sizeof (k), &k,
                    sizeof (lge), &lge,
                    sizeof (fk), &fk,
                    sizeof (word), &word,
                    sizeof (pow2k), &pow2k,
                    sizeof (k_mask), &k_mask,
                    sizeof (logB_r), &logB_r,
                    sizeof (mprime), &mprime));
  PURGEVARS_L ((6,  sizeof (a_l), a_l,
                    sizeof (a2_l), a2_l,
                    sizeof (e_l), e_l,
                    sizeof (r_l), r_l,
                    sizeof (acc_l), acc_l,
                    sizeof (md_l), md_l));

  ISPURGED_L  ((20, sizeof (i), &i,
                    sizeof (noofdigits), &noofdigits,
                    sizeof (s), &s,
                    sizeof (t), &t,
                    sizeof (bit), &bit,
                    sizeof (digit), &digit,
                    sizeof (k), &k,
                    sizeof (lge), &lge,
                    sizeof (fk), &fk,
                    sizeof (word), &word,
                    sizeof (pow2k), &pow2k,
                    sizeof (k_mask), &k_mask,
                    sizeof (logB_r), &logB_r,
                    sizeof (mprime), &mprime,
                    sizeof (a_l), a_l,
                    sizeof (a2_l), a2_l,
                    sizeof (e_l), e_l,
                    sizeof (r_l), r_l,
                    sizeof (acc_l), acc_l,
                    sizeof (md_l), md_l));

  return E_CLINT_OK;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Modulare Potenzierung mit Zweierpotenz-Exponenten 2^k          */
/*  Syntax:    int mexp2_l (CLINT a_l, USHORT k, CLINT p_l, CLINT m_l);       */
/*  Eingabe:   a_l (Basis), k (Exponent des Zweierpozenz-Exponenten 2^k)      */
/*             m_l (Modulus)                                                  */
/*  Ausgabe:   p_l (Rest der Potenz a_l^(2^k) mod m_l)                        */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_DBZ: Division durch Null                               */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
mexp2_l (CLINT a_l, USHORT k, CLINT p_l, CLINT m_l)
{
  CLINT tmp_l;
  if (EQZ_L (m_l))
    {
      return E_CLINT_DBZ;          /* Division durch Null */
    }

  if (EQONE_L (m_l))
    {
      SETZERO_L (p_l);             /* Modulus = 1 ==> Rest = 0 */
      return E_CLINT_OK;
    }

  if (k > 0)
    {
      cpy_l (tmp_l, a_l);
      while (k-- > 0)
        {
          msqr_l (tmp_l, tmp_l, m_l);
        }
      cpy_l (p_l, tmp_l);
    }
  else
    {
      mod_l (a_l, m_l, p_l);
    }

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((1, sizeof (tmp_l), tmp_l));
  ISPURGED_L  ((1, sizeof (tmp_l), tmp_l));

  return E_CLINT_OK;
}


/******************************************************************************/
/*                                                                            */
/* Arithmetik-Interface mit CLINT- und USHORT-Typen (Mixed-Funktionen)        */
/*                                                                            */
/* Funktionen:                                                                */
/*      wmexp_l, wmexpm_l                                                     */
/*                                                                            */
/* Das 1. Argument ist jeweils vom Typ USHORT                                 */
/*                                                                            */
/* Funktionen:                                                                */
/*      uadd_l, umadd_l, usub_l, umsub_l, umul_l, ummul_l, udiv_l, umod_l     */
/*      umexp_l, umexpm_l                                                     */
/*                                                                            */
/* Das 2. Argument ist jeweils vom Typ USHORT                                 */
/*                                                                            */
/******************************************************************************/


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Addition CLINT und USHORT-Typ                                  */
/*  Syntax:    int uadd_l (CLINT a_l, USHORT b, CLINT s_l);                   */
/*  Eingabe:   a_l, b (Summanden)                                             */
/*  Ausgabe:   s_l (Summe)                                                    */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_OFL: Ueberlauf                                         */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
uadd_l (CLINT a_l, USHORT b, CLINT s_l)
{
  int err;
  CLINT tmp_l;

  u2clint_l (tmp_l, b);
  err = add_l (a_l, tmp_l, s_l);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((1, sizeof (tmp_l), tmp_l));
  ISPURGED_L  ((1, sizeof (tmp_l), tmp_l));

  return err;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Modulare Addition CLINT und USHORT-Typ                         */
/*  Syntax:    int umadd_l (CLINT a_l, USHORT b, CLINT s_l, CLINT m_l);       */
/*  Eingabe:   a_l, b (Summanden), m_l (Modulus)                              */
/*  Ausgabe:   s_l (Rest der Summe a_l + b mod m_l)                           */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_DBZ: Division durch Null                               */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
umadd_l (CLINT a_l, USHORT b, CLINT s_l, CLINT m_l)
{
  int err;
  CLINT tmp_l;

  u2clint_l (tmp_l, b);
  err = madd_l (a_l, tmp_l, s_l, m_l);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((1, sizeof (tmp_l), tmp_l));
  ISPURGED_L  ((1, sizeof (tmp_l), tmp_l));

  return err;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Subtraktion CLINT und USHORT-Typ                               */
/*  Syntax:    int usub_l (CLINT a_l, USHORT b, CLINT d_l);                   */
/*  Eingabe:   a_l (Minuend), b (Subtrahend)                                  */
/*  Ausgabe:   d_l (Differenz)                                                */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_UFL: Unterlauf                                         */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
usub_l (CLINT a_l, USHORT b, CLINT d_l)
{
  int err;
  CLINT tmp_l;

  u2clint_l (tmp_l, b);
  err = sub_l (a_l, tmp_l, d_l);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((1, sizeof (tmp_l), tmp_l));
  ISPURGED_L  ((1, sizeof (tmp_l), tmp_l));

  return err;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Modulare Subtraktion CLINT und USHORT-Typ                      */
/*  Syntax:    int umsub_l (CLINT a_l, USHORT b, CLINT d_l, CLINT m_l);       */
/*  Eingabe:   a_l (Minuend), b (Subtrahend), m_l (Modulus)                   */
/*  Ausgabe:   d_l (Rest der Differenz a_l - b mod m_l)                       */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_DBZ: Division durch Null                               */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
umsub_l (CLINT a_l, USHORT b, CLINT d_l, CLINT m_l)
{
  int err;
  CLINT tmp_l;

  u2clint_l (tmp_l, b);
  err = msub_l (a_l, tmp_l, d_l, m_l);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((1, sizeof (tmp_l), tmp_l));
  ISPURGED_L  ((1, sizeof (tmp_l), tmp_l));

  return err;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Multiplikation CLINT und USHORT-Typ                            */
/*  Syntax:    int umul_l (CLINT aa_l, USHORT b, CLINT pp_l);                 */
/*  Eingabe:   aa_l, b (Faktoren)                                             */
/*  Ausgabe:   pp_l (Produkt)                                                 */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_OFL: Ueberlauf                                         */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
umul_l (CLINT aa_l, USHORT b, CLINT pp_l)
{
  CLINT a_l;
  CLINTD p_l;
  int OFL = 0;

  cpy_l (a_l, aa_l);

  umul (a_l, b, p_l);

  if (DIGITS_L (p_l) > (USHORT)CLINTMAXDIGIT)   /* Overflow ? */
    {
      ANDMAX_L (p_l);              /* Reduziere modulo Nmax+1 */
      OFL = E_CLINT_OFL;
    }

  cpy_l (pp_l, p_l);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((2, sizeof (a_l), a_l,
                   sizeof (p_l), p_l));

  ISPURGED_L  ((2, sizeof (a_l), a_l,
                   sizeof (p_l), p_l));
  return OFL;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Modulare Multiplikation CLINT und USHORT-Typ                   */
/*  Syntax:    int ummul_l (CLINT a_l, USHORT b, CLINT c_l, CLINT m_l);       */
/*  Eingabe:   a_l, b (Faktoren), m_l (Modulus)                               */
/*  Ausgabe:   c_l (Rest des Produktes a_l * b mod m_l)                       */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_DBZ: Division durch Null                               */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
ummul_l (CLINT a_l, USHORT b, CLINT c_l, CLINT m_l)
{
  CLINTD tmp_l;
  int err;

  umul (a_l, b, tmp_l);
  err = mod_l (tmp_l, m_l, c_l);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((1, sizeof (tmp_l), tmp_l));
  ISPURGED_L  ((1, sizeof (tmp_l), tmp_l));

  return err;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Modulare Potenzierung mit USHORT-Basis                         */
/*  Syntax:    int wmexp_l (USHORT bas, CLINT  e_l, CLINT rest_l, CLINT m_l); */
/*  Eingabe:   bas (Basis), e_l (Exponent), m_l (Modulus)                     */
/*  Ausgabe:   rest_l (Rest der Potenz bas^e_l mod m_l)                       */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_DBZ: Division durch Null                               */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
wmexp_l (USHORT bas, CLINT  e_l, CLINT rest_l, CLINT m_l)
{
  CLINT p_l, z_l;
  USHORT k, b, w;

  if (EQZ_L (m_l))
    {
      return E_CLINT_DBZ;          /* Division durch Null */
    }

  if (EQONE_L (m_l))
    {
      SETZERO_L (rest_l);          /* Modulus = 1 ==> Rest = 0 */
      return E_CLINT_OK;
    }

  if (EQZ_L (e_l))
    {
      SETONE_L (rest_l);
      return E_CLINT_OK;
    }

  if (0 == bas)
    {
      SETZERO_L (rest_l);
      return E_CLINT_OK;
    }

  cpy_l (z_l, e_l);
  SETONE_L (p_l);

  b = 1 << ((ld_l (z_l) - 1) & (BITPERDGT - 1UL));
  w = *MSDPTR_L (z_l);

  for (; b > 0; b >>= 1)
    {
      msqr_l (p_l, p_l, m_l);
      if ((w & b) > 0)
        {
          ummul_l (p_l, bas, p_l, m_l);
        }
    }

  for (k = DIGITS_L (z_l) - 1; k > 0; k--)
    {
      w = z_l[k];
      for (b = BASEDIV2; b > 0; b >>= 1)
        {
          msqr_l (p_l, p_l, m_l);
          if ((w & b) > 0)
            {
              ummul_l (p_l, bas, p_l, m_l);
            }
        }
    }

  cpy_l (rest_l, p_l);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((4, sizeof (b), &b,
                   sizeof (w), &w,
                   sizeof (p_l), p_l,
                   sizeof (z_l), z_l));

  ISPURGED_L  ((4, sizeof (b), &b,
                   sizeof (w), &w,
                   sizeof (p_l), p_l,
                   sizeof (z_l), z_l));

  return E_CLINT_OK;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Modulare Potenzierung mit USHORT-Basis fuer ungerade Moduli    */
/*             (Montgomery-Potenzierung)                                      */
/*  Syntax:    int wmexpm_l (USHORT bas, CLINT  e_l, CLINT rest_l, CLINT m_l);*/
/*  Eingabe:   bas (Basis), e_l (Exponent), m_l (Modulus)                     */
/*  Ausgabe:   rest_l (Rest der Potenz bas^e_l mod m_l)                       */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_MOD: Modulus gerade                                    */
/*             E_CLINT_DBZ: Division durch Null                               */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
wmexpm_l (USHORT bas, CLINT  e_l, CLINT rest_l, CLINT m_l)
{
  CLINT p_l, z_l, md_l;
  clint r_l[CLINTMAXSHORT + 1];
  USHORT k, b, w, logB_r, mprime;

#ifdef FLINT_DEBUG
  int sign_rmin1, sign_mprime;
  CLINTD d_l, mprime_l, rmin1_l;
#endif

  if (EQZ_L (m_l))
    {
      return E_CLINT_DBZ;          /* Division durch Null */
    }

  if (ISEVEN_L (m_l))
    {
      return E_CLINT_MOD;          /* Modulus nicht ungerade */
    }

  if (EQONE_L (m_l))
    {
      SETZERO_L (rest_l);          /* Modulus = 1 ==> Rest = 0 */
      return E_CLINT_OK;
    }

  if (EQZ_L (e_l))
    {
      SETONE_L (rest_l);
      return E_CLINT_OK;
    }

  if (0 == bas)
    {
      SETZERO_L (rest_l);
      return E_CLINT_OK;
    }

  cpy_l (md_l, m_l);
  cpy_l (z_l, e_l);

  SETZERO_L (r_l);
  logB_r = DIGITS_L (md_l);
  setbit_l (r_l, logB_r << LDBITPERDGT);

  mprime = invmon_l (md_l);

#ifdef FLINT_DEBUG
  if (logB_r < CLINTMAXDIGIT)
    {
      xgcd_l (r_l, md_l, d_l, rmin1_l, &sign_rmin1, mprime_l, &sign_mprime);
      if (sign_mprime > 0)
        {
          msub_l (r_l, mprime_l, mprime_l, r_l);
        }

      Assert(EQONE_L (d_l));
      Assert(*LSDPTR_L (mprime_l) == mprime);
    }
#endif /* FLINT_DEBUG */

  mod_l (r_l, md_l, p_l);

  b = 1 << ((ld_l (z_l) - 1) & (BITPERDGT - 1UL));
  w = *MSDPTR_L (z_l);

  for (; b > 0; b >>= 1)
    {
      sqrmon_l (p_l, md_l, mprime, logB_r, p_l);

      if ((w & b) > 0)
        {
          ummul_l (p_l, bas, p_l, md_l);
        }
    }

  for (k = DIGITS_L (z_l) - 1; k > 0; k--)
    {
      w = z_l[k];
      for (b = BASEDIV2; b > 0; b >>= 1)
        {
          sqrmon_l (p_l, md_l, mprime, logB_r, p_l);
          if ((w & b) > 0)
            {
              ummul_l (p_l, bas, p_l, md_l);
            }
        }
    }

  mulmon_l (p_l, one_l, md_l, mprime, logB_r, rest_l);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((8, sizeof (b), &b,
                   sizeof (w), &w,
                   sizeof (logB_r), &logB_r,
                   sizeof (mprime), &mprime,
                   sizeof (p_l), p_l,
                   sizeof (z_l), z_l,
                   sizeof (r_l), r_l,
                   sizeof (md_l), md_l));

  ISPURGED_L  ((8, sizeof (b), &b,
                   sizeof (w), &w,
                   sizeof (logB_r), &logB_r,
                   sizeof (mprime), &mprime,
                   sizeof (p_l), p_l,
                   sizeof (z_l), z_l,
                   sizeof (r_l), r_l,
                   sizeof (md_l), md_l));

  return E_CLINT_OK;
}



/******************************************************************************/
/*                                                                            */
/*  Funktion:  Modulare Potenzierung mit USHORT-Exponenten                    */
/*  Syntax:    int umexp_l (CLINT bas_l, USHORT e, CLINT rest_l, CLINT m_l);  */
/*  Eingabe:   bas_l (Basis), e (Exponent), m_l (Modulus)                     */
/*  Ausgabe:   rest_l (Rest der Potenz bas_l^e mod m_l)                       */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_DBZ: Division durch Null                               */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
umexp_l (CLINT bas_l, USHORT e, CLINT rest_l, CLINT m_l)
{
  CLINT tmp_l, tmpbas_l;
  USHORT k = BASEDIV2;
  int err = E_CLINT_OK;

  if (EQZ_L (m_l))
    {
      return E_CLINT_DBZ;          /* Division durch Null */
    }

  if (EQONE_L (m_l))
    {
      SETZERO_L (rest_l);          /* Modulus = 1 ==> Rest = 0 */
      return E_CLINT_OK;
    }

  if (e == 0)
    {
      SETONE_L (rest_l);
      return E_CLINT_OK;
    }

  if (EQZ_L (bas_l))
    {
      SETZERO_L (rest_l);
      return E_CLINT_OK;
    }

  mod_l (bas_l, m_l, tmp_l);
  cpy_l (tmpbas_l, tmp_l);
  while ((e & k) == 0)
    {
      k >>= 1;
    }

  k >>= 1;

  while (k != 0)
    {
      msqr_l (tmp_l, tmp_l, m_l);
      if (e & k)
        {
          mmul_l (tmp_l, tmpbas_l, tmp_l, m_l);
        }
      k >>= 1;
    }

  cpy_l (rest_l, tmp_l);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((2, sizeof (tmp_l), tmp_l,
                   sizeof (tmpbas_l), tmpbas_l));

  ISPURGED_L  ((2, sizeof (tmp_l), tmp_l,
                   sizeof (tmpbas_l), tmpbas_l));

  return err;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Modulare Potenzierung mit USHORT-Exponenten (Montgomery)       */
/*  Syntax:    int umexpm_l (CLINT bas_l, USHORT e, CLINT rest_l, CLINT m_l); */
/*  Eingabe:   bas_l (Basis), e (Exponent), m_l (Modulus)                     */
/*  Ausgabe:   rest_l (Rest der Potenz bas_l^e mod m_l)                       */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_DBZ: Division durch Null                               */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
umexpm_l (CLINT bas_l, USHORT e, CLINT rest_l, CLINT m_l)
{
  CLINT a_l, p_l, md_l;
  clint r_l[CLINTMAXSHORT + 1];
  USHORT k, logB_r, mprime;
  int err = E_CLINT_OK;

#ifdef FLINT_DEBUG
  int sign_rmin1, sign_mprime;
  CLINTD d_l, mprime_l, rmin1_l;
#endif

  if (EQZ_L (m_l))
    {
      return E_CLINT_DBZ;          /* Division durch Null */
    }

  if (ISEVEN_L (m_l))
    {
      return E_CLINT_MOD;          /* Modulus nicht ungerade */
    }

  if (EQONE_L (m_l))
    {
      SETZERO_L (rest_l);          /* Modulus = 1 ==> Rest = 0 */
      return E_CLINT_OK;
    }

  if (e == 0)
    {
      SETONE_L (rest_l);
      return E_CLINT_OK;
    }

  if (EQZ_L (bas_l))
    {
      cpy_l (rest_l, bas_l);
      return E_CLINT_OK;
    }

  if (DIGITS_L (bas_l) > (USHORT)CLINTMAXDIGIT)
    {
      err = E_CLINT_OFL;
    }

  cpy_l (md_l, m_l);

  SETZERO_L (r_l);
  logB_r = DIGITS_L (md_l);
  setbit_l (r_l, logB_r << LDBITPERDGT);
  if (DIGITS_L (r_l) > CLINTMAXDIGIT)
    {
      mod_l (r_l, md_l, r_l);
    }

  mprime = invmon_l (md_l);

#ifdef FLINT_DEBUG
  if (logB_r < CLINTMAXDIGIT)
    {
      xgcd_l (r_l, md_l, d_l, rmin1_l, &sign_rmin1, mprime_l, &sign_mprime);
      if (sign_mprime > 0)
       {
          msub_l (r_l, mprime_l, mprime_l, r_l);
        }

      Assert(EQONE_L (d_l));
      Assert(*LSDPTR_L (mprime_l) == mprime);
    }
#endif /* FLINT_DEBUG */

  mmul_l (bas_l, r_l, p_l, md_l);
  cpy_l (a_l, p_l);

  k = BASEDIV2;

  while ((e & k) == 0)
    {
      k >>= 1;
    }

  k >>= 1;

  while (k != 0)
    {
      sqrmon_l (p_l, md_l, mprime, logB_r, p_l);

      if (e & k)
        {
          mulmon_l (p_l, a_l, md_l, mprime, logB_r, p_l);
        }

      k >>= 1;
    }

  mulmon_l (p_l, one_l, md_l, mprime, logB_r, rest_l);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((5, sizeof (logB_r), &logB_r,
                   sizeof (mprime), &mprime,
                   sizeof (a_l), a_l,
                   sizeof (p_l), p_l,
                   sizeof (md_l), md_l));

  ISPURGED_L  ((5, sizeof (logB_r), &logB_r,
                   sizeof (mprime), &mprime,
                   sizeof (a_l), a_l,
                   sizeof (p_l), p_l,
                   sizeof (md_l), md_l));

  return err;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Division mit Rest mit USHORT-Divisor                           */
/*  Syntax:    int udiv_l (CLINT dv_l, USHORT uds, CLINT q_l, CLINT r_l);     */
/*  Eingabe:   dv_l (Dividend), uds (Divisor)                                 */
/*  Ausgabe:   q_l (Quotient), r_l (Rest)                                     */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_DBZ: Division durch Null                               */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
udiv_l (CLINT dv_l, USHORT uds, CLINT q_l, CLINT r_l)
{
  register clint *aptr_l;
  CLINTD a_l;                      /* Erlaube doppelt langen Dividenden */
  clint *qptr_l, *msdptra_l;
  ULONG rdach;
  USHORT rv;

  cpy_l (a_l, dv_l);

  if (0 == uds)
    {
      PURGEVARS_L ((1, sizeof (a_l), a_l));
      ISPURGED_L  ((1, sizeof (a_l), a_l));

      return E_CLINT_DBZ;          /* Division durch Null */
    }

  if (EQZ_L (a_l))
    {
      SETZERO_L (q_l);
      SETZERO_L (r_l);
      return E_CLINT_OK;
    }

  if (1 == DIGITS_L (a_l))
    {
      if (*LSDPTR_L (a_l) < uds)
        {
          cpy_l (r_l, a_l);
          SETZERO_L (q_l);
        }
      else if (*LSDPTR_L (a_l) == uds)
        {
          SETONE_L (q_l);
          SETZERO_L (r_l);
        }
      else
        {
          u2clint_l (q_l, (USHORT)(*LSDPTR_L (a_l) / uds));
          u2clint_l (r_l, (USHORT)(*LSDPTR_L (a_l) % uds));
        }

      PURGEVARS_L ((1, sizeof (a_l), a_l));
      ISPURGED_L  ((1, sizeof (a_l), a_l));

      return E_CLINT_OK;
    }

  msdptra_l = MSDPTR_L (a_l);

  rv = 0;
  for (aptr_l = msdptra_l, qptr_l = q_l + DIGITS_L (a_l); aptr_l >= LSDPTR_L (a_l); aptr_l--, qptr_l--)
    {
      *qptr_l = (USHORT)((rdach = ((((ULONG)rv) << BITPERDGT) +
                                          (ULONG)*aptr_l)) / uds);
      rv = (USHORT)(rdach - (ULONG)uds * (ULONG)*qptr_l);
    }
  SETDIGITS_L (q_l, DIGITS_L (a_l));

  RMLDZRS_L (q_l);

  if (rv == 0)
    {
      SETZERO_L (r_l);
    }
  else
    {
      u2clint_l (r_l, rv);
    }

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((3, sizeof (rdach), &rdach,
                   sizeof (rv), &rv,
                   sizeof (a_l), a_l));

  ISPURGED_L  ((3, sizeof (rdach), &rdach,
                   sizeof (rv), &rv,
                   sizeof (a_l), a_l));

  return E_CLINT_OK;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Restbildung modulo USHORT-Typ                                  */
/*  Syntax:    USHORT umod_l (CLINT dv_l, USHORT uds);                        */
/*  Eingabe:   dv_l (Dividend), uds (Divisor)                                 */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: Rest der Division dv_l durch uds < 0xffff                      */
/*             0xffff bei Division durch Null                                 */
/*                                                                            */
/******************************************************************************/
USHORT __FLINT_API
umod_l (CLINT dv_l, USHORT uds)
{
  CLINT q_l, r_l;
  USHORT rest;

  if (0 == uds)
    {
      return 0xffff;
    }

  udiv_l (dv_l, uds, q_l, r_l);
  switch (DIGITS_L (r_l))
    {
      case 1:
         rest = *LSDPTR_L (r_l);
        break;
      default:
        rest = 0;
    }

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((2, sizeof (q_l), q_l,
                   sizeof (r_l), r_l));

  ISPURGED_L  ((2, sizeof (q_l), q_l,
                   sizeof (r_l), r_l));

  return rest;
}

/*** Ende der Mixed-Funktionen ************************************************/


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Anzahl der Bits eines CLINT-Wertes                             */
/*             (Ganzzahliger Zweierlogarithmus + 1)                           */
/*  Syntax:    ld_l (n_l);                                                    */
/*  Eingabe:   n_l (Argument)                                                 */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: Anzahl der relevanten Binaerstellen von n_l                    */
/*                                                                            */
/******************************************************************************/
unsigned int __FLINT_API
ld_l (CLINT n_l)
{
  unsigned int l;
  USHORT test;

  l = (unsigned int)DIGITS_L (n_l);
  while (n_l[l] == 0 && l > 0)
    {
      --l;
    }

  if (l == 0)
    {
      return 0;
    }

  test = n_l[l];
  l <<= LDBITPERDGT;

  while ((test & BASEDIV2) == 0)
    {
      test <<= 1;
      --l;
    }

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((1, sizeof (test), &test));
  ISPURGED_L  ((1, sizeof (test), &test));

  return l;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Testen und Setzen eines Bit                                    */
/*  Syntax:    int setbit_l (CLINT a_l, unsigned int pos);                    */
/*  Eingabe:   a_l (Argument), pos (Position des in a_l zu setzenden Bit,     */
/*             gezaehlt ab Position 0)                                        */
/*  Ausgabe:   a_l, Bit in pos gesetzt                                        */
/*  Rueckgabe: 1 falls Bit in Position pos gesetzt war, 0 sonst               */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
setbit_l (CLINT a_l, unsigned int pos)
{
  int res = 0;
  unsigned int i;
  USHORT shortpos = (USHORT)(pos >> LDBITPERDGT);
  USHORT bitpos = (USHORT)(pos & (BITPERDGT - 1));
  USHORT m = (USHORT)(1U << bitpos);

  if (pos > CLINTMAXBIT)
    {
      return E_CLINT_OFL;
    }

  if (shortpos >= DIGITS_L (a_l))
    {
      /* Fuellen mit 0 bis zur Bitposition */
      for (i = DIGITS_L (a_l) + 1; i <= shortpos + 1U; i++)
        {
          a_l[i] = 0;
        }

      /* Neue Laenge in Laengenwort */
      SETDIGITS_L (a_l, shortpos + 1);
    }

  /* Bit testen */
  if (a_l[shortpos + 1] & m)
    {
      res = 1;
    }

  /* Bit setzen */
  a_l[shortpos + 1] |= m;

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((4, sizeof (i), &i,
                   sizeof (shortpos), &shortpos,
                   sizeof (bitpos), &bitpos,
                   sizeof (m), &m));

  ISPURGED_L  ((4, sizeof (i), &i,
                   sizeof (shortpos), &shortpos,
                   sizeof (bitpos), &bitpos,
                   sizeof (m), &m));

  return res;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Testen eines Bit                                               */
/*  Syntax:    int testbit_l (CLINT a_l, unsigned int pos);                   */
/*  Eingabe:   a_l (Argument), pos (Position des in a_l zu testenden Bit,     */
/*             gezaehlt ab Position 0)                                        */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: 1 falls Bit in Position pos gesetzt ist, 0 sonst               */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
testbit_l (CLINT a_l, unsigned int pos)
{
  int res = 0;
  USHORT shortpos = (USHORT)(pos >> LDBITPERDGT);
  USHORT bitpos = (USHORT)(pos & (BITPERDGT - 1));
  if (shortpos < DIGITS_L (a_l))
    {
      if (a_l[shortpos + 1] & (USHORT)(1U << bitpos))
        {
          res = 1;
        }
    }

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((2, sizeof (shortpos), &shortpos,
                   sizeof (bitpos), &bitpos));

  ISPURGED_L  ((2, sizeof (shortpos), &shortpos,
                   sizeof (bitpos), &bitpos));

  return res;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Testen und Loeschen eines Bit                                  */
/*  Syntax:    int clearbit_l (CLINT a_l, unsigned int pos);                  */
/*  Eingabe:   a_l (Argument), pos (Position des in a_l zu loeschenden Bit,   */
/*             gezaehlt ab Position 0)                                        */
/*  Ausgabe:   a_l, Bit in pos geloescht                                      */
/*  Rueckgabe: 1 falls Bit in Position pos gesetzt war, 0 sonst               */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
clearbit_l (CLINT a_l, unsigned int pos)
{
  int res = 0;
  USHORT shortpos = (USHORT)(pos >> LDBITPERDGT);
  USHORT bitpos = (USHORT)(pos & (BITPERDGT - 1));
  USHORT m = (USHORT)(1U << bitpos);

  if (shortpos < DIGITS_L (a_l))
    {
      if (a_l[shortpos + 1] & m)
        {
          res = 1;
        }
      a_l[shortpos + 1] &= (USHORT)(~m);
      RMLDZRS_L (a_l);
    }

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((3, sizeof (shortpos), &shortpos,
                   sizeof (bitpos), &bitpos,
                   sizeof (m), &m));

  ISPURGED_L  ((3, sizeof (shortpos), &shortpos,
                   sizeof (bitpos), &bitpos,
                   sizeof (m), &m));

  return res;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  XOR-Verknuepfung zweier CLINT-Typen                            */
/*  Syntax:    void xor_l (CLINT a_l, CLINT b_l, CLINT c_l);                  */
/*  Eingabe:   a_l, b_l (Operanden)                                           */
/*  Ausgabe:   c_l (XOR-Summe von a_l und b_l)                                */
/*  Rueckgabe: -                                                              */
/*                                                                            */
/******************************************************************************/
void __FLINT_API
xor_l (CLINT a_l, CLINT b_l, CLINT c_l)
{
  CLINT d_l;
  clint *r_l, *s_l, *t_l;
  clint *msdptrr_l;
  clint *msdptrs_l;

  if (DIGITS_L (a_l) < DIGITS_L (b_l))
    {
      r_l = LSDPTR_L (b_l);
      s_l = LSDPTR_L (a_l);
      msdptrr_l = MSDPTR_L (b_l);
      msdptrs_l = MSDPTR_L (a_l);
    }
  else
    {
      r_l = LSDPTR_L (a_l);
      s_l = LSDPTR_L (b_l);
      msdptrr_l = MSDPTR_L (a_l);
      msdptrs_l = MSDPTR_L (b_l);
    }

  t_l = LSDPTR_L (d_l);
  SETDIGITS_L (d_l, DIGITS_L (r_l - 1));

  while (s_l <= msdptrs_l)
    {
      *t_l++ = *r_l++ ^ *s_l++;
    }

  while (r_l <= msdptrr_l)
    {
      *t_l++ = *r_l++;
    }

  cpy_l (c_l, d_l);


  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((1, sizeof (d_l), d_l));
  ISPURGED_L  ((1, sizeof (d_l), d_l));
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  OR-Verknuepfung zweier CLINT-Objekte                           */
/*  Syntax:    void or_l (CLINT a_l, CLINT b_l, CLINT c_l);                   */
/*  Eingabe:   a_l, b_l (Operanden)                                           */
/*  Ausgabe:   c_l (ODER-Verknuepfung von a_l und b_l)                        */
/*  Rueckgabe: -                                                              */
/*                                                                            */
/******************************************************************************/
void __FLINT_API
or_l (CLINT a_l, CLINT b_l, CLINT c_l)
{
  CLINT d_l;
  clint *r_l, *s_l, *t_l;
  clint *msdptrr_l;
  clint *msdptrs_l;

  if (DIGITS_L (a_l) < DIGITS_L (b_l))
    {
      r_l = LSDPTR_L (b_l);
      s_l = LSDPTR_L (a_l);
      msdptrr_l = MSDPTR_L (b_l);
      msdptrs_l = MSDPTR_L (a_l);
    }
  else
    {
      r_l = LSDPTR_L (a_l);
      s_l = LSDPTR_L (b_l);
      msdptrr_l = MSDPTR_L (a_l);
      msdptrs_l = MSDPTR_L (b_l);
    }

  t_l = LSDPTR_L (d_l);
  SETDIGITS_L (d_l, DIGITS_L (r_l - 1));

  while (s_l <= msdptrs_l)
    {
      *t_l++ = *r_l++ | *s_l++;
    }

  while (r_l <= msdptrr_l)
    {
      *t_l++ = *r_l++;
    }

  cpy_l (c_l, d_l);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((1, sizeof (d_l), d_l));
  ISPURGED_L  ((1, sizeof (d_l), d_l));
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  AND-Verknuepfung zweier CLINT-Objekte                          */
/*  Syntax:    void and_l (CLINT a_l, CLINT b_l, CLINT c_l);                  */
/*  Eingabe:   a_l, b_l (Operanden)                                           */
/*  Ausgabe:   c_l (UND-Verknuepfung von a_l und b_l)                         */
/*  Rueckgabe: -                                                              */
/*                                                                            */
/******************************************************************************/
void __FLINT_API
and_l (CLINT a_l, CLINT b_l, CLINT c_l)
{
  CLINT d_l;
  clint *r_l, *s_l, *t_l;
  clint *lastptr_l;

  if (DIGITS_L (a_l) < DIGITS_L (b_l))
    {
      r_l = LSDPTR_L (b_l);
      s_l = LSDPTR_L (a_l);
      lastptr_l = MSDPTR_L (a_l);
    }
  else
    {
      r_l = LSDPTR_L (a_l);
      s_l = LSDPTR_L (b_l);
      lastptr_l = MSDPTR_L (b_l);
    }

  t_l = LSDPTR_L (d_l);
  SETDIGITS_L (d_l, DIGITS_L (s_l - 1));

  while (s_l <= lastptr_l)
    {
      *t_l++ = *r_l++ & *s_l++;
    }

  cpy_l (c_l, d_l);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((1, sizeof (d_l), d_l));
  ISPURGED_L  ((1, sizeof (d_l), d_l));
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Groesster gemeinsamer Teiler zweier CLINT-Objekte              */
/*  Syntax:    void gcd_l (CLINT aa_l, CLINT bb_l, CLINT cc_l);               */
/*  Eingabe:   aa_l, bb_l (Operanden)                                         */
/*  Ausgabe:   cc_l (ggT von a_l und b_l)                                     */
/*  Rueckgabe: -                                                              */
/*                                                                            */
/******************************************************************************/
void __FLINT_API
gcd_l (CLINT aa_l, CLINT bb_l, CLINT cc_l)
{
  CLINT a_l, b_l, r_l, t_l;
  unsigned int k = 0;
  int sign_of_t;

  /* Schritt 1 */
  if (LT_L (aa_l, bb_l))
    {
      cpy_l (a_l, bb_l);
      cpy_l (b_l, aa_l);
    }
  else
    {
      cpy_l (a_l, aa_l);
      cpy_l (b_l, bb_l);
    }

  if (EQZ_L (b_l))
    {
      cpy_l (cc_l, a_l);

      PURGEVARS_L ((1, sizeof (a_l), a_l));
      ISPURGED_L  ((1, sizeof (a_l), a_l));

      return;
    }

  /* Schritt 2 */
  div_l (a_l, b_l, t_l, r_l);
  cpy_l (a_l, b_l);
  cpy_l (b_l, r_l);

  if (EQZ_L (b_l))
    {
      cpy_l (cc_l, a_l);

      k = sign_of_t = 0;
      PURGEVARS_L ((3, sizeof (a_l), a_l,
                       sizeof (t_l), t_l,
                       sizeof (r_l), r_l));

      ISPURGED_L  ((3, sizeof (a_l), a_l,
                       sizeof (t_l), t_l,
                       sizeof (r_l), r_l));
      return;
    }

  while (ISEVEN_L (a_l) && ISEVEN_L (b_l))
    {
      ++k;
      shr_l (a_l);
      shr_l (b_l);
    }

  /* Schritt 3 */
  while (ISEVEN_L (a_l))
    {
      shr_l (a_l);
    }
  while (ISEVEN_L (b_l))
    {
      shr_l (b_l);
    }

  /* Schritt 4 */
  do
    {
      if (GE_L (a_l, b_l))
        {
          sub_l (a_l, b_l, t_l);
          sign_of_t = 1;
        }
      else
        {
          sub_l (b_l, a_l, t_l);
          sign_of_t = -1;
        }

      if (EQZ_L (t_l))
        {                                    /* fertig */
          cpy_l (cc_l, a_l);                 /* cc_l <- a */
          shift_l (cc_l, (long int)k);       /* cc_l <- cc_l*2**k */

          PURGEVARS_L ((3, sizeof (a_l), a_l,
                           sizeof (b_l), b_l,
                           sizeof (r_l), r_l));

          ISPURGED_L  ((3, sizeof (a_l), a_l,
                           sizeof (b_l), b_l,
                           sizeof (r_l), r_l));
          return;
        }

      /* Schritt 5 */
      while (ISEVEN_L (t_l))
        {
          shr_l (t_l);
        }

      if (-1 == sign_of_t)
        {
          cpy_l (b_l, t_l);
        }
      else
        {
          cpy_l (a_l, t_l);
        }
    }
  while (1);    /*lint !e506 Jammere nicht ueber >>constant value boolean<< */
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Groesster gemeinsamer Teiler d = ggT(a,b) und Darstellung des  */
/*             ggT d = au + bv (Erweiterter Euklidischer Algorithmus)         */
/*  Syntax:    void xgcd_l (CLINT a_l, CLINT b_l, CLINT d_l, CLINT u_l,       */
/*                                      int *sign_u, CLINT v_l, int *sign_v); */
/*  Eingabe:   a_l, b_l (Operanden)                                           */
/*  Ausgabe:   d_l (ggT von a_l und b_l)                                      */
/*             u_l, v_l (Faktoren der Darstellung des ggT von a_l und b_l     */
/*             mit Vorzeichen sign_u und sign_v)                              */
/*  Rueckgabe: -                                                              */
/*                                                                            */
/******************************************************************************/
void __FLINT_API
xgcd_l (CLINT a_l, CLINT b_l, CLINT d_l, CLINT u_l, int *sign_u, CLINT v_l, int *sign_v)
{
  CLINT v1_l, v3_l, t1_l, t3_l, q_l;
  CLINTD tmp_l, tmpu_l, tmpv_l;
  int sign_v1, sign_t1;

  cpy_l (d_l, a_l);
  cpy_l (v3_l, b_l);

  if (EQZ_L (v3_l))               /* b_l == 0 ? */
    {
      SETONE_L (u_l);
      SETZERO_L (v_l);
      *sign_u = 1;
      *sign_v = 1;
      return;
    }

  SETONE_L (tmpu_l);
  *sign_u = 1;
  SETZERO_L (v1_l);
  sign_v1 = 1;

  while (GTZ_L (v3_l))
    {
      div_l (d_l, v3_l, q_l, t3_l);
      mul_l (v1_l, q_l, q_l);
      sign_t1 = ssub (tmpu_l, *sign_u, q_l, sign_v1, t1_l);
      cpy_l (tmpu_l, v1_l);
      *sign_u = sign_v1;
      cpy_l (d_l, v3_l);
      cpy_l (v1_l, t1_l);
      sign_v1 = sign_t1;
      cpy_l (v3_l, t3_l);
    }

  mult (a_l, tmpu_l, tmp_l);
  *sign_v = ssub (d_l, 1, tmp_l, *sign_u, tmp_l);
  div_l (tmp_l, b_l, tmpv_l, tmp_l);

  Assert (EQZ_L (tmp_l));

#ifdef FLINT_DEBUG
  {
    CLINTD x_l, y_l, z_l;
    mult (a_l, tmpu_l, x_l);
    mult (b_l, tmpv_l, y_l);
    sadd (x_l, *sign_u, y_l, *sign_v, z_l);
    Assert (equ_l (z_l, d_l));
  }
#endif

  cpy_l (u_l, tmpu_l);
  cpy_l (v_l, tmpv_l);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((10, sizeof (sign_v1), &sign_v1,
                    sizeof (sign_t1), &sign_t1,
                    sizeof (v1_l), v1_l,
                    sizeof (v3_l), v3_l,
                    sizeof (t1_l), t1_l,
                    sizeof (t3_l), t3_l,
                    sizeof (q_l), q_l,
                    sizeof (tmp_l), tmp_l,
                    sizeof (tmpu_l), tmpu_l,
                    sizeof (tmpv_l), tmpv_l));

  ISPURGED_L  ((10, sizeof (sign_v1), &sign_v1,
                    sizeof (sign_t1), &sign_t1,
                    sizeof (v1_l), v1_l,
                    sizeof (v3_l), v3_l,
                    sizeof (t1_l), t1_l,
                    sizeof (t3_l), t3_l,
                    sizeof (q_l), q_l,
                    sizeof (tmp_l), tmp_l,
                    sizeof (tmpu_l), tmpu_l,
                    sizeof (tmpv_l), tmpv_l));

  return;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Multiplikatives Inverses von a modulo n                        */
/*  Syntax:    void inv_l (CLINT a_l, CLINT n_l, CLINT g_l, CLINT i_l);       */
/*  Eingabe:   a_l (Operand), n_l (Modulus)                                   */
/*  Ausgabe:   g_l (ggT von a_l und n_l), i_l (Inverses von a_l mod n_l)      */
/*  Rueckgabe: -                                                              */
/*                                                                            */
/******************************************************************************/
void __FLINT_API
inv_l (CLINT a_l, CLINT n_l, CLINT g_l, CLINT i_l)
{
  CLINT v1_l, v3_l, t1_l, t3_l, q_l;

  if (EQZ_L (a_l))
    {
      if (EQZ_L (n_l))
        {
          SETZERO_L (g_l);
          SETZERO_L (i_l);
          return;
        }
      else
        {
          cpy_l (g_l, n_l);
          SETZERO_L (i_l);
          return;
        }
    }
  else
    {
      if (EQZ_L (n_l))
        {
          cpy_l (g_l, a_l);
          SETZERO_L (i_l);
          return;
        }
    }

  cpy_l (g_l, a_l);
  cpy_l (v3_l, n_l);
  SETZERO_L (v1_l);
  SETONE_L (t1_l);

  do
    {
      div_l (g_l, v3_l, q_l, t3_l);

      if (GTZ_L (t3_l))
        {
          mmul_l (v1_l, q_l, q_l, n_l);
          msub_l (t1_l, q_l, q_l, n_l);
          cpy_l (t1_l, v1_l);
          cpy_l (v1_l, q_l);
          cpy_l (g_l, v3_l);
          cpy_l (v3_l, t3_l);
        }
    }

  while (GTZ_L (t3_l));
  cpy_l (g_l, v3_l);
  if (EQONE_L (g_l))
    {
      cpy_l (i_l, v1_l);
    }
  else
    {
      SETZERO_L (i_l);
    }

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((5, sizeof (v1_l), v1_l,
                   sizeof (v3_l), v3_l,
                   sizeof (t1_l), t1_l,
                   sizeof (t3_l), t3_l,
                   sizeof (q_l), q_l));

  ISPURGED_L  ((5, sizeof (v1_l), v1_l,
                   sizeof (v3_l), v3_l,
                   sizeof (t1_l), t1_l,
                   sizeof (t3_l), t3_l,
                   sizeof (q_l), q_l));
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Kleinstes gemeinsames Vielfaches (kgV) zweier CLINT-Werte      */
/*  Syntax:    int lcm_l (CLINT a_l, CLINT b_l, CLINT c_l);                   */
/*  Eingabe:   a_l, b_l (Operanden)                                           */
/*  Ausgabe:   c_l (kgV von a_l und b_l)                                      */
/*  Rueckgabe: E_CLINT_OK falls alles O.K.                                    */
/*             E_CLINT_OFL bei Ueberlauf                                      */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
lcm_l (CLINT a_l, CLINT b_l, CLINT c_l)
{
  CLINT g_l, junk_l;
  int err;

  if (EQZ_L (a_l) || EQZ_L (b_l))
    {
      SETZERO_L (c_l);
      return E_CLINT_OK;
    }

  gcd_l (a_l, b_l, g_l);
  div_l (a_l, g_l, g_l, junk_l);
  err = mul_l (g_l, b_l, c_l);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((2, sizeof (g_l), g_l,
                   sizeof (junk_l), junk_l));

  ISPURGED_L  ((2, sizeof (g_l), g_l,
                   sizeof (junk_l), junk_l));

  return err;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Test auf Quadrateigenschaft einer Zahl (Cohen, ACCANT, S. 40)  */
/*  Syntax:    unsigned issqr_l (CLINT n_l, CLINT r_l);                       */
/*  Eingabe:   n_l (Argument)                                                 */
/*  Ausgabe:   r_l (Quadratwurzel von n_l, falls n_l Quadrat ist;             */
/*                  r_l == 0 sonst)                                           */
/*  Rueckgabe: 1 falls n_l Quadrat einer Zahl ist                             */
/*             0 falls n_l kein Quadrat ist                                   */
/*                                                                            */
/******************************************************************************/
static const UCHAR q11[11]=
  {1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 0};

static const UCHAR q63[63]=
  {1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1,
   0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0,
   0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0};

static const UCHAR q64[64]=
  {1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1,
   0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
   0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0};

static const UCHAR q65[65]=
  {1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
   1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1,
   0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1};


unsigned __FLINT_API
issqr_l (CLINT n_l, CLINT r_l)
{
  CLINT q_l;
  USHORT r;

  if (EQZ_L (n_l))
    {
      SETZERO_L (r_l);
      return 1;
    }

  if (1 == q64[*LSDPTR_L (n_l) & 63])      /* q64[n_l mod 64] */
    {
      r = umod_l (n_l, 45045);

      if ((1 == q63[r % 63]) && (1 == q65[r % 65]) && (1 == q11[r % 11]))
        {
          iroot_l (n_l, r_l);
          sqr_l (r_l, q_l);

          if (equ_l (n_l, q_l))
            {
              PURGEVARS_L ((1, sizeof (q_l), q_l));
              ISPURGED_L  ((1, sizeof (q_l), q_l));

              return 1;
            }
        }
    }

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((2, sizeof (r), &r,
                   sizeof (q_l), q_l));

  ISPURGED_L  ((2, sizeof (r), &r,
                   sizeof (q_l), q_l));

  return 0;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Ganzzahliger Anteil der Quadratwurzel eines CLINT-Wertes       */
/*  Syntax:    void iroot_l (CLINT a_l, CLINT floor_l);                       */
/*  Eingabe:   a_l (Argument)                                                 */
/*  Ausgabe:   floor_l (Ganzahliger Anteil der Quadratwurzel von a_l)         */
/*  Rueckgabe: -                                                              */
/*                                                                            */
/******************************************************************************/
void __FLINT_API
iroot_l (CLINT a_l, CLINT floor_l)
{
  CLINT x_l, y_l, r_l;
  unsigned l;

  if (EQZ_L (a_l))
    {
      SETZERO_L (floor_l);
      return;
    }

  /* Schritt 1 */
  l = (ld_l (a_l) + 1) >> 1;       /* (ld(a_l) + 2) div 2 */
  SETZERO_L (y_l);
  setbit_l (y_l, l);

  do
    {
      cpy_l (x_l, y_l);
      /* Schritt 2 */
      div_l (a_l, x_l, y_l, r_l);
      add_l (y_l, x_l, y_l);
      shr_l (y_l);
    }

  /* Schritt 3 */
  while (LT_L (y_l, x_l));
  cpy_l (floor_l, x_l);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((4, sizeof (l), &l,
                   sizeof (x_l), x_l,
                   sizeof (y_l), y_l,
                   sizeof (r_l), r_l));

  ISPURGED_L  ((4, sizeof (l), &l,
                   sizeof (x_l), x_l,
                   sizeof (y_l), y_l,
                   sizeof (r_l), r_l));
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Quadratwurzel modulo p, p Primzahl                             */
/*  Syntax:    int proot_l (CLINT a_l, CLINT p_l, CLINT x_l);                 */
/*  Eingabe:   a_l (Quadrat), p_l (Primzahlmodul > 2)                         */
/*  Ausgabe:   x_l (Quadratwurzel von a_l mod p_l)                            */
/*  Rueckgabe: E_CLINT_OK falls alles O.K.                                    */
/*             -1: a_l ist kein quadratischer Rest mod p_l oder p_l gerade    */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
proot_l (CLINT a_l, CLINT p_l, CLINT x_l)
{
  CLINT b_l, q_l, t_l, y_l, z_l;
  int r, m;

  if (EQZ_L (p_l) || ISEVEN_L (p_l))
    {
      return -1;
    }

  if (EQZ_L (a_l))
    {
      SETZERO_L (x_l);
      return E_CLINT_OK;
    }

  cpy_l (q_l, p_l);
  dec_l (q_l);
  r = twofact_l (q_l, q_l);

  /* Schritt 1: Suche quadratischen Nichtrest */
  cpy_l (z_l, two_l);
  while (jacobi_l (z_l, p_l) == 1)
    {
      inc_l (z_l);
    }
  mexp_l (z_l, q_l, z_l, p_l);

  /* Schritt 2: Initialisierung */
  cpy_l (y_l, z_l);
  dec_l (q_l);
  shr_l (q_l);                     /* q_l = (q - 1)/2 */
  mexp_l (a_l, q_l, x_l, p_l);
  msqr_l (x_l, b_l, p_l);
  mmul_l (b_l, a_l, b_l, p_l);
  mmul_l (x_l, a_l, x_l, p_l);

  /* Schritt 3: Suche Exponenten */
  mod_l (b_l, p_l, q_l);

  while (!equ_l (q_l, one_l))
    {
      m = 0;
      do
        {
          ++m;
          msqr_l (q_l, q_l, p_l);       /* q_l = b^(2^m) */
        }
      while (!equ_l (q_l, one_l));      /* m <= r */

      if (m == r)
        {
          PURGEVARS_L ((5, sizeof (b_l), b_l,
                           sizeof (q_l), q_l,
                           sizeof (t_l), t_l,
                           sizeof (y_l), y_l,
                           sizeof (z_l), z_l));

          ISPURGED_L  ((5, sizeof (b_l), b_l,
                           sizeof (q_l), q_l,
                           sizeof (t_l), t_l,
                           sizeof (y_l), y_l,
                           sizeof (z_l), z_l));
          return -1;
        }

      /* Schritt 4: Reduziere Exponenten */

      mexp2_l (y_l, (USHORT)(r - m - 1), t_l, p_l);         /*lint !e732 */
      msqr_l (t_l, y_l, p_l);    /* Jammere nicht ueber >>loss of sign<< */
      mmul_l (x_l, t_l, x_l, p_l);
      mmul_l (b_l, y_l, b_l, p_l);
      cpy_l (q_l, b_l);
      r = m;
    }

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((7, sizeof (r), &r,
                   sizeof (m), &m,
                   sizeof (b_l), b_l,
                   sizeof (q_l), q_l,
                   sizeof (t_l), t_l,
                   sizeof (y_l), y_l,
                   sizeof (z_l), z_l));

  ISPURGED_L  ((7, sizeof (r), &r,
                   sizeof (m), &m,
                   sizeof (b_l), b_l,
                   sizeof (q_l), q_l,
                   sizeof (t_l), t_l,
                   sizeof (y_l), y_l,
                   sizeof (z_l), z_l));

  return E_CLINT_OK;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Quadratwurzel modulo pq; p, q  Primzahlen                      */
/*  Syntax:    int root_l (CLINT a_l, CLINT p_l, CLINT q_l, CLINT x_l);       */
/*  Eingabe:   a_l (Operand), p_l, q_l (Primzahlmoduln)                       */
/*  Ausgabe:   x_l (Quadratwurzel von a_l mod p_l*q_l)                        */
/*  Rueckgabe: E_CLINT_OK falls alles O.K.                                    */
/*             -1: a_l ist keine Quadratwurzel mod p_l*q_l                    */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
root_l (CLINT a_l, CLINT p_l, CLINT q_l, CLINT x_l)
{
  CLINT x0_l, x1_l, x2_l, x3_l, xp_l, xq_l, n_l;
  CLINTD u_l, v_l;
  clint *xptr_l;
  int sign_u, sign_v;

  if (0 != proot_l (a_l, p_l, xp_l) || 0 != proot_l (a_l, q_l, xq_l))
    {
      return -1;
    }

  if (EQZ_L (a_l))
    {
      SETZERO_L (x_l);
      return E_CLINT_OK;
    }

  mul_l (p_l, q_l, n_l);
  xgcd_l (p_l, q_l, x0_l, u_l, &sign_u, v_l, &sign_v);
  mul_l (u_l, p_l, u_l);
  mul_l (u_l, xq_l, u_l);
  mul_l (v_l, q_l, v_l);
  mul_l (v_l, xp_l, v_l);
  sign_u = sadd (u_l, sign_u, v_l, sign_v, x0_l);
  smod (x0_l, sign_u, n_l, x0_l);

  sub_l (n_l, x0_l, x1_l);
  msub_l (u_l, v_l, x2_l, n_l);
  sub_l (n_l, x2_l, x3_l);

  xptr_l = MIN_L (x0_l, x1_l);
  xptr_l = MIN_L (xptr_l, x2_l);
  xptr_l = MIN_L (xptr_l, x3_l);
  cpy_l (x_l, xptr_l);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((11, sizeof (sign_u), &sign_u,
                    sizeof (sign_v), &sign_v,
                    sizeof (x0_l), x0_l,
                    sizeof (x1_l), x1_l,
                    sizeof (x2_l), x2_l,
                    sizeof (x3_l), x3_l,
                    sizeof (xp_l), xp_l,
                    sizeof (xq_l), xq_l,
                    sizeof (n_l), n_l,
                    sizeof (u_l), u_l,
                    sizeof (v_l), v_l));

  ISPURGED_L  ((11, sizeof (sign_u), &sign_u,
                    sizeof (sign_v), &sign_v,
                    sizeof (x0_l), x0_l,
                    sizeof (x1_l), x1_l,
                    sizeof (x2_l), x2_l,
                    sizeof (x3_l), x3_l,
                    sizeof (xp_l), xp_l,
                    sizeof (xq_l), xq_l,
                    sizeof (n_l), n_l,
                    sizeof (u_l), u_l,
                    sizeof (v_l), v_l));

  return E_CLINT_OK;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Ad-Hoc-Suche nach Primitivwurzel modulo p, p Primzahl > 2      */
/*  Syntax:    int                                                            */
/*             primroot_l(CLINT a_l, unsigned noofprimes, clint *primes_l[]); */
/*  Eingabe:   noofprimes (Anzahl der verschiedenen Primfaktoren pi der       */
/*                         Gruppenordnung p - 1)                              */
/*             primes_l (Vektor von CLINT-Werten, beginnend mit p - 1, gefolgt*/
/*                       von den Primfaktoren pi von p - 1 = p1^e1*...*pk^ek, */
/*                       noofprimes = k)                                      */
/*  Ausgabe:   a_l (Primitivwurzel modulo p)                                  */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             -1         : p - 1 = DIGITS_L (primes_l) ist ungerade =>       */
/*                          p ist keine Pz                                    */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
primroot_l (CLINT a_l, unsigned int noofprimes, clint *primes_l[])
{
  CLINT p_l, t_l, junk_l;
  ULONG i;

  if (ISODD_L (primes_l[0])) /* DIGITS_L (primes_l) = p - 1 (Gruppenordnung) */
    {
      return -1;
    }

  cpy_l (p_l, primes_l[0]);
  inc_l (p_l);                     /* p_l = p  (Modulus) */
  SETONE_L (a_l);

  do
    {
      inc_l (a_l);

      /* Teste, ob a_l ein Quadrat ist. Falls ja, kann a_l keine */
      /* Primitivwurzel sein. Erhoehe dann um 1:                 */
      if (issqr_l (a_l, t_l))
        {
          inc_l (a_l);
        }

      i = 1;

      do
        {
          div_l (primes_l[0], primes_l[i++], t_l, junk_l);  /* t_l <- n/pi */
          mexpkm_l (a_l, t_l, t_l, p_l);
        }

      while ((i <= noofprimes) && !EQONE_L (t_l));

    }
  while (EQONE_L (t_l));

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((4, sizeof (i), &i,
                   sizeof (p_l), p_l,
                   sizeof (t_l), t_l,
                   sizeof (junk_l), junk_l));

  ISPURGED_L  ((4, sizeof (i), &i,
                   sizeof (p_l), p_l,
                   sizeof (t_l), t_l,
                   sizeof (junk_l), junk_l));

  return E_CLINT_OK;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Loesung quadratischer Kongruenzensysteme                       */
/*             unter Verwendung des Chinesischen Restsatzes                   */
/*  Syntax:    chinrem_l (unsigned int noofeq, clint** koeff_l, CLINT x_l);   */
/*  Eingabe:   noofeq  (Anzahl der zu loesenden Gleichungen)                  */
/*             koeff_l (Vektor von 2*noofeq-vielen Zeigern auf                */
/*             CLINT-Argumente ai, mi paarweise teilerfremd (vgl. Kap. 10))   */
/*  Ausgabe:   x_l (Loesung des Gleichungssystems)                            */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             1          : noofeq ist 0                                      */
/*             2          : mi sind nicht paarweise teilerfremd               */
/*             E_CLINT_OFL: Ueberlauf                                         */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
chinrem_l (unsigned int noofeq, clint** koeff_l, CLINT x_l)
{
  clint *ai_l, *mi_l;

  CLINT g_l, u_l, v_l, m_l;
  unsigned int i;
  int sign_u, sign_v, err, error = E_CLINT_OK;
  if (0 == noofeq)
    {
      return 1;
    }

  cpy_l (x_l, *(koeff_l++));
  cpy_l (m_l, *(koeff_l++));

  for (i = 1; i < noofeq; i++)
    {
      ai_l = *(koeff_l++);
      mi_l = *(koeff_l++);

      xgcd_l (m_l, mi_l, g_l, u_l, &sign_u, v_l, &sign_v);

      if (!EQONE_L (g_l))
        {
          PURGEVARS_L ((4, sizeof (g_l), g_l,
                           sizeof (u_l), u_l,
                           sizeof (v_l), v_l,
                           sizeof (m_l), m_l));

          ISPURGED_L  ((4, sizeof (g_l), g_l,
                           sizeof (u_l), u_l,
                           sizeof (v_l), v_l,
                           sizeof (m_l), m_l));
          return 2;
        }

      err = mul_l (u_l, m_l, u_l);
      if (E_CLINT_OK == error)
        {
          error = err;
        }
      err = mul_l (u_l, ai_l, u_l);
      if (E_CLINT_OK == error)
        {
          error = err;
        }
      err = mul_l (v_l, mi_l, v_l);
      if (E_CLINT_OK == error)
        {
          error = err;
        }
      err = mul_l (v_l, x_l, v_l);
      if (E_CLINT_OK == error)
        {
          error = err;
        }

      sign_u = sadd (u_l, sign_u, v_l, sign_v, x_l);

      err = mul_l (m_l, mi_l, m_l);
      if (E_CLINT_OK == error)
        {
          error = err;
        }

      smod (x_l, sign_u, m_l, x_l);
    }

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((7, sizeof (i), &i,
                   sizeof (sign_u), &sign_u,
                   sizeof (sign_v), &sign_v,
                   sizeof (g_l), g_l,
                   sizeof (u_l), u_l,
                   sizeof (v_l), v_l,
                   sizeof (m_l), m_l));

  ISPURGED_L  ((7, sizeof (i), &i,
                   sizeof (sign_u), &sign_u,
                   sizeof (sign_v), &sign_v,
                   sizeof (g_l), g_l,
                   sizeof (u_l), u_l,
                   sizeof (v_l), v_l,
                   sizeof (m_l), m_l));

  return error;
}


/******************************************************************************/


static int tab2[] =
{0, 1, 0, -1, 0, -1, 0, 1};


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Legendre-/Jacobi-Symbol (a/b)                                  */
/*  Syntax:    int jacobi_l (CLINT aa_l, CLINT bb_l);                         */
/*  Eingabe:   aa_l, bb_l (Argumente)                                         */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: Wert des Jacobi-Symbols aa_l ueber bb_l                        */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
jacobi_l (CLINT aa_l, CLINT bb_l)
{
  CLINT a_l, b_l, tmp_l;
  long int k, v;

  /* Schritt 1 */
  if (EQZ_L (bb_l))
    {
      if (EQONE_L (aa_l))
        {
          return 1;
        }
      else
        {
          return 0;
        }
    }

  /* Schritt 2 */
  if (ISEVEN_L (aa_l) && ISEVEN_L (bb_l))
    {
      return 0;
    }

  cpy_l (a_l, aa_l);
  cpy_l (b_l, bb_l);

  v = twofact_l (b_l, b_l);
  if ((v & 1) == 0)
    {
      k = 1;
    }
  else
    {
      k = tab2[*LSDPTR_L (a_l) & 7];    /* *LSDPTR_L(a_l) & 7 == a_l % 8 */
    }

  /* Schritt 3 */
  while (GTZ_L (a_l))
    {
      v = twofact_l (a_l, a_l);
      if ((v & 1) != 0)
        {
          k *= tab2[*LSDPTR_L (b_l) & 7];
        }

      /* Schritt 4 */
      if (*LSDPTR_L (a_l) & *LSDPTR_L (b_l) & 2)
        {
          k = -k;
        }
      cpy_l (tmp_l, a_l);
      mod_l (b_l, tmp_l, a_l);
      cpy_l (b_l, tmp_l);
    }

  if (GT_L (b_l, one_l))
    {
      k = 0;
    }

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((4, sizeof (v), &v,
                   sizeof (a_l), a_l,
                   sizeof (b_l), b_l,
                   sizeof (tmp_l), tmp_l));

  ISPURGED_L  ((4, sizeof (v), &v,
                   sizeof (a_l), a_l,
                   sizeof (b_l), b_l,
                   sizeof (tmp_l), tmp_l));

  return (int)k;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Divisionssieb mit kleinen Primzahlen                           */
/*  Syntax:    USHORT sieve_l (CLINT a_l, unsigned int no_of_smallprimes);    */
/*  Eingabe:   a_l (Dividend), no_of_smallprimes (Anzahl kleiner Primzahlen)  */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: Primfaktor von a_l, falls einer gefunden wurde                 */
/*             1 falls a_l selbst Primzahl ist                                */
/*             0 falls kein Faktor gefunden wurde                             */
/*                                                                            */
/******************************************************************************/
USHORT __FLINT_API
sieve_l (CLINT a_l, unsigned int no_of_smallprimes)
{
  clint *aptr_l;
  USHORT bv, rv, qv;
  ULONG rdach;
  unsigned int i = 1;

  if (ISEVEN_L (a_l))
    {
      if (equ_l (a_l, two_l))
        {
          return 1;
        }
      else
        {
          return 2;
        }
    }

  no_of_smallprimes = MIN (no_of_smallprimes, NOOFSMALLPRIMES);
  bv = 2;
  do
    {
      rv = 0;
      bv += smallprimes[i];

      Assert (bv < 2000);

      for (aptr_l = MSDPTR_L (a_l); aptr_l >= LSDPTR_L (a_l); aptr_l--)
        {
          qv = (USHORT)((rdach = ((((ULONG)rv) << BITPERDGT) +
                                           (ULONG)*aptr_l)) / bv);
          rv = (USHORT)(rdach - (ULONG)bv * (ULONG)qv);
        }

    }
  while (rv != 0 && ++i <= no_of_smallprimes);

  if (0 == rv)
    {
      if (DIGITS_L (a_l) == 1 && *LSDPTR_L (a_l) == bv)
        {
          bv = 1;
        }
      /* else: Ergebnis ist bv */
    }
  else
    {
      bv = 0;
    }

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((4, sizeof (rdach), &rdach,
                   sizeof (i), &i,
                   sizeof (rv), &rv,
                   sizeof (qv), &qv));

  ISPURGED_L  ((4, sizeof (rdach), &rdach,
                   sizeof (i), &i,
                   sizeof (rv), &rv,
                   sizeof (qv), &qv));

  return bv;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Probabilistischer Primzahltest nach Miller-Rabin               */
/*  Syntax:    int prime_l (CLINT n_l, unsigned int no_of_smallprimes,        */
/*                                                 unsigned int iterations);  */
/*  Eingabe:   n_l (Testkandidat),                                            */
/*             no_of_smallprimes (Anzahl kleiner Primzahlen fuer das Sieb     */
/*             iterations (Anzahl von Runden fuer MR-Test. Falls              */
/*                         iterations == 0 wird die optimale Rundenzahl fuer  */
/*                         eine Irrtumswahrscheinlichkeit < 2^-80 verwendet.) */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: 1 falls n_l wahrscheinlich prim                                */
/*             0 falls n_l zusammengesetzt                                    */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
prime_l (CLINT n_l, unsigned int no_of_smallprimes, unsigned int iterations)
{
  CLINT d_l, x_l, q_l;
  USHORT i, j, k, p;
  int isprime = 1;

  if (EQONE_L (n_l))
    {
      return 0;
    }

  no_of_smallprimes = MIN (no_of_smallprimes, NOOFSMALLPRIMES);
  k = sieve_l (n_l, no_of_smallprimes);
  if (1 == k)
    {
      return 1;
    }

  if (1 < k)
    {
      return 0;
    }
  else
    {
      /* Falls iterations = 0 vorgegeben bestimme optimale Rundenzahl */
      /* fuer eine Irrtumswahrscheinlichkeit < 2^-80 (vgl. Kap. 10.5) */
      if (0 == iterations)
        {
          k = ld_l (n_l);
          if      (k <   73) iterations = 37;
          else if (k <  105) iterations = 32;
          else if (k <  137) iterations = 25;
          else if (k <  197) iterations = 19;
          else if (k <  220) iterations = 15;
          else if (k <  235) iterations = 13;
          else if (k <  252) iterations = 12;
          else if (k <  273) iterations = 11;
          else if (k <  300) iterations = 10;
          else if (k <  332) iterations =  9;
          else if (k <  375) iterations =  8;
          else if (k <  433) iterations =  7;
          else if (k <  514) iterations =  6;
          else if (k <  638) iterations =  5;
          else if (k <  847) iterations =  4;
          else if (k < 1275) iterations =  3;
          else if (k < 2861) iterations =  2;
          else iterations = 1;
        }

      cpy_l (d_l, n_l);
      dec_l (d_l);
      k = (USHORT)twofact_l (d_l, q_l);
      p = 0; /* Beginne mit Basis a = 2 */
      i = 0;
      isprime = 1;

      do
        {
          p += smallprimes[i++];

#ifdef FLINT_ASM
          wmexp_l (p, q_l, x_l, n_l);
#else
          wmexpm_l (p, q_l, x_l, n_l);
#endif /* FLINT_ASM */

          if (!EQONE_L (x_l))
            {

              j = 0;

              while (!EQONE_L (x_l) && !equ_l (x_l, d_l) && ++j < k)
                {
                  msqr_l (x_l, x_l, n_l);
                }

              if (!equ_l (x_l, d_l))
                {
                  isprime = 0;
                }
            }
        }
      while ((--iterations > 0) && isprime);

      /* Ueberschreiben der Variablen */
      PURGEVARS_L ((7, sizeof (i), &i,
                       sizeof (j), &j,
                       sizeof (k), &k,
                       sizeof (p), &p,
                       sizeof (d_l), d_l,
                       sizeof (x_l), x_l,
                       sizeof (q_l), q_l));

      ISPURGED_L  ((7, sizeof (i), &i,
                       sizeof (j), &j,
                       sizeof (k), &k,
                       sizeof (p), &p,
                       sizeof (d_l), d_l,
                       sizeof (x_l), x_l,
                       sizeof (q_l), q_l));

      return isprime;
    }
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Zweianteil eines CLINT-Wertes                                  */
/*  Syntax:    int twofact_l (CLINT a_l, CLINT b_l);                          */
/*  Eingabe:   a_l (Argument)                                                 */
/*  Ausgabe:   b_l (ungerader Anteil von a_l)                                 */
/*  Rueckgabe: Logarithmus des Zweianteils von a_l                            */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
twofact_l (CLINT a_l, CLINT b_l)
{
  int k = 0;
  if (EQZ_L (a_l))
    {
      SETZERO_L (b_l);
      return 0;
    }

  cpy_l (b_l, a_l);
  while (ISEVEN_L (b_l))
    {
      shr_l (b_l);
      ++k;
    }

  return k;
}


/******************************************************************************/


static char ntable[16] =
{
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c',
  'd', 'e', 'f'};


#ifndef isxdigit
#define isxdigit(__c)    ((('0' <= (__c)) && ((__c) <= '9'))\
                          ?1\
                          :(('a' <= (__c)) && ((__c) <= 'f'))\
                          ?1\
                          :(('A' <= (__c)) && ((__c) <= 'F'))\
                          ?1\
                          :0)
#endif


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Darstellung eines CLINT-Wertes als Zeichenkette                */
/*  Syntax:    char *xclint2str_l (CLINT n_l, USHORT base, int showbase);     */
/*  Eingabe:   n_l (Darzustellendes Argument)                                 */
/*             base (Basis der Zieldarstellung)                               */
/*             showbase (==0: Kein Praefix;                                   */
/*                       !=0: Praefix 0b, 0 oder 0x fuer base 2, 8 oder 16)   */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: Zeiger auf die Zahldarstellung als Zeichenkette                */
/*                                                                            */
/******************************************************************************/
char * __FLINT_API
xclint2str_l (CLINT n_l, USHORT base, int showbase)
{
  CLINTD u_l, r_l;
  clint base_l[10];
  int i = 0;
  static char N[CLINTMAXBIT + 3];

  if (2U > base || base > 16U)
    {
      return (char *)NULL;
    }

  u2clint_l (base_l, base);

  cpy_l (u_l, n_l);
  do
    {
      (void)div_l (u_l, base_l, u_l, r_l);
      if (GTZ_L (r_l))
        {
          N[i++] = (char)ntable[*LSDPTR_L (r_l) & 0xff];
        }
      else
        {
          N[i++] = '0';
        }
    }
  while (GTZ_L (u_l));

  if (showbase)
    {
      switch (base)
        {
          case 2:
            N[i++] = 'b';
            N[i++] = '0';
            break;
          case 8:
            N[i++] = '0';
            break;
          case 16:
            N[i++] = 'x';
            N[i++] = '0';
            break;
        }                   /*lint !e744 default-Statement ist ueberfluessig  */
    }
  N[i] = '\0';

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((2, sizeof (u_l), u_l,
                   sizeof (r_l), r_l));

  ISPURGED_L  ((2, sizeof (u_l), u_l,
                   sizeof (r_l), r_l));

  return strrev_l (N);
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Konvertierung einer Zeichenkette in CLINT-Zahlformat           */
/*  Syntax:    int str2clint_l (CLINT n_l, char *str, USHORT b);              */
/*  Eingabe:   str (Zeiger auf Zeichenkette),                                 */
/*             base (Basis der Zahldarstellung in str)                        */
/*  Ausgabe:   n_l (CLINT-Objekt mit aus str konvertiertem Wert)              */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_BOR: Basis ungueltig                                   */
/*             E_CLINT_OFL: Ueberlauf                                         */
/*             E_CLINT_NPT: str ist NULL-Pointer                              */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
str2clint_l (CLINT n_l, char *str, USHORT base)
{
  clint base_l[10];
  USHORT n;
  int error = E_CLINT_OK;

  if (str == NULL)
    {
      return E_CLINT_NPT;
    }

  if (2 > base || base > 16)
    {
      return E_CLINT_BOR;          /* Fehler: Basis ungueltig */
    }

  u2clint_l (base_l, base);

  SETZERO_L (n_l);

  if (*str == '0')
    {
      if ((tolower_l(*(str+1)) == 'x') ||
          (tolower_l(*(str+1)) == 'b'))      /* Ignoriere evtl. Praefix */
        {
          ++str;
          ++str;
        }
    }

  while (isxdigit ((int)*str) || isspace ((int)*str))
    {
      if (!isspace ((int)*str))
        {
          n = (USHORT)tolower_l (*str);
          switch (n)
            {
              case 'a':
              case 'b':
              case 'c':
              case 'd':
              case 'e':
              case 'f':
                n -= (USHORT)('a' - 10);
                break;
              default:
                n -= (USHORT)'0';
            }

          if (n >= base)
            {
              error = E_CLINT_BOR;
              break;
            }

          if ((error = mul_l (n_l, base_l, n_l)) != E_CLINT_OK)
            {
              break;
            }
          if ((error = uadd_l (n_l, n, n_l)) != E_CLINT_OK)
            {
              break;
            }
        }
      ++str;
    }

  return error;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Darstellung eines CLINT-Wertes als Byte-Vektor (IEEE P1363)    */
/*  Syntax:    UCHAR * clint2byte_l (CLINT n_l, int *len);                    */
/*  Eingabe:   n_l (Darzustellendes Argument)                                 */
/*  Ausgabe:   len (Anzahl Bytes der Zahldarstellung als Byte-Vektor)         */
/*  Rueckgabe: Zeiger auf die Zahldarstellung als Byte-Vektor                 */
/*             Wertigkeit der Stellen aufsteigend von rechts nach links       */
/*             NULL, falls in len der NULL-Pointer uebergeben wurde           */
/*                                                                            */
/******************************************************************************/
UCHAR * __FLINT_API
clint2byte_l (CLINT n_l, int *len)
{
  CLINTD u_l, r_l;
  int i = 0, j;
  UCHAR help;
  static UCHAR bytes[CLINTMAXBYTE];

  if (len == NULL)
    {
      return NULL;
    }

  cpy_l (u_l, n_l);
  do
    {
      (void)udiv_l (u_l, 0x100, u_l, r_l);
      if (GTZ_L (r_l))
        {
          bytes[i++] = (UCHAR)*LSDPTR_L (r_l);
        }
      else
        {
          bytes[i++] = 0;
        }
    }
  while (GTZ_L (u_l));

  *len = i;

  for (i = 0, j = *len - 1; i < j; i++, j--)
    {
      help = bytes[i];
      bytes[i] = bytes[j];
      bytes[j] = help;
    }

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((5, sizeof (i), &i,
                   sizeof (j), &j,
                   sizeof (help), &help,
                   sizeof (u_l), u_l,
                   sizeof (r_l), r_l));

  ISPURGED_L  ((5, sizeof (i), &i,
                   sizeof (j), &j,
                   sizeof (help), &help,
                   sizeof (u_l), u_l,
                   sizeof (r_l), r_l));

  return bytes;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Konvertierung eines Byte-Vektors in CLINT-Zahlformat           */
/*             (IEEE P1363)                                                   */
/*  Syntax:    int byte2clint_l (CLINT n_l, UCHAR *bytestr, int len);         */
/*  Eingabe:   bytestr (Zeiger auf Vektor von UCHAR,                          */
/*                      Wertigkeit aufsteigend von rechts nach links)         */
/*             len  (Anzahl Bytes in bytestr)                                 */
/*  Ausgabe:   n_l (CLINT-Objekt mit aus bytestr konvertiertem Wert)          */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_OFL: Ueberlauf                                         */
/*             E_CLINT_NPT: bytestr ist NULL-Pointer                          */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
byte2clint_l (CLINT n_l, UCHAR *bytestr, int len)
{
  int error = E_CLINT_OK, i;

  if (bytestr == NULL)
    {
      return E_CLINT_NPT;
    }

  if ((unsigned)len > CLINTMAXBYTE)
    {
      return E_CLINT_OFL;
    }

  SETZERO_L (n_l);

  for (i = 0; i < len; i++, bytestr++)
    {
      if ((error = umul_l (n_l, 0x100, n_l)) != E_CLINT_OK)
        {
          break;
        }

      if ((error = uadd_l (n_l, *bytestr, n_l)) != E_CLINT_OK)
        {
          break;
        }
    }

  return error;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Konvertierung eines USHORT-Wertes in das CLINT-Zahlformat      */
/*  Syntax:    void u2clint_l (CLINT num_l, USHORT u);                        */
/*  Eingabe:   u (Zu konvertierender Wert)                                    */
/*  Ausgabe:   num_l (CLINT-Objekt mit Wert u)                                */
/*  Rueckgabe: -                                                              */
/*                                                                            */
/******************************************************************************/
void __FLINT_API
u2clint_l (CLINT num_l, USHORT u)
{
  *LSDPTR_L (num_l) = u;
  SETDIGITS_L (num_l, 1);
  RMLDZRS_L (num_l);
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Konvertierung eines ULONG-Wertes in das CLINT-Zahlformat       */
/*  Syntax:    void ul2clint_l (CLINT num_l, USHORT u);                       */
/*  Eingabe:   ul (Zu konvertierender Wert)                                   */
/*  Ausgabe:   num_l (CLINT-Objekt mit Wert ul)                               */
/*  Rueckgabe: -                                                              */
/*                                                                            */
/******************************************************************************/
void __FLINT_API
ul2clint_l (CLINT num_l, ULONG ul)
{
  *LSDPTR_L (num_l) = (USHORT)(ul & 0xffff);
  *(LSDPTR_L (num_l) + 1) = (USHORT)(ul >> 16);
  SETDIGITS_L (num_l, 2);
  RMLDZRS_L (num_l);
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Konvertierung eines CLINT-Wertes in eine Zeichenkette          */
/*             Darstellung zur Basis 16 (Hex-Darstellung)                     */
/*  Syntax:    char * fhexstr_l (CLINT n_l);                                  */
/*  Eingabe:   n_l (darzustellende CLINT-Zahl)                                */
/*  Ausgabe:   Zeiger auf die Darstellung von n_l                             */
/*             als Zeichenkette zur Basis 16                                  */
/*  Rueckgabe: -                                                              */
/*                                                                            */
/******************************************************************************/
char * __FLINT_API
fhexstr_l (CLINT n_l)
{
  return xclint2str_l (n_l, 16, 0);
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Konvertierung eines CLINT-Wertes in eine Zeichenkette          */
/*             Darstellung zur Basis 10 (Dezimal-Darstellung)                 */
/*  Syntax:    char * fdecstr_l (CLINT n_l);                                  */
/*  Eingabe:   n_l (darzustellende CLINT-Zahl)                                */
/*  Ausgabe:   Zeiger auf die Darstellung von n_l                             */
/*             als Zeichenkette zur Basis 10                                  */
/*  Rueckgabe: -                                                              */
/*                                                                            */
/******************************************************************************/
char * __FLINT_API
fdecstr_l (CLINT n_l)
{
  return xclint2str_l (n_l, 10, 0);
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Konvertierung eines CLINT-Wertes in eine Zeichenkette          */
/*             Darstellung zur Basis  8 (Octal-Darstellung)                   */
/*  Syntax:    char * foctstr_l (CLINT n_l);                                  */
/*  Eingabe:   n_l (darzustellende CLINT-Zahl)                                */
/*  Ausgabe:   Zeiger auf die Darstellung von n_l                             */
/*             als Zeichenkette zur Basis 8                                   */
/*  Rueckgabe: -                                                              */
/*                                                                            */
/******************************************************************************/
char * __FLINT_API
foctstr_l (CLINT n_l)
{
  return xclint2str_l (n_l, 8, 0);
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Konvertierung eines CLINT-Wertes in eine Zeichenkette          */
/*             Darstellung zur Basis  2 (Binaer-Darstellung)                  */
/*  Syntax:    char * fbinstr_l (CLINT n_l);                                  */
/*  Eingabe:   n_l (darzustellende CLINT-Zahl)                                */
/*  Ausgabe:   Zeiger auf die Darstellung von n_l                             */
/*             als Zeichenkette zur Basis 2                                   */
/*  Rueckgabe: -                                                              */
/*                                                                            */
/******************************************************************************/
char * __FLINT_API
fbinstr_l (CLINT n_l)
{
  return xclint2str_l (n_l, 2, 0);
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Ausgabe der Version der FLINT-Bibliothek                       */
/*  Syntax:    unsigned long version_l (void);                                */
/*  Eingabe:   -                                                              */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: Versionskennung, definiert in flint.h, im hoeherwertigen Wort  */
/*             Falls die Assembler-Unterstuetzung eingeschaltet ist, wird im  */
/*             hoeherwertigen Byte des niederwertigen Wortes der Wert 0x61    */
/*             (ASCII 'a') ausgegeben, ansonsten 0.                           */
/*             Falls flint.c im Sicherheits-Modus compiliert wurde, wird      */
/*             im niederwertigen Byte des niederwertigen Wortes der Wert      */
/*             0x73 (ASCII 's'), ansonsten 0.                                 */
/*                                                                            */
/******************************************************************************/
ULONG __FLINT_API
version_l (void)
{
  return (ULONG)((FLINT_VERSION << BITPERDGT) +
                 (_FLINT_ASM << 8) + _FLINT_SECMOD);
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Ausgabe der Version der FLINT-Bibliothek als String            */
/*  Syntax:    unsigned long verstr_l (void);                                 */
/*  Eingabe:   -                                                              */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: Zeiger auf String mit Versionskennung                          */
/*             Falls die Assembler-Unterstuetzung eingeschaltet ist, wird das */
/*             Zeichen "a" an die Versionskennung angehaengt.                 */
/*             Falls flint.c im Sicherheits-Modus compiliert wurde, wird      */
/*             das Zeichen "s" an die Versionskennung angehaengt (default).   */
/*                                                                            */
/******************************************************************************/
char * __FLINT_API
verstr_l (void)
{
  static char s[10];
  sprintf(s, "%d.%.2d", FLINT_VERMAJ, FLINT_VERMIN);

#ifdef FLINT_ASM
  strcat(s, "a");
#endif

#ifdef FLINT_SECURE
  strcat(s, "s");
#endif

  return (char *)s;
}


/******************************************************************************/
/* Register-Handling                                                          */
/* Protected Interface                                                        */

struct clint_registers
{
  unsigned int created;
  unsigned int noofregs;
  clint **reg_l;
};


/******************************************************************************/

static struct clint_registers registers =
{0, 0, 0};

static USHORT NoofRegs = NOOFREGS;

static int
allocate_reg_l (void)
{
  USHORT i, j;

  if ((registers.reg_l = (clint **)malloc (sizeof (clint *) * NoofRegs)) == NULL)
    {
      return E_CLINT_MAL;
    }

  for (i = 0; i < NoofRegs; i++)
    {
      if ((registers.reg_l[i] = (clint *)malloc (CLINTMAXBYTE)) == NULL)
        {
          for (j = 0; j < i; j++)
            {
              free (registers.reg_l[j]);
            }
          return E_CLINT_MAL;      /* Fehler: malloc */
        }
    }

  return E_CLINT_OK;
}


/******************************************************************************/


static void
destroy_reg_l (void)
{
  USHORT i;

  for (i = 0; i < registers.noofregs; i++)
    {
      memset (registers.reg_l[i], 0, CLINTMAXBYTE);
      free (registers.reg_l[i]);
      registers.reg_l[i] = NULL;
    }
  free (registers.reg_l);
}


/******************************************************************************/
/* Register-Handling Public Interface                                         */


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Bereitstellung eines Registers vom CLINT-Typ                   */
/*  Syntax:    clint * create_l (void);                                       */
/*  Eingabe:   -                                                              */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: Zeiger auf Register                                            */
/*                                                                            */
/******************************************************************************/
clint * __FLINT_API
create_l (void)
{
  return (clint *)malloc (CLINTMAXBYTE);
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Loeschen eines Registers                                       */
/*  Syntax:    void purge_l (CLINT reg_l);                                    */
/*  Eingabe:   reg_l (Register)                                               */
/*  Ausgabe:   reg_l erhaelt den Wert 0, alle Stellen werden ueberschrieben.  */
/*  Rueckgabe: -                                                              */
/*                                                                            */
/******************************************************************************/
void __FLINT_API
purge_l (CLINT reg_l)
{
  if (reg_l != NULL)
    {
      memset (reg_l, 0, CLINTMAXBYTE);
    }
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Loeschen einer doppel-langen CLINT-Variablen                   */
/*  Syntax:    void purged_l (CLINTD reg_l);                                  */
/*  Eingabe:   reg_l (Doppelt-lange CLINT-Variable)                           */
/*  Ausgabe:   reg_l erhaelt den Wert 0, alle Stellen werden ueberschrieben.  */
/*  Rueckgabe: -                                                              */
/*                                                                            */
/******************************************************************************/
void __FLINT_API
purged_l (CLINTD reg_l)
{
  if (reg_l != NULL)
    {
      memset (reg_l, 0, sizeof (CLINTD));
    }
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Loeschen einer vierfach-langen CLINT-Variablen                 */
/*  Syntax:    void purgeq_l (CLINTQ reg_l);                                  */
/*  Eingabe:   reg_l (Vierfach-lange CLINT-Variable)                          */
/*  Ausgabe:   reg_l erhaelt den Wert 0, alle Stellen werden ueberschrieben.  */
/*  Rueckgabe: -                                                              */
/*                                                                            */
/******************************************************************************/
void __FLINT_API
purgeq_l (CLINTQ reg_l)
{
  if (reg_l != NULL)
    {
      memset (reg_l, 0, sizeof (CLINTQ));
    }
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Loeschen und Rueckgabe eines mit create_l() erzeugten Registers*/
/*  Syntax:    void free_l (CLINT reg_l);                                     */
/*  Eingabe:   reg_l (Register)                                               */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: -                                                              */
/*                                                                            */
/******************************************************************************/
void __FLINT_API
free_l (CLINT reg_l)
{
  if (reg_l != NULL)
    {
      memset (reg_l, 0, CLINTMAXBYTE);
      free (reg_l);
    }
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Setzen der Register-Zahl                                       */
/*  Syntax:    void set_noofregs_l (unsigned int nregs);                      */
/*  Eingabe:   nregs (Anzahl der Register in der Registerbank)                */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: -                                                              */
/*                                                                            */
/******************************************************************************/
void __FLINT_API
set_noofregs_l (unsigned int nregs)
{
  NoofRegs = (USHORT)nregs;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Erzeugung eines Registersatzes, Inkrement einer Semaphore      */
/*  Syntax:    int create_reg_l (void);                                       */
/*  Eingabe:   -                                                              */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_MAL: Fehler bei malloc()                               */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
create_reg_l (void)
{
  int error = E_CLINT_OK;

  if (registers.created == 0)
    {
      error = allocate_reg_l ();
      registers.noofregs = NoofRegs;
    }

  if (!error)
    {
      ++registers.created;
    }

  return error;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Ausgabe eines Zeigers auf ein Register des Registersatzes      */
/*  Syntax:    clint * get_reg_l (unsigned int reg);                          */
/*  Eingabe:   reg (Registernummer)                                           */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: Zeiger auf das angegebene Register falls existent, sonst NULL  */
/*                                                                            */
/******************************************************************************/
clint * __FLINT_API
get_reg_l (unsigned int reg)
{
  if (!registers.created || (reg >= registers.noofregs))
    {
      return (clint *)NULL;
    }

  return registers.reg_l[reg];
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Loeschen eines Registers des Registersatzes                    */
/*  Syntax:    int purge_reg_l (unsigned int reg);                            */
/*  Eingabe:   reg (Registernummer)                                           */
/*  Ausgabe:   Das Register wird zu Null gesetzt, alle Stellen ueberschrieben */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_NOR: Registersatz nicht allokiert.                     */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
purge_reg_l (unsigned int reg)
{
  if (!registers.created || (reg >= registers.noofregs))
    {
      return E_CLINT_NOR;
    }

  memset (registers.reg_l[reg], 0, CLINTMAXBYTE);
  return E_CLINT_OK;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Loeschen des Registersatzes                                    */
/*  Syntax:    int purgeall_reg_l (void);                                     */
/*  Eingabe:   -                                                              */
/*  Ausgabe:   Alle Register erhalten den Wert 0, alle Stellen werden         */
/*             ueberschrieben.                                                */
/*  Rueckgabe: E_CLINT_OK : Alles O.K.                                        */
/*             E_CLINT_NOR: Registersatz nicht allokiert                      */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
purgeall_reg_l (void)
{
  USHORT i;

  if (registers.created)
    {
      for (i = 0; i < registers.noofregs; i++)
        {
          memset (registers.reg_l[i], 0, CLINTMAXBYTE);
        }
      return E_CLINT_OK;
    }

  return E_CLINT_NOR;              /* Fehler: Register nicht allokiert */
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Loeschen und Entfernen des Registersatzes, falls Semaphore == 1*/
/*  Syntax:    void free_reg_l (void);                                        */
/*  Eingabe:   -                                                              */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: -                                                              */
/*                                                                            */
/******************************************************************************/
void __FLINT_API
free_reg_l (void)
{
  if (registers.created == 1)
    {
      destroy_reg_l ();
    }

  if (registers.created)
    {
      --registers.created;
    }
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Pruefung auf gueltiges CLINT-Zahlformat                        */
/*  Syntax:    int vcheck_l (CLINT n_l);                                      */
/*  Eingabe:   n_l (zu pruefendes CLINT-Objekt)                               */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: E_VCHECK_OK : Alles O.K.                                       */
/*             E_VCHECK_MEM: n_l ist NULL-Pointer                             */
/*             E_VCHECK_OFL: Wert von n_l ist zu gross                        */
/*             E_VCHECK_LDZ: n_l hat fuehrende Nullen                         */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
vcheck_l (CLINT n_l)
{
  int error = E_VCHECK_OK;
  if (n_l == NULL)
    {
      error = E_VCHECK_MEM;             /* Speicherfehler */
    }
  else
    {
      if (((unsigned int)DIGITS_L (n_l)) > CLINTMAXDIGIT)
        {
          error = E_VCHECK_OFL;         /* Echter Ueberlauf */
        }
      else
        {
          if ((DIGITS_L (n_l) > 0) && (n_l[DIGITS_L (n_l)] == 0))
            {
              error = E_VCHECK_LDZ;     /* Fuehrende Nullen */
            }
        }
    }

  return error;
}


/******************************************************************************/


static clint SEED64[10];
static clint A64[] =
{0x0004, 0x7f2d, 0x4c95, 0xf42d, 0x5851};

static clint BUFF64[100];


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Erzeugung einer 64-Bit-Pseudozufallszahl                       */
/*  Syntax:    clint * rand64_l (void);                                       */
/*  Eingabe:   -                                                              */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: 64-Bit Pseudo-Zufallszahl vom Typ CLINT                        */
/*                                                                            */
/******************************************************************************/
clint * __FLINT_API
rand64_l (void)
{
  mul_l (SEED64, A64, SEED64);
  inc_l (SEED64);

  SEED64[0] = MIN (SEED64[0], 4);  /* Reduktion modulo 2^64 */
  return ((clint *)SEED64);
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Erzeugung einer Pseudozufallszahl vom Typ UCHAR                */
/*  Syntax:    UCHAR ucrand64_l (void);                                       */
/*  Eingabe:   -                                                              */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: Pseudo-Zufallszahl vom Typ unsigned char                       */
/*                                                                            */
/******************************************************************************/
UCHAR __FLINT_API
ucrand64_l (void)
{
  rand64_l();
  return (UCHAR)(SEED64[SEED64[0]] >> (BITPERDGT - 8));
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Erzeugung einer Pseudozufallszahl vom Typ USHORT               */
/*  Syntax:    USHORT usrand64_l (void);                                      */
/*  Eingabe:   -                                                              */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: Pseudo-Zufallszahl vom Typ unsigned short                      */
/*                                                                            */
/******************************************************************************/
USHORT __FLINT_API
usrand64_l (void)
{
  rand64_l();
  return SEED64[SEED64[0]];
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Erzeugung einer Pseudozufallszahl vom Typ ULONG                */
/*  Syntax:    ULONG ulrand64_l (void);                                       */
/*  Eingabe:   -                                                              */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: Pseudo-Zufallszahl vom Typ unsigned long                       */
/*                                                                            */
/******************************************************************************/
ULONG __FLINT_API
ulrand64_l (void)
{
  ULONG val;
  USHORT l;
  rand64_l();

  l = SEED64[0];
  switch (l)
    {
      case 4:
      case 3:
      case 2:
        val = (ULONG)SEED64[l-1];
        val += ((ULONG)SEED64[l] << BITPERDGT);
        break;
      case 1:
        val = (ULONG)SEED64[l];
        break;
      default:
        val = 0;
    }

  return val;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Initialisierung des 64-Bit-Pseudozufallszahlengenerators       */
/*  Syntax:    clint * seed64_l (CLINT seed_l);                               */
/*  Eingabe:   seed_l (Startwert)                                             */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: Zeiger auf den internen Puffer BUFF64, der den vorherigen      */
/*             Folgenwert beinhaltet                                          */
/*                                                                            */
/******************************************************************************/
clint * __FLINT_API
seed64_l (CLINT seed_l)
{
  int i;
  cpy_l (BUFF64, SEED64);
  for (i = 0; i <= MIN (DIGITS_L (seed_l), 4); i++)
    {
      SEED64[i] = seed_l[i];
    }

  return BUFF64;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Initialisierung des 64-Bit-Pseudozufallszahlengenerators       */
/*             mit ULONG-Typ                                                  */
/*  Syntax:    clint * seed64_l (ULONG seed_l);                               */
/*  Eingabe:   seed_l (Startwert)                                             */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: Zeiger auf den internen Puffer BUFF64, der den vorherigen      */
/*             Folgenwert beinhaltet                                          */
/*                                                                            */
/******************************************************************************/
clint * __FLINT_API
ulseed64_l (ULONG seed)
{
  cpy_l (BUFF64, SEED64);
  ul2clint_l (SEED64, seed);

  return BUFF64;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Erzeugung einer Pseudo-Zufallszahl vom CLINT-Typ               */
/*             (Vorherige Initialisierung durch Aufruf von seed64_l())        */
/*  Syntax:    void rand_l (CLINT a_l, int l);                                */
/*  Eingabe:   l (Anzahl der Binarstellen der zu erzeugenden Zufallszahl)     */
/*  Ausgabe:   a_l (Pseudo-Zufallszahl)                                       */
/*  Rueckgabe: -                                                              */
/*                                                                            */
/******************************************************************************/
void __FLINT_API
rand_l (CLINT a_l, int l)
{
  USHORT i, j;
  USHORT ls, lr;

  l = MIN ((unsigned int)l, CLINTMAXBIT);
  ls = (USHORT)l >> LDBITPERDGT;
  lr = (USHORT)l & ((USHORT)BITPERDGT - 1);

  for (i = 1; i <= ls; i++)
    {
      a_l[i] = usrand64_l ();
    }

  if (lr > 0)
    {
      ++ls;
      a_l[ls] = usrand64_l ();
      j = 1U << (lr - 1);                         /* j <- 2^(lr - 1) */
      a_l[ls] = (a_l[ls] | j) & ((j << 1) - 1);   /* Bit lr auf 1, hoehere Bits auf 0 */
    }
  else
    {
      a_l[ls] |= BASEDIV2;
    }

  SETDIGITS_L (a_l, ls);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((4, sizeof (i), &i,
                   sizeof (j), &j,
                   sizeof (ls), &ls,
                   sizeof (lr), &lr));

  ISPURGED_L  ((4, sizeof (i), &i,
                   sizeof (j), &j,
                   sizeof (ls), &ls,
                   sizeof (lr), &lr));
}


/******************************************************************************/


static CLINT XBBS;
static CLINT MODBBS;

#if (CLINTMAXDIGIT >= 128)
static const char *MODBBSSTR =
"81aa5c97200fb2504c08b92e2f8b7e0805775c72b139b455846028653793ba9d616cd01cef719"
"5b2f952e046ab62d063b048c8f62b21c7fc7fa4e71eef2588aa59b81f355b5539a471ee483b02"
"2bdab25b3fb41bc6224d60c17bbcb091294f76cb64c3b6da5504085a958b679d1f1147158189d"
"4fa76ab721c535a3ecfe4452cc61b9466e315ba4b4392db04f686dbb61084b21c9540f972718f"
"c96ed25a40a917a07c668048683ec116219621a301610bfba40e1ab11bd1e13aa8476aa6d37b2"
"61228df85c7df67043c51b37a250b27401aaf837101d2db1a55572dd7a79646ff6e5d20a24e4b"
"43c6d8ab5e9a77becd76d5f0355252f4318e2066d3f9c42f25";
#elif (CLINTMAXDIGIT >= 64)
static const char *MODBBSSTR =
"a1c0a7edba2a2aee2cb3947c3d1c0468ee5a5791ec3ebb97238bd4c3bdad1a00280f0a7518d56"
"523003d5cee48a60d606d78b818d81b0ef963555b9b62fc3b5f796815946ed28987596f84ccc1"
"7f87b9ca5959fc9763bc43521aa467cdcec60cd9fa7548268169750adf746df899cc64b059b7c"
"194ab4ba492c04c3a6c630103";
#elif (CLINTMAXDIGIT >= 48)
static const char *MODBBSSTR =
"c58d49cd9529aed21da56db12844522c04ace305362219478a99da74751213f8ccdfb52fb7a8b"
"fc2d5ce18c86c9e447f78b9013071d2fbb4be506f942cc45793e752733c71b07f40c3e54a9bdc"
"3d9bb18a2c9411e8f898b28d060ea0dc9b309b";
#elif (CLINTMAXDIGIT >= 32)
static const char *MODBBSSTR =
"a3d46604762377bccd0ab8562b46132740b75feb0e3ca7a79022736c6a5ca0b17a03465222af1"
"a074e31224ea01fc48b3150579c06ef8f073673a5169e8ea021";
#elif (CLINTMAXDIGIT >= 16)
static const char *MODBBSSTR =
"b2c31d33668afb5600be97e13b769fe4f558fc96bc46b8d174d94fb468ff31a5";
#elif (CLINTMAXDIGIT >= 8)
static const char *MODBBSSTR =
"845196304e498ea78ff06d51bd58c9e3";
#endif


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Erzeugung eines Zufallsbit nach Blum-Blum-Shub                 */
/*  Syntax:    int randbit_l (void);                                          */
/*  Eingabe:   -                                                              */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: Zufallsbit 0 oder 1                                            */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
randbit_l (void)
{
  msqr_l (XBBS, XBBS, MODBBS);

  /* Gebe niedrigstwertiges Bit von XBBS aus */
  return (*LSDPTR_L (XBBS) & 1);
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Erzeugung einer Pseudozufallszahl vom Typ UCHAR                */
/*  Syntax:    UCHAR ucrandBBS_l (void);                                      */
/*  Eingabe:   -                                                              */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: Pseudo-Zufallszahl vom Typ unsigned char                       */
/*                                                                            */
/******************************************************************************/
UCHAR __FLINT_API
ucrandBBS_l (void)
{
  UCHAR i, r = 0;
  for (i = 0; i < (UCHAR)(sizeof (UCHAR) << 3); i++)
    {
      r = (r << 1) + randbit_l();
    }

  return r;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Erzeugung einer Pseudozufallszahl vom Typ USHORT               */
/*  Syntax:    USHORT usrandBBS_l (void);                                     */
/*  Eingabe:   -                                                              */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: Pseudo-Zufallszahl vom Typ unsigned short                      */
/*                                                                            */
/******************************************************************************/
USHORT __FLINT_API
usrandBBS_l (void)
{
  USHORT i, r = 0;
  for (i = 0; i < (USHORT)(sizeof (USHORT) << 3); i++)
    {
      r = (r << 1) + randbit_l();
    }

  return r;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Erzeugung einer Pseudozufallszahl vom Typ ULONG                */
/*  Syntax:    ULONG ulrandBBS_l (void);                                      */
/*  Eingabe:   -                                                              */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: Pseudo-Zufallszahl vom Typ unsigned long                       */
/*                                                                            */
/******************************************************************************/
ULONG __FLINT_API
ulrandBBS_l (void)
{
  ULONG i, r = 0;
  for (i = 0; i < (ULONG)(sizeof (ULONG) << 3); i++)
    {
      r = (r << 1) + randbit_l();
    }

  return r;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Initialisierung des BLUM-BLUM-SHUB-Pseudozufallszahlen-        */
/*             Generators                                                     */
/*  Syntax:    int seedBBS_l (CLINT seed_l);                                  */
/*  Eingabe:   seed_l (Startwert)                                             */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: E_CLINT_OK falls alles O.K.                                    */
/*             -1: Startwert und Modulus sind nicht teilerfremd               */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
seedBBS_l (CLINT seed_l)
{
  CLINT g_l;

  str2clint_l (MODBBS, (char*)MODBBSSTR, 16);
  gcd_l (MODBBS, seed_l, g_l);

  if (!EQONE_L (g_l))
    {
      return -1;
    }

  msqr_l (seed_l, XBBS, MODBBS);
  return E_CLINT_OK;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Initialisierung des BBS-Pseudozufallszahlengenerators          */
/*             mit ULONG-Typ                                                  */
/*  Syntax:    int ulseedBBS_l (ULONG seed);                                  */
/*  Eingabe:   seed (Startwert)                                               */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: -                                                              */
/*                                                                            */
/******************************************************************************/
void __FLINT_API
ulseedBBS_l (ULONG seed)
{
  CLINT seed_l;
  ul2clint_l (seed_l, seed);
  seedBBS_l (seed_l);
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Erzeugung einer Zufallszahl vom CLINT-Typ                      */
/*             unter Verwendung des BLUM-BLUM-SHUB-Zufallszahlengenerators    */
/*             (Vorherige Initialisierung durch Aufruf von seedBBS_l())       */
/*  Syntax:    void randBBS_l (CLINT a_l, int l);                             */
/*  Eingabe:   l (Anzahl von Binaerstellen der zu erzeugenden Zufallszahl)    */
/*  Ausgabe:   a_l (Pseudo-Zufallszahl)                                       */
/*  Rueckgabe: -                                                              */
/*                                                                            */
/******************************************************************************/
void __FLINT_API
randBBS_l (CLINT a_l, int l)
{
  USHORT i, j;
  USHORT ls, lr;

  l = (int)MIN ((unsigned int)l, CLINTMAXBIT);
  ls = (USHORT)l >> LDBITPERDGT;
  lr = (USHORT)l & ((USHORT)BITPERDGT - 1);

  for (i = 1; i <= ls; i++)
    {
      a_l[i] = usrandBBS_l ();
    }

  if (lr > 0)
    {
      ++ls;
      a_l[ls] = usrandBBS_l ();
      j = 1U << (lr - 1);                         /* j <- 2^(lr - 1) */
      a_l[ls] = (a_l[ls] | j) & ((j << 1) - 1);   /* Bit lr auf 1, hoehere Bits auf 0 */
    }
  else
    {
      a_l[ls] |= BASEDIV2;
    }

  SETDIGITS_L (a_l, ls);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((4, sizeof (i), &i,
                   sizeof (j), &j,
                   sizeof (ls), &ls,
                   sizeof (lr), &lr));

  ISPURGED_L  ((4, sizeof (i), &i,
                   sizeof (j), &j,
                   sizeof (ls), &ls,
                   sizeof (lr), &lr));
}


/******************************************************************************/



/******************************************************************************
 * Private arithmetische Kernfunktionen                                       *
 ******************************************************************************/


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Addition-Kernfunktion  ohne Kontrolle auf Ueberlauf            */
/*             Ohne Kontrolle fuehrender Nullen                               */
/*  Syntax:    void add (CLINT a_l, CLINT b_l, CLINT s_l);                    */
/*  Eingabe:   a_l, b_l (Summanden)                                           */
/*  Ausgabe:   s_l (Summe)                                                    */
/*  Rueckgabe: -                                                              */
/*                                                                            */
/******************************************************************************/
void __FLINT_API
add (CLINT a_l, CLINT b_l, CLINT s_l)   /* Keine Overflow-Behandlung          */
{                                       /* Keine Kontrolle auf fuehrende Null */
  clint *msdptra_l, *msdptrb_l;
  clint *aptr_l, *bptr_l, *sptr_l = LSDPTR_L (s_l);
  ULONG carry = 0L;

  if (DIGITS_L (a_l) < DIGITS_L (b_l))
    {
      aptr_l = LSDPTR_L (b_l);
      bptr_l = LSDPTR_L (a_l);
      msdptra_l = MSDPTR_L (b_l);
      msdptrb_l = MSDPTR_L (a_l);
      SETDIGITS_L (s_l, DIGITS_L (b_l));
    }
  else
    {
      aptr_l = LSDPTR_L (a_l);
      bptr_l = LSDPTR_L (b_l);
      msdptra_l = MSDPTR_L (a_l);
      msdptrb_l = MSDPTR_L (b_l);
      SETDIGITS_L (s_l, DIGITS_L (a_l));
    }

  while (bptr_l <= msdptrb_l)
    {
      *sptr_l++ = (USHORT)(carry = (ULONG)*aptr_l++ + (ULONG)*bptr_l++
                                   + (ULONG)(USHORT)(carry >> BITPERDGT));
    }
  while (aptr_l <= msdptra_l)
    {
      *sptr_l++ = (USHORT)(carry = (ULONG)*aptr_l++
               + (ULONG)(USHORT)(carry >> BITPERDGT));
    }
  if (carry & BASE)
    {
      *sptr_l = 1;
      INCDIGITS_L (s_l);
    }

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((1, sizeof (carry), &carry));
  ISPURGED_L  ((1, sizeof (carry), &carry));
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Subtraktion-Kernfunktion ohne Kontrolle auf Unterlauf          */
/*             Ohne Kontrolle fuehrender Nullen                               */
/*  Syntax:    void sub (CLINT a_l, CLINT b_l, CLINT d_l);                    */
/*  Eingabe:   a_l (Minuend), b_l (Subtrahend)                                */
/*  Ausgabe:   d_l (Differenz)                                                */
/*  Rueckgabe: -                                                              */
/*                                                                            */
/******************************************************************************/
void __FLINT_API
sub (CLINT a_l, CLINT b_l, CLINT d_l)   /* Keine Kontrolle auf fuehrende Null */
{
  clint *msdptra_l, *msdptrb_l;
  clint *aptr_l = LSDPTR_L (a_l), *bptr_l = LSDPTR_L (b_l), *dptr_l = LSDPTR_L (d_l);
  ULONG carry = 0L;

  msdptra_l = MSDPTR_L (a_l);
  msdptrb_l = MSDPTR_L (b_l);

  SETDIGITS_L (d_l, DIGITS_L (a_l));

  while (bptr_l <= msdptrb_l)
    {
      *dptr_l++ = (USHORT)(carry = (ULONG)*aptr_l++ - (ULONG)*bptr_l++
                                         - ((carry & BASE) >> BITPERDGT));
    }

  while (aptr_l <= msdptra_l)
    {
      *dptr_l++ = (USHORT)(carry = (ULONG)*aptr_l++
                     - ((carry & BASE) >> BITPERDGT));
    }

  RMLDZRS_L (d_l);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((1, sizeof (carry), &carry));
  ISPURGED_L  ((1, sizeof (carry), &carry));
}


#if !defined FLINT_ASM
/******************************************************************************/
/*                                                                            */
/*  Funktion:  Multiplikation-Kernfunktion ohne Kontrolle auf Ueberlauf       */
/*             Ohne Kontrolle fuehrender Nullen, ohne Akku-Betrieb            */
/*  Syntax:    void mult (CLINT aa_l, CLINT bb_l, CLINT p_l);                 */
/*  Eingabe:   aa_l, bb_l (Faktoren)                                          */
/*  Ausgabe:   p_l (Produkt)                                                  */
/*  Rueckgabe: -                                                              */
/*                                                                            */
/******************************************************************************/
void __FLINT_API
mult (CLINT aa_l, CLINT bb_l, CLINT p_l) /* Doppelt langes Ergebnis moeglich  */
{                                        /* Keine Kontrolle auf fuehrende Null*/
  register clint *cptr_l, *bptr_l;       /* Kein Akku-Betrieb                 */
  clint *a_l, *b_l, *aptr_l, *csptr_l, *msdptra_l, *msdptrb_l;
  USHORT av;
  ULONG carry;

  if (EQZ_L (aa_l) || EQZ_L (bb_l))
    {
      SETZERO_L (p_l);
      return;
    }

  if (DIGITS_L (aa_l) < DIGITS_L (bb_l))
    {
      a_l = bb_l;
      b_l = aa_l;
    }
  else
    {
      a_l = aa_l;
      b_l = bb_l;
    }

  msdptra_l = MSDPTR_L (a_l);
  msdptrb_l = MSDPTR_L (b_l);

  carry = 0;
  av = *LSDPTR_L (a_l);
  for (bptr_l = LSDPTR_L (b_l), cptr_l = LSDPTR_L (p_l); bptr_l <= msdptrb_l; bptr_l++, cptr_l++)
    {
      *cptr_l = (USHORT)(carry = (ULONG)av * (ULONG)*bptr_l +
                           (ULONG)(USHORT)(carry >> BITPERDGT));
    }
  *cptr_l = (USHORT)(carry >> BITPERDGT);

  for (csptr_l = LSDPTR_L (p_l) + 1, aptr_l = LSDPTR_L (a_l) + 1; aptr_l <= msdptra_l; csptr_l++, aptr_l++)
    {
      carry = 0;
      av = *aptr_l;
      for (bptr_l = LSDPTR_L (b_l), cptr_l = csptr_l; bptr_l <= msdptrb_l; bptr_l++, cptr_l++)
        {
          *cptr_l = (USHORT)(carry = (ULONG)av * (ULONG)*bptr_l +
              (ULONG)*cptr_l + (ULONG)(USHORT)(carry >> BITPERDGT));
        }
      *cptr_l = (USHORT)(carry >> BITPERDGT);
    }

  SETDIGITS_L (p_l, DIGITS_L (a_l) + DIGITS_L (b_l));
  RMLDZRS_L (p_l);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((2, sizeof (carry), &carry,
                   sizeof (av), &av));

  ISPURGED_L  ((2, sizeof (carry), &carry,
                   sizeof (av), &av));
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Multiplikation von CLINT mit USHORT-Typ ohne Ueberlauf-        */
/*             Kontrolle, ohne Kontrolle fuehrender Nullen, ohne Akku-Betrieb */
/*  Syntax:    void umul (CLINT a_l, USHORT b, CLINT p_l);                    */
/*  Eingabe:   a_l, b (Faktoren)                                              */
/*  Ausgabe:   p_l (Produkt)                                                  */
/*  Rueckgabe: -                                                              */
/*                                                                            */
/******************************************************************************/
void __FLINT_API
umul (CLINT a_l, USHORT b, CLINT p_l)   /* Doppelt langes Ergebnis moeglich   */
{                                       /* Keine Kontrolle auf fuehrende Null */
  register clint *aptr_l, *cptr_l;      /* Kein Akku-Betrieb                  */
  clint *msdptra_l;
  ULONG carry;

  if (EQZ_L (a_l) || 0 == b)
    {
      SETZERO_L (p_l);
      return;
    }

  msdptra_l = MSDPTR_L (a_l);

  carry = 0;
  for (aptr_l = LSDPTR_L (a_l), cptr_l = LSDPTR_L (p_l); aptr_l <= msdptra_l; aptr_l++, cptr_l++)
    {
      *cptr_l = (USHORT)(carry = (ULONG)b * (ULONG)*aptr_l +
                          (ULONG)(USHORT)(carry >> BITPERDGT));
    }
  *cptr_l = (USHORT)(carry >> BITPERDGT);

  SETDIGITS_L (p_l, DIGITS_L (a_l) + 1);
  RMLDZRS_L (p_l);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((1, sizeof (carry), &carry));
  ISPURGED_L  ((1, sizeof (carry), &carry));
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Quadrierung-Kernfunktion ohne Kontrolle auf Ueberlauf          */
/*             Ohne Kontrolle fuehrender Nullen, ohne Akku-Betrieb            */
/*  Syntax:    void sqr (CLINT a_l, CLINT r_l);                               */
/*  Eingabe:   a_l (Faktor)                                                   */
/*  Ausgabe:   p_l (Quadrat)                                                  */
/*  Rueckgabe: -                                                              */
/*                                                                            */
/******************************************************************************/
void __FLINT_API
sqr (CLINT a_l, CLINT p_l)              /* Doppelt langes Ergebnis moeglich   */
{                                       /* Keine Kontrolle auf fuehrende Null */
  register clint *cptr_l, *bptr_l;      /* Kein Akku-Betrieb                  */
  clint *aptr_l, *csptr_l, *msdptra_l, *msdptrb_l, *msdptrc_l;
  USHORT av;
  ULONG carry;

  if (EQZ_L (a_l))
    {
      SETZERO_L (p_l);
      return;
    }

  msdptrb_l = MSDPTR_L (a_l);
  msdptra_l = msdptrb_l - 1;
  *LSDPTR_L (p_l) = 0;
  carry = 0;
  av = *LSDPTR_L (a_l);
  for (bptr_l = LSDPTR_L (a_l) + 1, cptr_l = LSDPTR_L (p_l) + 1; bptr_l <= msdptrb_l; bptr_l++, cptr_l++)
    {
      *cptr_l = (USHORT)(carry = (ULONG)av * (ULONG)*bptr_l +
                           (ULONG)(USHORT)(carry >> BITPERDGT));
    }
  *cptr_l = (USHORT)(carry >> BITPERDGT);

  for (aptr_l = LSDPTR_L (a_l) + 1, csptr_l = LSDPTR_L (p_l) + 3; aptr_l <= msdptra_l; aptr_l++, csptr_l += 2)
    {
      carry = 0;
      av = *aptr_l;
      for (bptr_l = aptr_l + 1, cptr_l = csptr_l; bptr_l <= msdptrb_l; bptr_l++, cptr_l++)
        {
          *cptr_l = (USHORT)(carry = (ULONG)av * (ULONG)*bptr_l +
              (ULONG)*cptr_l + (ULONG)(USHORT)(carry >> BITPERDGT));
        }
      *cptr_l = (USHORT)(carry >> BITPERDGT);
    }

  msdptrc_l = cptr_l;
  carry = 0;
  for (cptr_l = LSDPTR_L (p_l); cptr_l <= msdptrc_l; cptr_l++)
    {
      *cptr_l = (USHORT)(carry = (((ULONG)*cptr_l) << 1) +
                        (ULONG)(USHORT)(carry >> BITPERDGT));
    }
  *cptr_l = (USHORT)(carry >> BITPERDGT);

  carry = 0;
  for (bptr_l = LSDPTR_L (a_l), cptr_l = LSDPTR_L (p_l); bptr_l <= msdptrb_l; bptr_l++, cptr_l++)
    {
      *cptr_l = (USHORT)(carry = (ULONG)*bptr_l * (ULONG)*bptr_l +
                (ULONG)*cptr_l + (ULONG)(USHORT)(carry >> BITPERDGT));
      cptr_l++;
      *cptr_l = (USHORT)(carry = (ULONG)*cptr_l + (carry >> BITPERDGT));
    }

  SETDIGITS_L (p_l, DIGITS_L (a_l) << 1);
  RMLDZRS_L (p_l);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((2, sizeof (carry), &carry,
                   sizeof (av), &av));

  ISPURGED_L  ((2, sizeof (carry), &carry,
                   sizeof (av), &av));
}

#endif /* FLINT_ASM */


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Addition mit Vorzeichen                                        */
/*  Syntax:    int sadd (CLINT a_l, int sign_a, CLINT b_l, int sign_b,        */
/*                                                               CLINT c_l);  */
/*  Eingabe:   a_l (Summand), sign_a (Vorzeichen a_l), b_l (Summand),         */
/*             sign_b (Vorzeichen b_l)                                        */
/*  Ausgabe:   c_l (Summe)                                                    */
/*  Rueckgabe: Vorzeichen der Summe c_l                                       */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
sadd (CLINT a_l, int sign_a, CLINT b_l, int sign_b, CLINT c_l)
{
  int sign_c;

  Assert (sign_a == 1 || sign_a == -1);
  Assert (sign_b == 1 || sign_b == -1);

  if (1 == sign_a)
    {
      if (1 == sign_b)             /* a + b */
        {
          add (a_l, b_l, c_l);
          sign_c = 1;
        }
      else                         /* -1 == sign_b, a - b */
        {
          if (LT_L (a_l, b_l))
            {
              sub (b_l, a_l, c_l);
              sign_c = -1;
            }
          else
            {
              sub (a_l, b_l, c_l);
              sign_c = 1;
            }
        }
    }
  else                             /* -1 == sign_a */
    {
      if (1 == sign_b)             /* b - a */
        {
          if (LT_L (b_l, a_l))
            {
              sub (a_l, b_l, c_l);
              sign_c = -1;
            }
          else
            {
              sub (b_l, a_l, c_l);
              sign_c = 1;
            }
        }
      else                         /* -1 == sign_b, -(a + b) */
        {
          add (a_l, b_l, c_l);
          sign_c = -1;
        }
    }

  return sign_c;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Subtraktion mit Vorzeichen                                     */
/*  Syntax:    int ssub (CLINT a_l, int sign_a, CLINT b_l, int sign_b,        */
/*                                                                CLINT c_l); */
/*  Eingabe:   a_l (Minuend), sign_a (Vorzeichen a_l), b_l (Subtrahend),      */
/*             sign_b (Vorzeichen b_l)                                        */
/*  Ausgabe:   c_l (Differenz)                                                */
/*  Rueckgabe: Vorzeichen der Differenz c_l                                   */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
ssub (CLINT a_l, int sign_a, CLINT b_l, int sign_b, CLINT c_l)
{
  int sign_c;

  Assert (sign_a == 1 || sign_a == -1);
  Assert (sign_b == 1 || sign_b == -1);

  if (1 == sign_a)
    {
      if (1 == sign_b)             /* a - b */
        {
          if (LT_L (a_l, b_l))
            {
              sub (b_l, a_l, c_l);
              sign_c = -1;
            }
          else
            {
              sub (a_l, b_l, c_l);
              sign_c = 1;
            }
        }
      else                         /* -1 == sign_b, a + b */
        {
          add (a_l, b_l, c_l);
          sign_c = 1;
        }
    }
  else                             /* -1 == sign_a */
    {
      if (1 == sign_b)             /* -(a + b) */
        {
          add (a_l, b_l, c_l);
          sign_c = -1;
        }
      else                         /* -1 == sign_b, b - a) */
        {
          if (LT_L (b_l, a_l))
            {
              sub (a_l, b_l, c_l);
              sign_c = -1;
            }
          else
            {
              sub (b_l, a_l, c_l);
              sign_c = 1;
            }
        }
    }

  return sign_c;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Restbildung a mod b, a mit Vorzeichen                          */
/*  Syntax:    int smod (CLINT a_l, int sign_a, CLINT b_l, CLINT c_l);        */
/*  Eingabe:   a_l (Dividend), sign_a (Vorzeichen a_l), b_l (Divisor)         */
/*  Ausgabe:   c_l (Rest)                                                     */
/*  Rueckgabe: Vorzeichen Rest = 1                                            */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
smod (CLINT a_l, int sign_a, CLINT b_l, CLINT c_l)
{
  CLINT q_l, r_l;

  if (EQZ_L (b_l))
    {
      return E_CLINT_DBZ;
    }

  div_l (a_l, b_l, q_l, r_l);

  if ((-1 == sign_a) && GTZ_L (r_l))
    {
      sub_l (b_l, r_l, r_l);
    }

  cpy_l (c_l, r_l);

  /* Ueberschreiben der Variablen */
  PURGEVARS_L ((2, sizeof (q_l), q_l,
                   sizeof (r_l), r_l));

  ISPURGED_L  ((2, sizeof (q_l), q_l,
                   sizeof (r_l), r_l));

  return 1;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Sieb des Erathostenes                                          */
/*  Syntax:    ULONG * genprimes (ULONG N);                                   */
/*  Eingabe:   N (Obere Grenze fuer Primzahlerzeugung)                        */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: Zeiger auf Vektor von ULONG-Werten mit Primzahlen <= N         */
/*             Der Wert des Vektors an der Stelle 0 enthaelt die Anzahl der   */
/*             gefundenen Primzahlen                                          */
/*             NULL: Fehler bei der Allokierung von Speicher                  */
/*                                                                            */
/******************************************************************************/
ULONG * __FLINT_API
genprimes (ULONG N)
{
  ULONG i, k, p, s, L, B, count;
  char *f;
  ULONG *primes;

  B = (1 + ul_iroot (N)) >> 1;
  L = N >> 1;
  if (((N&1) == 0) && (N > 0))
    {
      --L;
    }

  if ((f = (char *)malloc ((size_t)L+1)) == NULL)
    {
      return (ULONG *)NULL;
    }

  for (i = 1; i <= L; i++)
    {
      f[i] = 1;
    }

  p = 3;
  s = 4;
  for (i = 1; i <= B; i++)
    {
      if (f[i])
        {
          for (k = s; k <= L; k += p)
            {
              f[k] = 0;
            }
        }
      s += p + p + 2;
      p += 2;
    }

  for (count = i = 1; i <= L; i++)
    {
      count += f[i];
    }

  if ((primes = (ULONG*)malloc ((size_t)(count+1) * sizeof (ULONG))) == NULL)
    {
      free (f);
      return (ULONG *)NULL;
    }

  for (count = i = 1; i <= L; i++)
    {
      if (f[i])
        {
          ++count;
          primes[count] = (i << 1) + 1;      /*lint !e796 !e797 */
        }
    }

  if (N < 2)
    {
      primes[0] = 0;
    }
  else
    {
      primes[0] = count;
      primes[1] = 2;                         /*lint !e796 */
    }
  free (f);

  return primes;
}



/******************************************************************************/
/*                                                                            */
/*                              Hilfsfunktionen                               */
/*                                                                            */
/******************************************************************************/


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Ganzzahliger Anteil der Quadratwurzel einer ULONG-Zahl         */
/*  Syntax:    ULONG ul_iroot (ULONG n);                                      */
/*  Eingabe:   n                                                              */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: Ganzzahliger Anteil der Quadratwurzel von n                    */
/*                                                                            */
/******************************************************************************/
static ULONG
ul_iroot (ULONG n)
{
  ULONG x = 1, y = 0;
  if (0 == n)
    {
      return 0;
    }

  while (y <= n)
    {
      x = x << 1;
      y = x * x;
    }
  do
    {
      y = x;
      x = (x * x + n) / (2 * x);
    }
  while (x < y);

  y = 0;
  return x;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Umkehrung der Reihenfolge einer Zeichenkette                   */
/*  Syntax:    char *strrev_l (char *str);                                    */
/*  Eingabe:   str (Zeiger auf Zeichenkette)                                  */
/*  Ausgabe:   str (Umgekehrte Zeichenkette)                                  */
/*  Rueckgabe: Zeiger auf umgekehrte Zeichenkette                             */
/*                                                                            */
/******************************************************************************/
char * __FLINT_API
strrev_l (char *str)
{
  char help;
  char *anfang = str;
  char *ende = str + strlen (str) - 1;  /* '\0' bleibt am Platz */

  for (; ende > anfang; ende--, anfang++)
    {
      help = *anfang;
      *anfang = *ende;
      *ende = help;
    }

  return str;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Sichere tolower-Funktion                                       */
/*  Syntax:    int tolower_l (int c);                                         */
/*  Eingabe:   c (ASCII-Zeichen)                                              */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: Lowercase-Zeichen, falls c Uppercase ist                       */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
tolower_l (int c)
{
  if (isupper (c))
    {
      return tolower (c);
    }
  else
    {
      return c;
    }
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Sichere toupper-Funktion                                       */
/*  Syntax:    int toupper_l (int c);                                         */
/*  Eingabe:   c (ASCII-Zeichen)                                              */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: Uppercase-Zeichen, falls c Lowercase ist                       */
/*                                                                            */
/******************************************************************************/
int __FLINT_API
toupper_l (int c)
{
  if (islower (c))
    {
      return toupper (c);
    }
  else
    {
      return c;
    }
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Konvertierung einer Zeichenkette in Kleinbuchstaben            */
/*  Syntax:    int strupr_l (char *str);                                      */
/*  Eingabe:   str (Zeiger auf Null-terminierte Zeichenkette)                 */
/*  Ausgabe:   Konvertierte Zeichenkette                                      */
/*  Rueckgabe: Zeiger str auf konvertierte Zeichenkette                       */
/*                                                                            */
/******************************************************************************/
char * __FLINT_API
strlwr_l (char *str)
{
  unsigned i;

  for (i = 0 ; i < (unsigned)strlen (str); i++)
    {
      str[i] = (char)tolower_l (str[i]);
    }

  return str;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Konvertierung einer Zeichenkette in Grossbuchstaben            */
/*  Syntax:    int strupr_l (char *str);                                      */
/*  Eingabe:   str (Zeiger auf Null-terminierte Zeichenkette)                 */
/*  Ausgabe:   Konvertierte Zeichenkette                                      */
/*  Rueckgabe: Zeiger str auf konvertierte Zeichenkette                       */
/*                                                                            */
/******************************************************************************/
char * __FLINT_API
strupr_l (char *str)
{
  unsigned i;
  for (i = 0 ; i < (unsigned)strlen (str) ; i++)
    {
      str[i] = (char)toupper_l (str[i]);
    }
  return str;
}


/******************************************************************************/
/*                                                                            */
/*  Funktion:  Konvertierung eines ULONG-Wertes in eine Zeichenkette          */
/*  Syntax:    char *ultoa_l (char *str, ULONG ul, int base);                 */
/*  Eingabe:   str (Puffer zur Aufnahme der Zeichenkette                      */
/*             ul (Darzustellender ULONG-Wert)                                */
/*             base (Basis der Zahldarstellung, 2 <= base <= 16)              */
/*  Ausgabe:   Null-terminierte Zeichenkette mit Zahldarstellung von ul       */
/*             zur Basis base                                                 */
/*  Rueckgabe: Zeiger str auf Zeichenkette                                    */
/*             NULL, falls base < 2 oder base > 16                            */
/*                                                                            */
/******************************************************************************/
char * __FLINT_API
ultoa_l (char *str, ULONG ul, int base)
{
  int i = 0, j;

  if ((base < 2) || (base > 16))
    {
      return (char *)NULL;
    }

  do
    {
      j = ul % (unsigned)(base + '0');
      if (j > (int)'9')
        {
          j += (int)('a' - '9') - 1;
        }
      str[i++] = j;
    }
  while ((ul /= (unsigned)base) != 0);
  str[i] = '\0';
  return (strrev_l (str));
}


#ifdef FLINT_SECURE
#include <stdarg.h>
/******************************************************************************/
/*                                                                            */
/*  Funktion:  Loeschen von Variablen                                         */
/*             Diese Funktion wird verwendet, um Variablen durch              */
/*             Ueberschreiben zu loeschen, auch bei eingeschalteter           */
/*             Optimierung des Compilers, die typischerweise Zuweisungen an   */
/*             lokale Variablen oder Aufrufe von memset() fuer automatische   */
/*             CLINT-Objekte am Ende einer Funktion ignoriert.                */
/*  Syntax:    static void purgevars_l (int noofvars, ...);                   */
/*  Eingabe:   noofvars (Anzahl der folgenden Paare von Argumenten)           */
/*             ... (noofvars-viele Paare von (sizeof (var), (typ *)var))      */
/*  Ausgabe:   Zu Null gesetzte Variablen *var                                */
/*  Rueckgabe: -                                                              */
/*                                                                            */
/******************************************************************************/
static void purgevars_l (int noofvars, ...)
{
  va_list ap;
  size_t size;
  va_start (ap, noofvars);
  for (; noofvars > 0; --noofvars)
    {
      switch (size = va_arg (ap, size_t))
        {
          case 1:  *va_arg (ap, char *) = 0;
                   break;
          case 2:  *va_arg (ap, short *) = 0;
                   break;
          case 4:  *va_arg (ap, long *) = 0;
                   break;
          default: Assert (size >= CLINTMAXBYTE);
                   memset (va_arg(ap, char *), 0, size);
        }
    }
  va_end (ap);
}

#ifdef FLINT_DEBUG
/******************************************************************************/
/*                                                                            */
/*  Funktion:  Pruefung, ob Speicherbereiche mit 0 belegt sind                */
/*  Syntax:    static int ispurged_l (int noofvars, ...);                     */
/*  Eingabe:   noofvars (Anzahl der folgenden Paare von Argumenten)           */
/*             ... (noofvars-viele Paare von (sizeof (var), (typ *)var))      */
/*  Ausgabe:   -                                                              */
/*  Rueckgabe: 1 falls die angegebenen Speicherbereiche mit 0 belegt sind     */
/*             0 sonst                                                        */
/*                                                                            */
/******************************************************************************/
static int ispurged_l (int noofvars, ...)
{
  va_list ap;
  size_t size;
  char *cptr;
  va_start (ap, noofvars);
  for (; noofvars > 0; --noofvars)
    {
      size = va_arg (ap, size_t);
      cptr = va_arg(ap, char *);
      for (; size > 0; size--)
        {
          if (0 != *cptr++) return 0;
        }
    }
  va_end (ap);
  return 1;
}
#endif /* FLINT_DEBUG */
#endif /* FLINT_SECURE */

/******************************************************************************/

/* Differenzen der ersten 6542 Primzahlen                                     */
/* 2, 2 + 1 = 3, 3 + 2 = 5, 5 + 2 = 7, 7 + 4 = 11 usw. bis 65519 + 2 = 65521  */

USHORT __FLINT_API_DATA smallprimes[NOOFSMALLPRIMES] =
{   2, 1, 2, 2, 4, 2, 4, 2, 4, 6, 2, 6, 4, 2, 4, 6, 6, 2, 6, 4, 2,
    6, 4, 6, 8, 4, 2, 4, 2, 4, 14, 4, 6, 2, 10, 2, 6, 6, 4, 6, 6,
    2, 10, 2, 4, 2, 12, 12, 4, 2, 4, 6, 2, 10, 6, 6, 6, 2, 6, 4, 2,
    10, 14, 4, 2, 4, 14, 6, 10, 2, 4, 6, 8, 6, 6, 4, 6, 8, 4, 8, 10,
    2, 10, 2, 6, 4, 6, 8, 4, 2, 4, 12, 8, 4, 8, 4, 6, 12, 2, 18, 6,
    10, 6, 6, 2, 6, 10, 6, 6, 2, 6, 6, 4, 2, 12, 10, 2, 4, 6, 6, 2,
    12, 4, 6, 8, 10, 8, 10, 8, 6, 6, 4, 8, 6, 4, 8, 4, 14, 10, 12, 2,
    10, 2, 4, 2, 10, 14, 4, 2, 4, 14, 4, 2, 4, 20, 4, 8, 10, 8, 4, 6,
    6, 14, 4, 6, 6, 8, 6, 12, 4, 6, 2, 10, 2, 6, 10, 2, 10, 2, 6, 18,
    4, 2, 4, 6, 6, 8, 6, 6, 22, 2, 10, 8, 10, 6, 6, 8, 12, 4, 6, 6,
    2, 6, 12, 10, 18, 2, 4, 6, 2, 6, 4, 2, 4, 12, 2, 6, 34, 6, 6, 8,
    18, 10, 14, 4, 2, 4, 6, 8, 4, 2, 6, 12, 10, 2, 4, 2, 4, 6, 12, 12,
    8, 12, 6, 4, 6, 8, 4, 8, 4, 14, 4, 6, 2, 4, 6, 2, 6, 10, 20, 6,
    4, 2, 24, 4, 2, 10, 12, 2, 10, 8, 6, 6, 6, 18, 6, 4, 2, 12, 10, 12,
    8, 16, 14, 6, 4, 2, 4, 2, 10, 12, 6, 6, 18, 2, 16, 2, 22, 6, 8, 6,
    4, 2, 4, 8, 6, 10, 2, 10, 14, 10, 6, 12, 2, 4, 2, 10, 12, 2, 16, 2,
    6, 4, 2, 10, 8, 18, 24, 4, 6, 8, 16, 2, 4, 8, 16, 2, 4, 8, 6, 6,
    4, 12, 2, 22, 6, 2, 6, 4, 6, 14, 6, 4, 2, 6, 4, 6, 12, 6, 6, 14,
    4, 6, 12, 8, 6, 4, 26, 18, 10, 8, 4, 6, 2, 6, 22, 12, 2, 16, 8, 4,
    12, 14, 10, 2, 4, 8, 6, 6, 4, 2, 4, 6, 8, 4, 2, 6, 10, 2, 10, 8,
    4, 14, 10, 12, 2, 6, 4, 2, 16, 14, 4, 6, 8, 6, 4, 18, 8, 10, 6, 6,
    8, 10, 12, 14, 4, 6, 6, 2, 28, 2, 10, 8, 4, 14, 4, 8, 12, 6, 12, 4,
    6, 20, 10, 2, 16, 26, 4, 2, 12, 6, 4, 12, 6, 8, 4, 8, 22, 2, 4, 2,
    12, 28, 2, 6, 6, 6, 4, 6, 2, 12, 4, 12, 2, 10, 2, 16, 2, 16, 6, 20,
    16, 8, 4, 2, 4, 2, 22, 8, 12, 6, 10, 2, 4, 6, 2, 6, 10, 2, 12, 10,
    2, 10, 14, 6, 4, 6, 8, 6, 6, 16, 12, 2, 4, 14, 6, 4, 8, 10, 8, 6,
    6, 22, 6, 2, 10, 14, 4, 6, 18, 2, 10, 14, 4, 2, 10, 14, 4, 8, 18, 4,
    6, 2, 4, 6, 2, 12, 4, 20, 22, 12, 2, 4, 6, 6, 2, 6, 22, 2, 6, 16,
    6, 12, 2, 6, 12, 16, 2, 4, 6, 14, 4, 2, 18, 24, 10, 6, 2, 10, 2, 10,
    2, 10, 6, 2, 10, 2, 10, 6, 8, 30, 10, 2, 10, 8, 6, 10, 18, 6, 12, 12,
    2, 18, 6, 4, 6, 6, 18, 2, 10, 14, 6, 4, 2, 4, 24, 2, 12, 6, 16, 8,
    6, 6, 18, 16, 2, 4, 6, 2, 6, 6, 10, 6, 12, 12, 18, 2, 6, 4, 18, 8,
    24, 4, 2, 4, 6, 2, 12, 4, 14, 30, 10, 6, 12, 14, 6, 10, 12, 2, 4, 6,
    8, 6, 10, 2, 4, 14, 6, 6, 4, 6, 2, 10, 2, 16, 12, 8, 18, 4, 6, 12,
    2, 6, 6, 6, 28, 6, 14, 4, 8, 10, 8, 12, 18, 4, 2, 4, 24, 12, 6, 2,
    16, 6, 6, 14, 10, 14, 4, 30, 6, 6, 6, 8, 6, 4, 2, 12, 6, 4, 2, 6,
    22, 6, 2, 4, 18, 2, 4, 12, 2, 6, 4, 26, 6, 6, 4, 8, 10, 32, 16, 2,
    6, 4, 2, 4, 2, 10, 14, 6, 4, 8, 10, 6, 20, 4, 2, 6, 30, 4, 8, 10,
    6, 6, 8, 6, 12, 4, 6, 2, 6, 4, 6, 2, 10, 2, 16, 6, 20, 4, 12, 14,
    28, 6, 20, 4, 18, 8, 6, 4, 6, 14, 6, 6, 10, 2, 10, 12, 8, 10, 2, 10,
    8, 12, 10, 24, 2, 4, 8, 6, 4, 8, 18, 10, 6, 6, 2, 6, 10, 12, 2, 10,
    6, 6, 6, 8, 6, 10, 6, 2, 6, 6, 6, 10, 8, 24, 6, 22, 2, 18, 4, 8,
    10, 30, 8, 18, 4, 2, 10, 6, 2, 6, 4, 18, 8, 12, 18, 16, 6, 2, 12, 6,
    10, 2, 10, 2, 6, 10, 14, 4, 24, 2, 16, 2, 10, 2, 10, 20, 4, 2, 4, 8,
    16, 6, 6, 2, 12, 16, 8, 4, 6, 30, 2, 10, 2, 6, 4, 6, 6, 8, 6, 4,
    12, 6, 8, 12, 4, 14, 12, 10, 24, 6, 12, 6, 2, 22, 8, 18, 10, 6, 14, 4,
    2, 6, 10, 8, 6, 4, 6, 30, 14, 10, 2, 12, 10, 2, 16, 2, 18, 24, 18, 6,
    16, 18, 6, 2, 18, 4, 6, 2, 10, 8, 10, 6, 6, 8, 4, 6, 2, 10, 2, 12,
    4, 6, 6, 2, 12, 4, 14, 18, 4, 6, 20, 4, 8, 6, 4, 8, 4, 14, 6, 4,
    14, 12, 4, 2, 30, 4, 24, 6, 6, 12, 12, 14, 6, 4, 2, 4, 18, 6, 12, 8,
    6, 4, 12, 2, 12, 30, 16, 2, 6, 22, 14, 6, 10, 12, 6, 2, 4, 8, 10, 6,
    6, 24, 14, 6, 4, 8, 12, 18, 10, 2, 10, 2, 4, 6, 20, 6, 4, 14, 4, 2,
    4, 14, 6, 12, 24, 10, 6, 8, 10, 2, 30, 4, 6, 2, 12, 4, 14, 6, 34, 12,
    8, 6, 10, 2, 4, 20, 10, 8, 16, 2, 10, 14, 4, 2, 12, 6, 16, 6, 8, 4,
    8, 4, 6, 8, 6, 6, 12, 6, 4, 6, 6, 8, 18, 4, 20, 4, 12, 2, 10, 6,
    2, 10, 12, 2, 4, 20, 6, 30, 6, 4, 8, 10, 12, 6, 2, 28, 2, 6, 4, 2,
    16, 12, 2, 6, 10, 8, 24, 12, 6, 18, 6, 4, 14, 6, 4, 12, 8, 6, 12, 4,
    6, 12, 6, 12, 2, 16, 20, 4, 2, 10, 18, 8, 4, 14, 4, 2, 6, 22, 6, 14,
    6, 6, 10, 6, 2, 10, 2, 4, 2, 22, 2, 4, 6, 6, 12, 6, 14, 10, 12, 6,
    8, 4, 36, 14, 12, 6, 4, 6, 2, 12, 6, 12, 16, 2, 10, 8, 22, 2, 12, 6,
    4, 6, 18, 2, 12, 6, 4, 12, 8, 6, 12, 4, 6, 12, 6, 2, 12, 12, 4, 14,
    6, 16, 6, 2, 10, 8, 18, 6, 34, 2, 28, 2, 22, 6, 2, 10, 12, 2, 6, 4,
    8, 22, 6, 2, 10, 8, 4, 6, 8, 4, 12, 18, 12, 20, 4, 6, 6, 8, 4, 2,
    16, 12, 2, 10, 8, 10, 2, 4, 6, 14, 12, 22, 8, 28, 2, 4, 20, 4, 2, 4,
    14, 10, 12, 2, 12, 16, 2, 28, 8, 22, 8, 4, 6, 6, 14, 4, 8, 12, 6, 6,
    4, 20, 4, 18, 2, 12, 6, 4, 6, 14, 18, 10, 8, 10, 32, 6, 10, 6, 6, 2,
    6, 16, 6, 2, 12, 6, 28, 2, 10, 8, 16, 6, 8, 6, 10, 24, 20, 10, 2, 10,
    2, 12, 4, 6, 20, 4, 2, 12, 18, 10, 2, 10, 2, 4, 20, 16, 26, 4, 8, 6,
    4, 12, 6, 8, 12, 12, 6, 4, 8, 22, 2, 16, 14, 10, 6, 12, 12, 14, 6, 4,
    20, 4, 12, 6, 2, 6, 6, 16, 8, 22, 2, 28, 8, 6, 4, 20, 4, 12, 24, 20,
    4, 8, 10, 2, 16, 2, 12, 12, 34, 2, 4, 6, 12, 6, 6, 8, 6, 4, 2, 6,
    24, 4, 20, 10, 6, 6, 14, 4, 6, 6, 2, 12, 6, 10, 2, 10, 6, 20, 4, 26,
    4, 2, 6, 22, 2, 24, 4, 6, 2, 4, 6, 24, 6, 8, 4, 2, 34, 6, 8, 16,
    12, 2, 10, 2, 10, 6, 8, 4, 8, 12, 22, 6, 14, 4, 26, 4, 2, 12, 10, 8,
    4, 8, 12, 4, 14, 6, 16, 6, 8, 4, 6, 6, 8, 6, 10, 12, 2, 6, 6, 16,
    8, 6, 6, 12, 10, 2, 6, 18, 4, 6, 6, 6, 12, 18, 8, 6, 10, 8, 18, 4,
    14, 6, 18, 10, 8, 10, 12, 2, 6, 12, 12, 36, 4, 6, 8, 4, 6, 2, 4, 18,
    12, 6, 8, 6, 6, 4, 18, 2, 4, 2, 24, 4, 6, 6, 14, 30, 6, 4, 6, 12,
    6, 20, 4, 8, 4, 8, 6, 6, 4, 30, 2, 10, 12, 8, 10, 8, 24, 6, 12, 4,
    14, 4, 6, 2, 28, 14, 16, 2, 12, 6, 4, 20, 10, 6, 6, 6, 8, 10, 12, 14,
    10, 14, 16, 14, 10, 14, 6, 16, 6, 8, 6, 16, 20, 10, 2, 6, 4, 2, 4, 12,
    2, 10, 2, 6, 22, 6, 2, 4, 18, 8, 10, 8, 22, 2, 10, 18, 14, 4, 2, 4,
    18, 2, 4, 6, 8, 10, 2, 30, 4, 30, 2, 10, 2, 18, 4, 18, 6, 14, 10, 2,
    4, 20, 36, 6, 4, 6, 14, 4, 20, 10, 14, 22, 6, 2, 30, 12, 10, 18, 2, 4,
    14, 6, 22, 18, 2, 12, 6, 4, 8, 4, 8, 6, 10, 2, 12, 18, 10, 14, 16, 14,
    4, 6, 6, 2, 6, 4, 2, 28, 2, 28, 6, 2, 4, 6, 14, 4, 12, 14, 16, 14,
    4, 6, 8, 6, 4, 6, 6, 6, 8, 4, 8, 4, 14, 16, 8, 6, 4, 12, 8, 16,
    2, 10, 8, 4, 6, 26, 6, 10, 8, 4, 6, 12, 14, 30, 4, 14, 22, 8, 12, 4,
    6, 8, 10, 6, 14, 10, 6, 2, 10, 12, 12, 14, 6, 6, 18, 10, 6, 8, 18, 4,
    6, 2, 6, 10, 2, 10, 8, 6, 6, 10, 2, 18, 10, 2, 12, 4, 6, 8, 10, 12,
    14, 12, 4, 8, 10, 6, 6, 20, 4, 14, 16, 14, 10, 8, 10, 12, 2, 18, 6, 12,
    10, 12, 2, 4, 2, 12, 6, 4, 8, 4, 44, 4, 2, 4, 2, 10, 12, 6, 6, 14,
    4, 6, 6, 6, 8, 6, 36, 18, 4, 6, 2, 12, 6, 6, 6, 4, 14, 22, 12, 2,
    18, 10, 6, 26, 24, 4, 2, 4, 2, 4, 14, 4, 6, 6, 8, 16, 12, 2, 42, 4,
    2, 4, 24, 6, 6, 2, 18, 4, 14, 6, 28, 18, 14, 6, 10, 12, 2, 6, 12, 30,
    6, 4, 6, 6, 14, 4, 2, 24, 4, 6, 6, 26, 10, 18, 6, 8, 6, 6, 30, 4,
    12, 12, 2, 16, 2, 6, 4, 12, 18, 2, 6, 4, 26, 12, 6, 12, 4, 24, 24, 12,
    6, 2, 12, 28, 8, 4, 6, 12, 2, 18, 6, 4, 6, 6, 20, 16, 2, 6, 6, 18,
    10, 6, 2, 4, 8, 6, 6, 24, 16, 6, 8, 10, 6, 14, 22, 8, 16, 6, 2, 12,
    4, 2, 22, 8, 18, 34, 2, 6, 18, 4, 6, 6, 8, 10, 8, 18, 6, 4, 2, 4,
    8, 16, 2, 12, 12, 6, 18, 4, 6, 6, 6, 2, 6, 12, 10, 20, 12, 18, 4, 6,
    2, 16, 2, 10, 14, 4, 30, 2, 10, 12, 2, 24, 6, 16, 8, 10, 2, 12, 22, 6,
    2, 16, 20, 10, 2, 12, 12, 18, 10, 12, 6, 2, 10, 2, 6, 10, 18, 2, 12, 6,
    4, 6, 2, 24, 28, 2, 4, 2, 10, 2, 16, 12, 8, 22, 2, 6, 4, 2, 10, 6,
    20, 12, 10, 8, 12, 6, 6, 6, 4, 18, 2, 4, 12, 18, 2, 12, 6, 4, 2, 16,
    12, 12, 14, 4, 8, 18, 4, 12, 14, 6, 6, 4, 8, 6, 4, 20, 12, 10, 14, 4,
    2, 16, 2, 12, 30, 4, 6, 24, 20, 24, 10, 8, 12, 10, 12, 6, 12, 12, 6, 8,
    16, 14, 6, 4, 6, 36, 20, 10, 30, 12, 2, 4, 2, 28, 12, 14, 6, 22, 8, 4,
    18, 6, 14, 18, 4, 6, 2, 6, 34, 18, 2, 16, 6, 18, 2, 24, 4, 2, 6, 12,
    6, 12, 10, 8, 6, 16, 12, 8, 10, 14, 40, 6, 2, 6, 4, 12, 14, 4, 2, 4,
    2, 4, 8, 6, 10, 6, 6, 2, 6, 6, 6, 12, 6, 24, 10, 2, 10, 6, 12, 6,
    6, 14, 6, 6, 52, 20, 6, 10, 2, 10, 8, 10, 12, 12, 2, 6, 4, 14, 16, 8,
    12, 6, 22, 2, 10, 8, 6, 22, 2, 22, 6, 8, 10, 12, 12, 2, 10, 6, 12, 2,
    4, 14, 10, 2, 6, 18, 4, 12, 8, 18, 12, 6, 6, 4, 6, 6, 14, 4, 2, 12,
    12, 4, 6, 18, 18, 12, 2, 16, 12, 8, 18, 10, 26, 4, 6, 8, 6, 6, 4, 2,
    10, 20, 4, 6, 8, 4, 20, 10, 2, 34, 2, 4, 24, 2, 12, 12, 10, 6, 2, 12,
    30, 6, 12, 16, 12, 2, 22, 18, 12, 14, 10, 2, 12, 12, 4, 2, 4, 6, 12, 2,
    16, 18, 2, 40, 8, 16, 6, 8, 10, 2, 4, 18, 8, 10, 8, 12, 4, 18, 2, 18,
    10, 2, 4, 2, 4, 8, 28, 2, 6, 22, 12, 6, 14, 18, 4, 6, 8, 6, 6, 10,
    8, 4, 2, 18, 10, 6, 20, 22, 8, 6, 30, 4, 2, 4, 18, 6, 30, 2, 4, 8,
    6, 4, 6, 12, 14, 34, 14, 6, 4, 2, 6, 4, 14, 4, 2, 6, 28, 2, 4, 6,
    8, 10, 2, 10, 2, 10, 2, 4, 30, 2, 12, 12, 10, 18, 12, 14, 10, 2, 12, 6,
    10, 6, 14, 12, 4, 14, 4, 18, 2, 10, 8, 4, 8, 10, 12, 18, 18, 8, 6, 18,
    16, 14, 6, 6, 10, 14, 4, 6, 2, 12, 12, 4, 6, 6, 12, 2, 16, 2, 12, 6,
    4, 14, 6, 4, 2, 12, 18, 4, 36, 18, 12, 12, 2, 4, 2, 4, 8, 12, 4, 36,
    6, 18, 2, 12, 10, 6, 12, 24, 8, 6, 6, 16, 12, 2, 18, 10, 20, 10, 2, 6,
    18, 4, 2, 40, 6, 2, 16, 2, 4, 8, 18, 10, 12, 6, 2, 10, 8, 4, 6, 12,
    2, 10, 18, 8, 6, 4, 20, 4, 6, 36, 6, 2, 10, 6, 24, 6, 14, 16, 6, 18,
    2, 10, 20, 10, 8, 6, 4, 6, 2, 10, 2, 12, 4, 2, 4, 8, 10, 6, 12, 18,
    14, 12, 16, 8, 6, 16, 8, 4, 2, 6, 18, 24, 18, 10, 12, 2, 4, 14, 10, 6,
    6, 6, 18, 12, 2, 28, 18, 14, 16, 12, 14, 24, 12, 22, 6, 2, 10, 8, 4, 2,
    4, 14, 12, 6, 4, 6, 14, 4, 2, 4, 30, 6, 2, 6, 10, 2, 30, 22, 2, 4,
    6, 8, 6, 6, 16, 12, 12, 6, 8, 4, 2, 24, 12, 4, 6, 8, 6, 6, 10, 2,
    6, 12, 28, 14, 6, 4, 12, 8, 6, 12, 4, 6, 14, 6, 12, 10, 6, 6, 8, 6,
    6, 4, 2, 4, 8, 12, 4, 14, 18, 10, 2, 16, 6, 20, 6, 10, 8, 4, 30, 36,
    12, 8, 22, 12, 2, 6, 12, 16, 6, 6, 2, 18, 4, 26, 4, 8, 18, 10, 8, 10,
    6, 14, 4, 20, 22, 18, 12, 8, 28, 12, 6, 6, 8, 6, 12, 24, 16, 14, 4, 14,
    12, 6, 10, 12, 20, 6, 4, 8, 18, 12, 18, 10, 2, 4, 20, 10, 14, 4, 6, 2,
    10, 24, 18, 2, 4, 20, 16, 14, 10, 14, 6, 4, 6, 20, 6, 10, 6, 2, 12, 6,
    30, 10, 8, 6, 4, 6, 8, 40, 2, 4, 2, 12, 18, 4, 6, 8, 10, 6, 18, 18,
    2, 12, 16, 8, 6, 4, 6, 6, 2, 52, 14, 4, 20, 16, 2, 4, 6, 12, 2, 6,
    12, 12, 6, 4, 14, 10, 6, 6, 14, 10, 14, 16, 8, 6, 12, 4, 8, 22, 6, 2,
    18, 22, 6, 2, 18, 6, 16, 14, 10, 6, 12, 2, 6, 4, 8, 18, 12, 16, 2, 4,
    14, 4, 8, 12, 12, 30, 16, 8, 4, 2, 6, 22, 12, 8, 10, 6, 6, 6, 14, 6,
    18, 10, 12, 2, 10, 2, 4, 26, 4, 12, 8, 4, 18, 8, 10, 14, 16, 6, 6, 8,
    10, 6, 8, 6, 12, 10, 20, 10, 8, 4, 12, 26, 18, 4, 12, 18, 6, 30, 6, 8,
    6, 22, 12, 2, 4, 6, 6, 2, 10, 2, 4, 6, 6, 2, 6, 22, 18, 6, 18, 12,
    8, 12, 6, 10, 12, 2, 16, 2, 10, 2, 10, 18, 6, 20, 4, 2, 6, 22, 6, 6,
    18, 6, 14, 12, 16, 2, 6, 6, 4, 14, 12, 4, 2, 18, 16, 36, 12, 6, 14, 28,
    2, 12, 6, 12, 6, 4, 2, 16, 30, 8, 24, 6, 30, 10, 2, 18, 4, 6, 12, 8,
    22, 2, 6, 22, 18, 2, 10, 2, 10, 30, 2, 28, 6, 14, 16, 6, 20, 16, 2, 6,
    4, 32, 4, 2, 4, 6, 2, 12, 4, 6, 6, 12, 2, 6, 4, 6, 8, 6, 4, 20,
    4, 32, 10, 8, 16, 2, 22, 2, 4, 6, 8, 6, 16, 14, 4, 18, 8, 4, 20, 6,
    12, 12, 6, 10, 2, 10, 2, 12, 28, 12, 18, 2, 18, 10, 8, 10, 48, 2, 4, 6,
    8, 10, 2, 10, 30, 2, 36, 6, 10, 6, 2, 18, 4, 6, 8, 16, 14, 16, 6, 14,
    4, 20, 4, 6, 2, 10, 12, 2, 6, 12, 6, 6, 4, 12, 2, 6, 4, 12, 6, 8,
    4, 2, 6, 18, 10, 6, 8, 12, 6, 22, 2, 6, 12, 18, 4, 14, 6, 4, 20, 6,
    16, 8, 4, 8, 22, 8, 12, 6, 6, 16, 12, 18, 30, 8, 4, 2, 4, 6, 26, 4,
    14, 24, 22, 6, 2, 6, 10, 6, 14, 6, 6, 12, 10, 6, 2, 12, 10, 12, 8, 18,
    18, 10, 6, 8, 16, 6, 6, 8, 16, 20, 4, 2, 10, 2, 10, 12, 6, 8, 6, 10,
    20, 10, 18, 26, 4, 6, 30, 2, 4, 8, 6, 12, 12, 18, 4, 8, 22, 6, 2, 12,
    34, 6, 18, 12, 6, 2, 28, 14, 16, 14, 4, 14, 12, 4, 6, 6, 2, 36, 4, 6,
    20, 12, 24, 6, 22, 2, 16, 18, 12, 12, 18, 2, 6, 6, 6, 4, 6, 14, 4, 2,
    22, 8, 12, 6, 10, 6, 8, 12, 18, 12, 6, 10, 2, 22, 14, 6, 6, 4, 18, 6,
    20, 22, 2, 12, 24, 4, 18, 18, 2, 22, 2, 4, 12, 8, 12, 10, 14, 4, 2, 18,
    16, 38, 6, 6, 6, 12, 10, 6, 12, 8, 6, 4, 6, 14, 30, 6, 10, 8, 22, 6,
    8, 12, 10, 2, 10, 2, 6, 10, 2, 10, 12, 18, 20, 6, 4, 8, 22, 6, 6, 30,
    6, 14, 6, 12, 12, 6, 10, 2, 10, 30, 2, 16, 8, 4, 2, 6, 18, 4, 2, 6,
    4, 26, 4, 8, 6, 10, 2, 4, 6, 8, 4, 6, 30, 12, 2, 6, 6, 4, 20, 22,
    8, 4, 2, 4, 72, 8, 4, 8, 22, 2, 4, 14, 10, 2, 4, 20, 6, 10, 18, 6,
    20, 16, 6, 8, 6, 4, 20, 12, 22, 2, 4, 2, 12, 10, 18, 2, 22, 6, 18, 30,
    2, 10, 14, 10, 8, 16, 50, 6, 10, 8, 10, 12, 6, 18, 2, 22, 6, 2, 4, 6,
    8, 6, 6, 10, 18, 2, 22, 2, 16, 14, 10, 6, 2, 12, 10, 20, 4, 14, 6, 4,
    36, 2, 4, 6, 12, 2, 4, 14, 12, 6, 4, 6, 2, 6, 4, 20, 10, 2, 10, 6,
    12, 2, 24, 12, 12, 6, 6, 4, 24, 2, 4, 24, 2, 6, 4, 6, 8, 16, 6, 2,
    10, 12, 14, 6, 34, 6, 14, 6, 4, 2, 30, 22, 8, 4, 6, 8, 4, 2, 28, 2,
    6, 4, 26, 18, 22, 2, 6, 16, 6, 2, 16, 12, 2, 12, 4, 6, 6, 14, 10, 6,
    8, 12, 4, 18, 2, 10, 8, 16, 6, 6, 30, 2, 10, 18, 2, 10, 8, 4, 8, 12,
    24, 40, 2, 12, 10, 6, 12, 2, 12, 4, 2, 4, 6, 18, 14, 12, 6, 4, 14, 30,
    4, 8, 10, 8, 6, 10, 18, 8, 4, 14, 16, 6, 8, 4, 6, 2, 10, 2, 12, 4,
    2, 4, 6, 8, 4, 6, 32, 24, 10, 8, 18, 10, 2, 6, 10, 2, 4, 18, 6, 12,
    2, 16, 2, 22, 6, 6, 8, 18, 4, 18, 12, 8, 6, 4, 20, 6, 30, 22, 12, 2,
    6, 18, 4, 62, 4, 2, 12, 6, 10, 2, 12, 12, 28, 2, 4, 14, 22, 6, 2, 6,
    6, 10, 14, 4, 2, 10, 6, 8, 10, 14, 10, 6, 2, 12, 22, 18, 8, 10, 18, 12,
    2, 12, 4, 12, 2, 10, 2, 6, 18, 6, 6, 34, 6, 2, 12, 4, 6, 18, 18, 2,
    16, 6, 6, 8, 6, 10, 18, 8, 10, 8, 10, 2, 4, 18, 26, 12, 22, 2, 4, 2,
    22, 6, 6, 14, 16, 6, 20, 10, 12, 2, 18, 42, 4, 24, 2, 6, 10, 12, 2, 6,
    10, 8, 4, 6, 12, 12, 8, 4, 6, 12, 30, 20, 6, 24, 6, 10, 12, 2, 10, 20,
    6, 6, 4, 12, 14, 10, 18, 12, 8, 6, 12, 4, 14, 10, 2, 12, 30, 16, 2, 12,
    6, 4, 2, 4, 6, 26, 4, 18, 2, 4, 6, 14, 54, 6, 52, 2, 16, 6, 6, 12,
    26, 4, 2, 6, 22, 6, 2, 12, 12, 6, 10, 18, 2, 12, 12, 10, 18, 12, 6, 8,
    6, 10, 6, 8, 4, 2, 4, 20, 24, 6, 6, 10, 14, 10, 2, 22, 6, 14, 10, 26,
    4, 18, 8, 12, 12, 10, 12, 6, 8, 16, 6, 8, 6, 6, 22, 2, 10, 20, 10, 6,
    44, 18, 6, 10, 2, 4, 6, 14, 4, 26, 4, 2, 12, 10, 8, 4, 8, 12, 4, 12,
    8, 22, 8, 6, 10, 18, 6, 6, 8, 6, 12, 4, 8, 18, 10, 12, 6, 12, 2, 6,
    4, 2, 16, 12, 12, 14, 10, 14, 6, 10, 12, 2, 12, 6, 4, 6, 2, 12, 4, 26,
    6, 18, 6, 10, 6, 2, 18, 10, 8, 4, 26, 10, 20, 6, 16, 20, 12, 10, 8, 10,
    2, 16, 6, 20, 10, 20, 4, 30, 2, 4, 8, 16, 2, 18, 4, 2, 6, 10, 18, 12,
    14, 18, 6, 16, 20, 6, 4, 8, 6, 4, 6, 12, 8, 10, 2, 12, 6, 4, 2, 6,
    10, 2, 16, 12, 14, 10, 6, 8, 6, 28, 2, 6, 18, 30, 34, 2, 16, 12, 2, 18,
    16, 6, 8, 10, 8, 10, 8, 10, 44, 6, 6, 4, 20, 4, 2, 4, 14, 28, 8, 6,
    16, 14, 30, 6, 30, 4, 14, 10, 6, 6, 8, 4, 18, 12, 6, 2, 22, 12, 8, 6,
    12, 4, 14, 4, 6, 2, 4, 18, 20, 6, 16, 38, 16, 2, 4, 6, 2, 40, 42, 14,
    4, 6, 2, 24, 10, 6, 2, 18, 10, 12, 2, 16, 2, 6, 16, 6, 8, 4, 2, 10,
    6, 8, 10, 2, 18, 16, 8, 12, 18, 12, 6, 12, 10, 6, 6, 18, 12, 14, 4, 2,
    10, 20, 6, 12, 6, 16, 26, 4, 18, 2, 4, 32, 10, 8, 6, 4, 6, 6, 14, 6,
    18, 4, 2, 18, 10, 8, 10, 8, 10, 2, 4, 6, 2, 10, 42, 8, 12, 4, 6, 18,
    2, 16, 8, 4, 2, 10, 14, 12, 10, 20, 4, 8, 10, 38, 4, 6, 2, 10, 20, 10,
    12, 6, 12, 26, 12, 4, 8, 28, 8, 4, 8, 24, 6, 10, 8, 6, 16, 12, 8, 10,
    12, 8, 22, 6, 2, 10, 2, 6, 10, 6, 6, 8, 6, 4, 14, 28, 8, 16, 18, 8,
    4, 6, 20, 4, 18, 6, 2, 24, 24, 6, 6, 12, 12, 4, 2, 22, 2, 10, 6, 8,
    12, 4, 20, 18, 6, 4, 12, 24, 6, 6, 54, 8, 6, 4, 26, 36, 4, 2, 4, 26,
    12, 12, 4, 6, 6, 8, 12, 10, 2, 12, 16, 18, 6, 8, 6, 12, 18, 10, 2, 54,
    4, 2, 10, 30, 12, 8, 4, 8, 16, 14, 12, 6, 4, 6, 12, 6, 2, 4, 14, 12,
    4, 14, 6, 24, 6, 6, 10, 12, 12, 20, 18, 6, 6, 16, 8, 4, 6, 20, 4, 32,
    4, 14, 10, 2, 6, 12, 16, 2, 4, 6, 12, 2, 10, 8, 6, 4, 2, 10, 14, 6,
    6, 12, 18, 34, 8, 10, 6, 24, 6, 2, 10, 12, 2, 30, 10, 14, 12, 12, 16, 6,
    6, 2, 18, 4, 6, 30, 14, 4, 6, 6, 2, 6, 4, 6, 14, 6, 4, 8, 10, 12,
    6, 32, 10, 8, 22, 2, 10, 6, 24, 8, 4, 30, 6, 2, 12, 16, 8, 6, 4, 6,
    8, 16, 14, 6, 6, 4, 2, 10, 12, 2, 16, 14, 4, 2, 4, 20, 18, 10, 2, 10,
    6, 12, 30, 8, 18, 12, 10, 2, 6, 6, 4, 12, 12, 2, 4, 12, 18, 24, 2, 10,
    6, 8, 16, 8, 6, 12, 10, 14, 6, 12, 6, 6, 4, 2, 24, 4, 6, 8, 6, 4,
    2, 4, 6, 14, 4, 8, 10, 24, 24, 12, 2, 6, 12, 22, 30, 2, 6, 18, 10, 6,
    6, 8, 4, 2, 6, 10, 8, 10, 6, 8, 16, 6, 14, 6, 4, 24, 8, 10, 2, 12,
    6, 4, 36, 2, 22, 6, 8, 6, 10, 8, 6, 12, 10, 14, 10, 6, 18, 12, 2, 12,
    4, 26, 10, 14, 16, 18, 8, 18, 12, 12, 6, 16, 14, 24, 10, 12, 8, 22, 6, 2,
    10, 60, 6, 2, 4, 8, 16, 14, 10, 6, 24, 6, 12, 18, 24, 2, 30, 4, 2, 12,
    6, 10, 2, 4, 14, 6, 16, 2, 10, 8, 22, 20, 6, 4, 32, 6, 18, 4, 2, 4,
    2, 4, 8, 52, 14, 22, 2, 22, 20, 10, 8, 10, 2, 6, 4, 14, 4, 6, 20, 4,
    6, 2, 12, 12, 6, 12, 16, 2, 12, 10, 8, 4, 6, 2, 28, 12, 8, 10, 12, 2,
    4, 14, 28, 8, 6, 4, 2, 4, 6, 2, 12, 58, 6, 14, 10, 2, 6, 28, 32, 4,
    30, 8, 6, 4, 6, 12, 12, 2, 4, 6, 6, 14, 16, 8, 30, 4, 2, 10, 8, 6,
    4, 6, 26, 4, 12, 2, 10, 18, 12, 12, 18, 2, 4, 12, 8, 12, 10, 20, 4, 8,
    16, 12, 8, 6, 16, 8, 10, 12, 14, 6, 4, 8, 12, 4, 20, 6, 40, 8, 16, 6,
    36, 2, 6, 4, 6, 2, 22, 18, 2, 10, 6, 36, 14, 12, 4, 18, 8, 4, 14, 10,
    2, 10, 8, 4, 2, 18, 16, 12, 14, 10, 14, 6, 6, 42, 10, 6, 6, 20, 10, 8,
    12, 4, 12, 18, 2, 10, 14, 18, 10, 18, 8, 6, 4, 14, 6, 10, 30, 14, 6, 6,
    4, 12, 38, 4, 2, 4, 6, 8, 12, 10, 6, 18, 6, 50, 6, 4, 6, 12, 8, 10,
    32, 6, 22, 2, 10, 12, 18, 2, 6, 4, 30, 8, 6, 6, 18, 10, 2, 4, 12, 20,
    10, 8, 24, 10, 2, 6, 22, 6, 2, 18, 10, 12, 2, 30, 18, 12, 28, 2, 6, 4,
    6, 14, 6, 12, 10, 8, 4, 12, 26, 10, 8, 6, 16, 2, 10, 18, 14, 6, 4, 6,
    14, 16, 2, 6, 4, 12, 20, 4, 20, 4, 6, 12, 2, 36, 4, 6, 2, 10, 2, 22,
    8, 6, 10, 12, 12, 18, 14, 24, 36, 4, 20, 24, 10, 6, 2, 28, 6, 18, 8, 4,
    6, 8, 6, 4, 2, 12, 28, 18, 14, 16, 14, 18, 10, 8, 6, 4, 6, 6, 8, 22,
    12, 2, 10, 18, 6, 2, 18, 10, 2, 12, 10, 18, 32, 6, 4, 6, 6, 8, 6, 6,
    10, 20, 6, 12, 10, 8, 10, 14, 6, 10, 14, 4, 2, 22, 18, 2, 10, 2, 4, 20,
    4, 2, 34, 2, 12, 6, 10, 2, 10, 18, 6, 14, 12, 12, 22, 8, 6, 16, 6, 8,
    4, 12, 6, 8, 4, 36, 6, 6, 20, 24, 6, 12, 18, 10, 2, 10, 26, 6, 16, 8,
    6, 4, 24, 18, 8, 12, 12, 10, 18, 12, 2, 24, 4, 12, 18, 12, 14, 10, 2, 4,
    24, 12, 14, 10, 6, 2, 6, 4, 6, 26, 4, 6, 6, 2, 22, 8, 18, 4, 18, 8,
    4, 24, 2, 12, 12, 4, 2, 52, 2, 18, 6, 4, 6, 12, 2, 6, 12, 10, 8, 4,
    2, 24, 10, 2, 10, 2, 12, 6, 18, 40, 6, 20, 16, 2, 12, 6, 10, 12, 2, 4,
    6, 14, 12, 12, 22, 6, 8, 4, 2, 16, 18, 12, 2, 6, 16, 6, 2, 6, 4, 12,
    30, 8, 16, 2, 18, 10, 24, 2, 6, 24, 4, 2, 22, 2, 16, 2, 6, 12, 4, 18,
    8, 4, 14, 4, 18, 24, 6, 2, 6, 10, 2, 10, 38, 6, 10, 14, 6, 6, 24, 4,
    2, 12, 16, 14, 16, 12, 2, 6, 10, 26, 4, 2, 12, 6, 4, 12, 8, 12, 10, 18,
    6, 14, 28, 2, 6, 10, 2, 4, 14, 34, 2, 6, 22, 2, 10, 14, 4, 2, 16, 8,
    10, 6, 8, 10, 8, 4, 6, 2, 16, 6, 6, 18, 30, 14, 6, 4, 30, 2, 10, 14,
    4, 20, 10, 8, 4, 8, 18, 4, 14, 6, 4, 24, 6, 6, 18, 18, 2, 36, 6, 10,
    14, 12, 4, 6, 2, 30, 6, 4, 2, 6, 28, 20, 4, 20, 12, 24, 16, 18, 12, 14,
    6, 4, 12, 32, 12, 6, 10, 8, 10, 6, 18, 2, 16, 14, 6, 22, 6, 12, 2, 18,
    4, 8, 30, 12, 4, 12, 2, 10, 38, 22, 2, 4, 14, 6, 12, 24, 4, 2, 4, 14,
    12, 10, 2, 16, 6, 20, 4, 20, 22, 12, 2, 4, 2, 12, 22, 24, 6, 6, 2, 6,
    4, 6, 2, 10, 12, 12, 6, 2, 6, 16, 8, 6, 4, 18, 12, 12, 14, 4, 12, 6,
    8, 6, 18, 6, 10, 12, 14, 6, 4, 8, 22, 6, 2, 28, 18, 2, 18, 10, 6, 14,
    10, 2, 10, 14, 6, 10, 2, 22, 6, 8, 6, 16, 12, 8, 22, 2, 4, 14, 18, 12,
    6, 24, 6, 10, 2, 12, 22, 18, 6, 20, 6, 10, 14, 4, 2, 6, 12, 22, 14, 12,
    4, 6, 8, 22, 2, 10, 12, 8, 40, 2, 6, 10, 8, 4, 42, 20, 4, 32, 12, 10,
    6, 12, 12, 2, 10, 8, 6, 4, 8, 4, 26, 18, 4, 8, 28, 6, 18, 6, 12, 2,
    10, 6, 6, 14, 10, 12, 14, 24, 6, 4, 20, 22, 2, 18, 4, 6, 12, 2, 16, 18,
    14, 6, 6, 4, 6, 8, 18, 4, 14, 30, 4, 18, 8, 10, 2, 4, 8, 12, 4, 12,
    18, 2, 12, 10, 2, 16, 8, 4, 30, 2, 6, 28, 2, 10, 2, 18, 10, 14, 4, 26,
    6, 18, 4, 20, 6, 4, 8, 18, 4, 12, 26, 24, 4, 20, 22, 2, 18, 22, 2, 4,
    12, 2, 6, 6, 6, 4, 6, 14, 4, 24, 12, 6, 18, 2, 12, 28, 14, 4, 6, 8,
    22, 6, 12, 18, 8, 4, 20, 6, 4, 6, 2, 18, 6, 4, 12, 12, 8, 28, 6, 8,
    10, 2, 24, 12, 10, 24, 8, 10, 20, 12, 6, 12, 12, 4, 14, 12, 24, 34, 18, 8,
    10, 6, 18, 8, 4, 8, 16, 14, 6, 4, 6, 24, 2, 6, 4, 6, 2, 16, 6, 6,
    20, 24, 4, 2, 4, 14, 4, 18, 2, 6, 12, 4, 14, 4, 2, 18, 16, 6, 6, 2,
    16, 20, 6, 6, 30, 4, 8, 6, 24, 16, 6, 6, 8, 12, 30, 4, 18, 18, 8, 4,
    26, 10, 2, 22, 8, 10, 14, 6, 4, 18, 8, 12, 28, 2, 6, 4, 12, 6, 24, 6,
    8, 10, 20, 16, 8, 30, 6, 6, 4, 2, 10, 14, 6, 10, 32, 22, 18, 2, 4, 2,
    4, 8, 22, 8, 18, 12, 28, 2, 16, 12, 18, 14, 10, 18, 12, 6, 32, 10, 14, 6,
    10, 2, 10, 2, 6, 22, 2, 4, 6, 8, 10, 6, 14, 6, 4, 12, 30, 24, 6, 6,
    8, 6, 4, 2, 4, 6, 8, 6, 6, 22, 18, 8, 4, 2, 18, 6, 4, 2, 16, 18,
    20, 10, 6, 6, 30, 2, 12, 28, 6, 6, 6, 2, 12, 10, 8, 18, 18, 4, 8, 18,
    10, 2, 28, 2, 10, 14, 4, 2, 30, 12, 22, 26, 10, 8, 6, 10, 8, 16, 14, 6,
    6, 10, 14, 6, 4, 2, 10, 12, 2, 6, 10, 8, 4, 2, 10, 26, 22, 6, 2, 12,
    18, 4, 26, 4, 8, 10, 6, 14, 10, 2, 18, 6, 10, 20, 6, 6, 4, 24, 2, 4,
    8, 6, 16, 14, 16, 18, 2, 4, 12, 2, 10, 2, 6, 12, 10, 6, 6, 20, 6, 4,
    6, 38, 4, 6, 12, 14, 4, 12, 8, 10, 12, 12, 8, 4, 6, 14, 10, 6, 12, 2,
    10, 18, 2, 18, 10, 8, 10, 2, 12, 4, 14, 28, 2, 16, 2, 18, 6, 10, 6, 8,
    16, 14, 30, 10, 20, 6, 10, 24, 2, 28, 2, 12, 16, 6, 8, 36, 4, 8, 4, 14,
    12, 10, 8, 12, 4, 6, 8, 4, 6, 14, 22, 8, 6, 4, 2, 10, 6, 20, 10, 8,
    6, 6, 22, 18, 2, 16, 6, 20, 4, 26, 4, 14, 22, 14, 4, 12, 6, 8, 4, 6,
    6, 26, 10, 2, 18, 18, 4, 2, 16, 2, 18, 4, 6, 8, 4, 6, 12, 2, 6, 6,
    28, 38, 4, 8, 16, 26, 4, 2, 10, 12, 2, 10, 8, 6, 10, 12, 2, 10, 2, 24,
    4, 30, 26, 6, 6, 18, 6, 6, 22, 2, 10, 18, 26, 4, 18, 8, 6, 6, 12, 16,
    6, 8, 16, 6, 8, 16, 2, 42, 58, 8, 4, 6, 2, 4, 8, 16, 6, 20, 4, 12,
    12, 6, 12, 2, 10, 2, 6, 22, 2, 10, 6, 8, 6, 10, 14, 6, 6, 4, 18, 8,
    10, 8, 16, 14, 10, 2, 10, 2, 12, 6, 4, 20, 10, 8, 52, 8, 10, 6, 2, 10,
    8, 10, 6, 6, 8, 10, 2, 22, 2, 4, 6, 14, 4, 2, 24, 12, 4, 26, 18, 4,
    6, 14, 30, 6, 4, 6, 2, 22, 8, 4, 6, 2, 22, 6, 8, 16, 6, 14, 4, 6,
    18, 8, 12, 6, 12, 24, 30, 16, 8, 34, 8, 22, 6, 14, 10, 18, 14, 4, 12, 8,
    4, 36, 6, 6, 2, 10, 2, 4, 20, 6, 6, 10, 12, 6, 2, 40, 8, 6, 28, 6,
    2, 12, 18, 4, 24, 14, 6, 6, 10, 20, 10, 14, 16, 14, 16, 6, 8, 36, 4, 12,
    12, 6, 12, 50, 12, 6, 4, 6, 6, 8, 6, 10, 2, 10, 2, 18, 10, 14, 16, 8,
    6, 4, 20, 4, 2, 10, 6, 14, 18, 10, 38, 10, 18, 2, 10, 2, 12, 4, 2, 4,
    14, 6, 10, 8, 40, 6, 20, 4, 12, 8, 6, 34, 8, 22, 8, 12, 10, 2, 16, 42,
    12, 8, 22, 8, 22, 8, 6, 34, 2, 6, 4, 14, 6, 16, 2, 22, 6, 8, 24, 22,
    6, 2, 12, 4, 6, 14, 4, 8, 24, 4, 6, 6, 2, 22, 20, 6, 4, 14, 4, 6,
    6, 8, 6, 10, 6, 8, 6, 16, 14, 6, 6, 22, 6, 24, 32, 6, 18, 6, 18, 10,
    8, 30, 18, 6, 16, 12, 6, 12, 2, 6, 4, 12, 8, 6, 22, 8, 6, 4, 14, 10,
    18, 20, 10, 2, 6, 4, 2, 28, 18, 2, 10, 6, 6, 6, 14, 40, 24, 2, 4, 8,
    12, 4, 20, 4, 32, 18, 16, 6, 36, 8, 6, 4, 6, 14, 4, 6, 26, 6, 10, 14,
    18, 10, 6, 6, 14, 10, 6, 6, 14, 6, 24, 4, 14, 22, 8, 12, 10, 8, 12, 18,
    10, 18, 8, 24, 10, 8, 4, 24, 6, 18, 6, 2, 10, 30, 2, 10, 2, 4, 2, 40,
    2, 28, 8, 6, 6, 18, 6, 10, 14, 4, 18, 30, 18, 2, 12, 30, 6, 30, 4, 18,
    12, 2, 4, 14, 6, 10, 6, 8, 6, 10, 12, 2, 6, 12, 10, 2, 18, 4, 20, 4,
    6, 14, 6, 6, 22, 6, 6, 8, 18, 18, 10, 2, 10, 2, 6, 4, 6, 12, 18, 2,
    10, 8, 4, 18, 2, 6, 6, 6, 10, 8, 10, 6, 18, 12, 8, 12, 6, 4, 6, 14,
    16, 2, 12, 4, 6, 38, 6, 6, 16, 20, 28, 20, 10, 6, 6, 14, 4, 26, 4, 14,
    10, 18, 14, 28, 2, 4, 14, 16, 2, 28, 6, 8, 6, 34, 8, 4, 18, 2, 16, 8,
    6, 40, 8, 18, 4, 30, 6, 12, 2, 30, 6, 10, 14, 40, 14, 10, 2, 12, 10, 8,
    4, 8, 6, 6, 28, 2, 4, 12, 14, 16, 8, 30, 16, 18, 2, 10, 18, 6, 32, 4,
    18, 6, 2, 12, 10, 18, 2, 6, 10, 14, 18, 28, 6, 8, 16, 2, 4, 20, 10, 8,
    18, 10, 2, 10, 8, 4, 6, 12, 6, 20, 4, 2, 6, 4, 20, 10, 26, 18, 10, 2,
    18, 6, 16, 14, 4, 26, 4, 14, 10, 12, 14, 6, 6, 4, 14, 10, 2, 30, 18, 22, 2};


