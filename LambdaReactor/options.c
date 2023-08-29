/*
    options.c;  interface to GNU's getopt
    
    c 1994 Walter Fontana
 */

#include <stdio.h>
#include <string.h>
#include "include.h"
#include "options.h"
#include <stdlib.h>

#define SIZE_SHORT 300
#define SIZE_LONG  300

PUBLIC void command_line_args (int argc, char *argv[], Option option[]);

PRIVATE void set_option (Option * option, char *name, char *argument);
PRIVATE void print_description (Option option[]);
PRIVATE void help (char *progname, Option option[]);
PRIVATE void echo_settings (Option option[]);
PRIVATE void make_options (Option option[]);

/*-------------------------------------------------------------------------*/

PRIVATE char shortOptions[SIZE_SHORT];
PRIVATE struct option longOptions[SIZE_LONG];

/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/

PRIVATE void
set_option (Option * option, char *name, char *argument)
{
  int i = 0, rc = 0;

  while (option[i].name)
    {
      if (strcmp (option[i].name, name) == 0)
	break;
      i++;
    }

  switch ((int) (option[i].type))
    {
    case OPT_SWITCH:
      rc = 1;
      *((boolean *) option[i].arg) = (*((boolean *) option[i].arg) + 1) % 2;
      break;
    case OPT_INTEGER:
      rc = sscanf (argument, "%d", option[i].arg);
      break;
    case OPT_HEX:
      rc = sscanf (argument, "%x", option[i].arg);
      break;
    case OPT_OCTAL:
      rc = sscanf (argument, "%o", option[i].arg);
      break;
    case OPT_UNSIGNED:
      rc = sscanf (argument, "%u", option[i].arg);
      break;
    case OPT_LINTEGER:
      rc = sscanf (argument, "%ld", option[i].arg);
      break;
    case OPT_LHEX:
      rc = sscanf (argument, "%lx", option[i].arg);
      break;
    case OPT_LOCTAL:
      rc = sscanf (argument, "%lo", option[i].arg);
      break;
    case OPT_LUNSIGNED:
      rc = sscanf (argument, "%lu", option[i].arg);
      break;
    case OPT_FLOAT:
      rc = sscanf (argument, "%f", option[i].arg);
      break;
    case OPT_DOUBLE:
      rc = sscanf (argument, "%lf", option[i].arg);
      break;
    case OPT_STRING:
      rc = 1;
      *((char **) option[i].arg) = argument;	/* This always works */
      break;
    default:
      rc = 0;
    }

  if (rc == 0)
    {
      printf ("an error occurred in setting an option value\n");
      exit (-1);
    }
}

/*-------------------------------------------------------------------------*/

PRIVATE void
print_description (Option option[])
{
  int i = 0;

  while (option[i].name)
    {
      if (option[i].type == OPT_SWITCH)
	printf ("  -%-20s       %s\n", option[i].name, option[i].desc);
      else
	printf ("  -%-20s <arg> %s\n", option[i].name, option[i].desc);
      i++;
    }
}

/*-------------------------------------------------------------------------*/

PRIVATE void
help (char *progname, Option option[])
{
  printf ("This is %s on %s at %s\n\n", progname, __DATE__, __TIME__);
  printf ("Usage: %s\n\n", progname);
  printf ("Options are:\n");
  print_description (option);
}

/*-------------------------------------------------------------------------*/

PRIVATE void
echo_settings (Option option[])
{
  int i = 0;
  char *s;

  printf ("command line settings:\n");

  while (option[i].name)
    {
      if (strlen(option[i].name) == 1)
	s = " -";
      else
	s = "--";
	
      switch ((int) (option[i].type))
	{
	case OPT_INTEGER:
	case OPT_SWITCH:
	  printf ("%s%-20s = %d\n", s, option[i].name, *((int *) option[i].arg));
	  break;
	case OPT_HEX:
	  printf ("%s%-20s = %x\n", s, option[i].name, *((int *) option[i].arg));
	  break;
	case OPT_OCTAL:
	  printf ("%s%-20s = %o\n", s, option[i].name, *((int *) option[i].arg));
	  break;
	case OPT_UNSIGNED:
	  printf ("%s%-20s = %u\n", s, option[i].name, *((unsigned int *) option[i].arg));
	  break;
	case OPT_LINTEGER:
	  printf ("%s%-20s = %ld\n", s, option[i].name, *((long *) option[i].arg));
	  break;
	case OPT_LHEX:
	  printf ("%s%-20s = %lx\n", s, option[i].name, *((long *) option[i].arg));
	  break;
	case OPT_LOCTAL:
	  printf ("%s%-20s = %lo\n", s, option[i].name, *((long *) option[i].arg));
	  break;
	case OPT_LUNSIGNED:
	  printf ("%s%-20s = %lu\n", s, option[i].name, *((unsigned long *) option[i].arg));
	  break;
	case OPT_FLOAT:
	  printf ("%s%-20s = %f\n", s, option[i].name, *((float *) option[i].arg));
	  break;
	case OPT_DOUBLE:
	  printf ("%s%-20s = %lf\n", s, option[i].name, *((double *) option[i].arg));
	  break;
	case OPT_STRING:
	  printf ("%s%-20s = %s\n", s, option[i].name, *((char **) option[i].arg));
	  break;
	default:
	  printf ("error in echoing settings\n");
	  exit (1);
	}
      i++;
    }
}

/*-------------------------------------------------------------------------*/

PRIVATE void
make_options (Option option[])
{
  int i = 0;
  int p = 0, q = 0;

  while (option[i].name)
    {
      if (strlen (option[i].name) == 1)	/* short option */
	{
	  shortOptions[p++] = *option[i].name;
	  if (option[i].type != OPT_SWITCH)
	    shortOptions[p++] = ':';
	}
      else
	{
	  longOptions[q].name = option[i].name;
	  if (option[i].type != OPT_SWITCH)
	    longOptions[q].has_arg = 1;
	  else
	    longOptions[q].has_arg = 0;
	  longOptions[q].flag = NULL;
	  longOptions[q++].val = 0;
	}
      i++;
    }
    
  /* wire help */
  
  shortOptions[p++] = 'h';
  longOptions[q].name = "help";
  longOptions[q].has_arg = 0;
  longOptions[q].flag = NULL;
  longOptions[q++].val = 0;

  shortOptions[p] = '\0';
}

/*-------------------------------------------------------------------------*/

PUBLIC void
command_line_args (int argc, char *argv[], Option option[])
{
  int opt;
  int option_index;
  char n[] = {'\0','\0'}; 
  
  make_options (option);

  while (1)
    {
      opt = getopt_long (argc, argv, shortOptions, longOptions, &option_index);

      if (opt == EOF)
	break;

      switch (opt)
	{
	case 0:		/* long option */
	  if (strcmp (longOptions[option_index].name, "help") == 0)
	    {
	      help (argv[0], option);
	      echo_settings (option);
	      exit (0);
	    }
	  set_option (option, (char *) longOptions[option_index].name, optarg);
	  break;
	case '?':
	  printf ("getopt returned unknown character code 0%o\n", opt);
	  exit (-1);
	case 'h':
	  help (argv[0], option);
	  echo_settings (option);
	  exit (0);
	  break;
	default:		/* short option */
	  n[0] = opt;
	  set_option (option, n, optarg);
	  break;
	}
    }
}

/*-------------------------------------------------------------------------*/

/* Comment out all this code if we are using the GNU C Library, and are not
   actually compiling the library itself.  This code is part of the GNU C
   Library, but also included in many other GNU distributions.  Compiling
   and linking in this code is a waste when using the GNU C library
   (especially if it is a shared library).  Rather than having every GNU
   program understand `configure --with-gnu-libc' and omit the object files,
   it is simpler to just do this in the source for each such file.  */

#if defined (_LIBC) || !defined (__GNU_LIBRARY__)

/* This needs to come after some library #include
   to get __GNU_LIBRARY__ defined.  */
   
#ifdef __GNU_LIBRARY__
#include <stdlib.h>
#else
char *getenv ();
#endif

PUBLIC int
getopt_long (argc, argv, options, long_options, opt_index)
     int argc;
     char *const *argv;
     const char *options;
     const struct option *long_options;
     int *opt_index;
{
  return _getopt_internal (argc, argv, options, long_options, opt_index, 0);
}

/*-------------------------------------------------------------------------*/

/* Like getopt_long, but '-' as well as '--' can indicate a long option.
   If an option that starts with '-' (not '--') doesn't match a long option,
   but does match a short option, it is parsed as a short option
   instead.  */

PUBLIC int
getopt_long_only (argc, argv, options, long_options, opt_index)
     int argc;
     char *const *argv;
     const char *options;
     const struct option *long_options;
     int *opt_index;
{
  return _getopt_internal (argc, argv, options, long_options, opt_index, 1);
}

#endif	/* _LIBC or not __GNU_LIBRARY__.  */

/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/

#ifdef OPTIONS_TEST

/*--------------------------------------------------------------------------
*
*			    command line options
*
*-------------------------------------------------------------------------*/

char		*InFile		= NULL;
char		*DistanceFile	= NULL;
char		*DistChoice	= "hamming";
boolean		WriteDistances  = 0;
boolean		DoPlot		= 0;
boolean		DoCoords	= 0;
boolean		Weight		= 0;

Option	option[] = {

/*  option name   type descriptor   variable       description	      */
/* (uchar name)	  (uchar type)	   (void *arg)     (char *desc)       */

  { "f",	 OPT_STRING,       &InFile,	    "input filename"},
  { "d",	 OPT_STRING,	   &DistanceFile,   "filename of distance matrix"},
  { "Distance",  OPT_STRING,	   &DistChoice,     "distance method: hamming or tree"},
  { "weight",	 OPT_SWITCH,	   &Weight,	    "weight metric matrix (switch)"},
  { "W",	 OPT_SWITCH,	   &WriteDistances, "write distances to file (switch)"},
  { "P",	 OPT_SWITCH,	   &DoPlot,	    "plot (switch)"},
  { "C",	 OPT_SWITCH,	   &DoCoords,	    "diagonalize (switch)"},
  {  0,		    0,		      0,		  0} 
};

int
main (int argc, char **argv)
{
  help (argv[0], option);
  echo_settings (option);
  command_line_args (argc, argv, option);
  echo_settings (option);
}

#endif  /* OPTIONS_TEST */

