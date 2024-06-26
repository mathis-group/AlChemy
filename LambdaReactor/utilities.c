/*
    utilities.c

    c 1994 Walter Fontana and Ivo Hofacker
 */

#include <stdio.h>
#include <malloc.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "include.h"
#include <time.h>
#include <stdlib.h>

PUBLIC void randomize (long val);
PUBLIC double urn (void);
PUBLIC void *space (unsigned int size);
PUBLIC void nrerror (char *message);
PUBLIC int int_urn (int from, int to);
PUBLIC void file_copy (FILE * from, FILE * to);
PUBLIC char *time_stamp (void);
PUBLIC char *random_string (int l, char *symbols);
PUBLIC int hamming (char *s1, char *s2);
PUBLIC char *get_line (FILE * fp);
PUBLIC int lines_in_file (char *fname);
PUBLIC boolean check_line (FILE *fp);
PUBLIC void next_line (FILE * fp);
PUBLIC int str_index (char *s, char *t);
PUBLIC void reverse (char *s);
PUBLIC void sort (int n, int *ra);
PUBLIC double cpu_time (void);
PUBLIC char *int_to_char (int n, char *s);
PUBLIC char *float_to_char (float n, char *s);
PUBLIC char *double_to_char (double n, char *s);
PUBLIC int **int_matrix (int nrl, int nrh, int ncl, int nch);
PUBLIC void free_int_matrix (int **m, int nrl, int nrh, int ncl, int nch);
PUBLIC float **float_matrix (int rl, int rh, int cl, int ch);
PUBLIC void free_float_matrix (float **m, int nrl, int nrh, int ncl, int nch);
PUBLIC double **double_matrix (int nrl, int nrh, int ncl, int nch);
PUBLIC void free_double_matrix (double **m, int nrl, int nrh, int ncl, int nch);

/*-------------------------------------------------------------------------*/

PUBLIC void *
space (unsigned size)
{
  void *pointer;

  if ((pointer = (void *) calloc (1, size)) == NULL)
    {
      if (errno == EINVAL)
	{
	  fprintf (stderr, "SPACE: requested size: %d\n", size);
	  nrerror ("SPACE allocation failure -> EINVAL");
	}
      if (errno == ENOMEM)
	nrerror ("SPACE allocation failure -> no memory");
    }
  return pointer;
}

/*------------------------------------------------------------------------*/

PUBLIC void
nrerror (char *message)		/* output message upon error */
{
  fprintf (stderr, "\n%s\n", message);
  exit (-1);
}

/*------------------------------------------------------------------------*/

PUBLIC int
int_urn (int from, int to)
{
  return (((int) (urn () * (to - from + 1))) + from);
}

/*------------------------------------------------------------------------*/

PUBLIC void
file_copy (FILE * from, FILE * to)
{
  int c;

  while ((c = getc (from)) != EOF)
    putc (c, to);
}

/*------------------------------------------------------------------------*/

PUBLIC char *
time_stamp (void)
{
  time_t cal_time;

  cal_time = time (NULL);
  return (ctime (&cal_time));
}

/*-----------------------------------------------------------------*/

PUBLIC char *
random_string (int l, char *symbols)
{
  char *r;
  int i, rn, base;

  base = strlen (symbols);
  r = (char *) space (sizeof (char) * (l + 1));

  for (i = 0; i < l; i++)
    {
      rn = (int) (urn () * base);	/* [0, base-1] */
      r[i] = symbols[rn];
    }
  r[l] = '\0';
  return r;
}

/*-----------------------------------------------------------------*/

PUBLIC int
hamming (char *s1, char *s2)
{
  int h = 0, i;

  for (i = 0; i < strlen (s1); i++)
    if (s1[i] != s2[i])
      h++;
  return h;
}

/*-----------------------------------------------------------------*/

PUBLIC char *
get_line (FILE * fp)  /* reads lines of arbitrary length from fp */
{
  char s[512], *line, *cp;

  line = NULL;
  do
    {
      if (fgets (s, 512, fp) == NULL)
	break;
      cp = strchr (s, '\n');
      if (cp != NULL)
	*cp = '\0';
      if (line == NULL)
	line = (char *) space (strlen (s) + 1);
      else
	line = (char *) realloc (line, strlen (s) + strlen (line) + 1);
      strcat (line, s);
    }
  while (cp == NULL);

  return line;
}

/*------------------------------------------------------------------------*/

PUBLIC int
lines_in_file (char *fname)  /* returns the number of lines in a file */
{
  register int l;
  FILE *fp, *fopen ();
  
  fp = fopen (fname, "r");
  if (!fp)
    {
      printf ("file %s does not exist\n", fname);
      exit (-1);
    }
  l = 0;
  while (check_line (fp)) l++;
  fclose (fp);
  
  return l;
}

/*------------------------------------------------------------------------*/

PUBLIC boolean
check_line (FILE *fp)	  /* 1 ... line read, 0 ... EOF encountered */
{
  char s[512], *cp;

  do
    {
      if (fgets (s, 512, fp) == NULL)
	return FALSE;
      cp = strchr (s, '\n');
      if (cp != NULL)
	return TRUE;
    }
  while (cp == NULL);
}

/*------------------------------------------------------------------------*/

PUBLIC void
next_line (FILE * fp)

{
  char c;

  while ((c = getc (fp)) != '\n');
}

/*-----------------------------------------------------------------------*/

PUBLIC int
str_index (char *s, char *t)

/* return index of t in s, -1 if none, K&R p69 */

{
  register i, j, k;

  if (!s || !t)
    return (-1);

  for (i = 0; s[i] != '\0'; i++)
    {
      for (j = i, k = 0; t[k] != '\0' && s[j] == t[k]; j++, k++);
      if (k > 0 && t[k] == '\0')
	return (i);
    }
  return (-1);
}

/*-------------------------------------------------------------------------*/

PUBLIC void
reverse (char *s)
{
  long c, i, j;

  for (i = 0, j = strlen (s) - 1; i < j; i++, j--)
    {
      c = s[i];
      s[i] = s[j];
      s[j] = c;
    }
}

/*-------------------------------------------------------------------------------*/

PUBLIC void
sort (int n, int *ra)		/* heap sort */
{
  int l, j, ir, i;
  int rra;

  if (n == 0 || n == 1)
    return;

  l = (n >> 1) + 1;
  ir = n;
  for (;;)
    {
      if (l > 1)
	rra = ra[--l];
      else
	{
	  rra = ra[ir];
	  ra[ir] = ra[1];
	  if (--ir == 1)
	    {
	      ra[1] = rra;
	      return;
	    }
	}
      i = l;
      j = l << 1;
      while (j <= ir)
	{
	  if (j < ir && ra[j] < ra[j + 1])
	    ++j;
	  if (rra < ra[j])
	    {
	      ra[i] = ra[j];
	      j += (i = j);
	    }
	  else
	    j = ir + 1;
	}
      ra[i] = rra;
    }
}

/*-------------------------------------------------------------------------------*/

/* return cpu seconds elapsed since last call to routine */

PUBLIC double
cpu_time (void)
{
  static double previous_measurement = 0.;
  double elapsed_secs, increment;
  struct rusage usage;

  if (getrusage (RUSAGE_SELF, &usage) == -1)
    nrerror ("cpu_time(), getrusage error condition.");

  elapsed_secs = (double) usage.ru_utime.tv_sec +
    (double) usage.ru_utime.tv_usec / 1000000.;

  increment = elapsed_secs - previous_measurement;
  previous_measurement = elapsed_secs;

  return increment;
}

/*-------------------------------------------------------------------------*/

PUBLIC char *
int_to_char (int n, char *s)
{
  sprintf (s, "%d", n);
  return (s);
}

/*-------------------------------------------------------------------------*/

PUBLIC char *
float_to_char (float n, char *s)
{
  sprintf (s, "%f", n);
  return (s);
}

/*-------------------------------------------------------------------------*/

PUBLIC char *
double_to_char (double n, char *s)
{
  sprintf (s, "%lf", n);
  return (s);
}

/*-------------------------------------------------------------------------*/

PUBLIC int **
int_matrix (int nrl, int nrh, int ncl, int nch)
{
  int i;
  int **m;

  m = (int **) space ((unsigned) (nrh - nrl + 1) * sizeof (int *));
  if (!m)
    nrerror ("MATRIX, allocation failure 1.");
  m -= nrl;

  for (i = nrl; i <= nrh; i++)
    {
      m[i] = (int *) space ((unsigned) (nch - ncl + 1) * sizeof (int));
      if (!m[i])
	nrerror ("MATRIX, allocation failure 2.");
      m[i] -= ncl;
    }
  return m;
}

/*-------------------------------------------------------------------------*/

PUBLIC void
free_int_matrix (int **m, int nrl, int nrh, int ncl, int nch)
{
  int i;

  for (i = nrh; i >= nrl; i--)
    free (m[i] + ncl);
  free (m + nrl);
}

/*------------------------------------------------------------------------*/

PUBLIC float **
float_matrix (int rl, int rh, int cl, int ch)
{
  int i;
  float **m;

  m = (float **) space ((unsigned) (rh - rl + 1) * sizeof (float *));
  if (!m)
    nrerror ("MATRIX, allocation failure 1.");
  m -= rl;

  for (i = rl; i <= rh; i++)
    {
      m[i] = (float *) space ((unsigned) (ch - cl + 1) * sizeof (float));
      if (!m[i])
	nrerror ("MATRIX, allocation failure 2.");
      m[i] -= cl;
    }
  return m;
}

/*-------------------------------------------------------------------------*/

PUBLIC void
free_float_matrix (float **m, int nrl, int nrh, int ncl, int nch)
{
  int i;

  for (i = nrh; i >= nrl; i--)
    free (m[i] + ncl);
  free (m + nrl);
}

/*-------------------------------------------------------------------------*/

PUBLIC double **
double_matrix (int nrl, int nrh, int ncl, int nch)
{
  int i;
  double **m;

  m = (double **) space ((unsigned) (nrh - nrl + 1) * sizeof (double *));
  if (!m)
    nrerror ("MATRIX, allocation failure 1.");
  m -= nrl;

  for (i = nrl; i <= nrh; i++)
    {
      m[i] = (double *) space ((unsigned) (nch - ncl + 1) * sizeof (double));
      if (!m[i])
	nrerror ("MATRIX, allocation failure 2.");
      m[i] -= ncl;
    }
  return m;
}

/*-------------------------------------------------------------------------*/

PUBLIC void
free_double_matrix (double **m, int nrl, int nrh, int ncl, int nch)
{
  int i;

  for (i = nrh; i >= nrl; i--)
    free (m[i] + ncl);
  free (m + nrl);
}

/****************************************************************************
 * PMMMLCG - Prime Modulus M Multiplicative Linear Congruential Generator   *
 *  Modified version of the Random number generator proposed by             *
 *  Park & Miller in "Random Number Generators: Good Ones Are Hard to Find" *
 *  CACM October 1988, Vol 31, No. 10                                       *
 *   - Modifications proposed by Park to provide better statistical         *
 *     properties (i.e. more "random" - less correlation between sets of    *
 *     generated numbers                                                    *
 *   - generator is of the form                                             *
 *         x = ( x * A) % M                                                 *
 *   - Choice of A & M can radically modify the properties of the generator *
 *     the current values were chosen after followup work to the original   *
 *     paper mentioned above.                                               *
 *   - The generator has a period of 2^31 - 1 with numbers generated in the *
 *     range of 0 < x < M                                                   *
 *   - The generator can run on any machine with a 32-bit integer, without  *
 *     overflow.                                                            *
 *   - This generator is currently running on Sun 3/50, Sparc, IBM PC/XT,   *
 *     IBM RS/6000 just to name a few...                                    *
 ****************************************************************************
 *    John Burton                                                           *
 *    G & A Technical Software, Inc                                         *
 *    28 Research Drive                                                     *
 *    Hampton, Va. 23666                                                    *
 *                                                                          *
 *    jcburt@cs.wm.edu                                                      *
 *    jcburt@gatsibm.larc.nasa.gov                                          *
 *    burton@asdsun.larc.nasa.gov                                           *
 ****************************************************************************/

#define A 48271
#define M 2147483647
#define Q (M / A)
#define R (M % A)

PRIVATE long seed = 123456789;	/*2345678901*/
PRIVATE double temp = 1.0 / (double) M;

/*------------------------------------------------------------------------*/

PUBLIC void
randomize (long val)
{
  seed = val;
}

/*------------------------------------------------------------------------*/

PUBLIC double
urn (void)
{
  long lo, hi, test;
  hi = seed / Q;
  lo = seed % Q;
  test = A * lo - R * hi;
  seed = (test > 0) ? (test) : (test + M);
  return ((double) seed * temp);
}
