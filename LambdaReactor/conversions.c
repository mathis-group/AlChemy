/*
    conversions.c 
       
    c 1995 Walter Fontana
 */

/* 
    developer: there's lots of repetition in here! Partly to make
    things faster by inlining, partly because the code reflects an
    experimental phase of the research.
 */
 
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include "utilities.h"
#include "lambda.h"
#include "structs.h"
#include "include.h"

#define MAX	500
#define NAMELEN 10

PUBLIC char * fast_standardize (char *expression, interpreter *Interp);
PUBLIC char * annotate (char *expression, basis *Basis);
PUBLIC char * mini_annotate (char *expression, basis * Basis);
PUBLIC char * minimize_annotation (char *expression);
PUBLIC char * strip_annotation (char *expression);
PUBLIC char * expand_annotation (char *expression);

PRIVATE char * _annotate (char *expr, int *v, int ids, basis *Basis);
PRIVATE char * _rename (char *expr, int *bv, int id, char format);
PRIVATE char * _minimize_annotation (char *expr, int *v, int ids);
PRIVATE char * _strip_annotation (char *expr, int *v, int ids);
PRIVATE char * _expand_annotate (char *expr, int *v, int ids);
PRIVATE char * identifier (char *expr, char *name);
PRIVATE char * basis_type (basis *Basis, char *name);
PRIVATE char * write_type (char *expr);
PRIVATE void write_buffer (char *text);

/*---------------------------------------------------------------------*/      

typedef struct 
  {
    char name[NAMELEN];
    char new_name[NAMELEN];
    char *type;
  }
ident;
  
PRIVATE ident IDENTIFIER[MAX];
PRIVATE int ID;
PRIVATE int BP;
PRIVATE boolean ESSENTIAL;
PRIVATE char buffer[BUFSIZE];
PRIVATE char ROOT;
PRIVATE jmp_buf RECOVER;

/*---------------------------------------------------------------------*/      
/*---------------------------------------------------------------------*/      

PUBLIC char *
fast_standardize (char *expression, interpreter * Interp)
{
  char *result, *Copy;
  
  if (!expression || strlen (expression) >= (BUFSIZE-500))
    return NULL;

  Copy = expression;
  ROOT = Interp->parms->standard_variable;
  ID = BP = 0;

  if (setjmp (RECOVER))
    return NULL;

  _rename (Copy, NULL, 0, 'O');

  buffer[BP] = '\0';
  result = (char *) space (BP + 1);
  strcpy (result, buffer);

  return result;
}

/*---------------------------------------------------------------------*/      

PUBLIC char *
annotate (char *expression, basis * Basis)
{
  char *result, *Copy;
  
  if (!Basis) 
    return NULL;
    
  if (!expression || strlen (expression) >= (BUFSIZE-500))
    return NULL;

  ESSENTIAL = FALSE;
  Copy = expression;
  ID = BP = 0;

  if (setjmp (RECOVER))
    return NULL;

  _annotate (Copy, NULL, 0, Basis);

  buffer[BP] = '\0';
  result = (char *) space (BP + 1);
  strcpy (result, buffer);

  return result;
}

/*---------------------------------------------------------------------*/      

PUBLIC char *
mini_annotate (char *expression, basis * Basis)
{
  char *result, *Copy;
  
  if (!Basis) 
    return NULL;
    
  if (!expression || strlen (expression) >= (BUFSIZE-500))
    return NULL;
  
  ESSENTIAL = TRUE;
  Copy = expression;
  ID = BP = 0;

  if (setjmp (RECOVER))
    return NULL;

  _annotate (Copy, NULL, 0, Basis);

  buffer[BP] = '\0';
  result = (char *) space (BP + 1);
  strcpy (result, buffer);

  return result;
}

/*---------------------------------------------------------------------*/      

PUBLIC char *
expand_annotation (char *expression)
{
  char *result, *Copy;
  
  if (!expression || strlen (expression) >= (BUFSIZE-500))
    return NULL;
  
  Copy = expression;
  ID = BP = 0;

  if (setjmp (RECOVER))
    return NULL;

  _expand_annotate (Copy, NULL, 0);

  buffer[BP] = '\0';
  result = (char *) space (BP + 1);
  strcpy (result, buffer);

  return result;
}

/*---------------------------------------------------------------------*/      

PUBLIC char *
minimize_annotation (char *expression)
{
  char *result, *Copy;
  
  if (!expression)
    return NULL;

  Copy = expression;
  ID = BP = 0;

  if (setjmp (RECOVER))
    return NULL;

  _minimize_annotation (Copy, NULL, 0);

  buffer[BP] = '\0';
  result = (char *) space (BP + 1);
  strcpy (result, buffer);

  return result;
}

/*---------------------------------------------------------------------*/      

PUBLIC char *
strip_annotation (char *expression)
{
  char *result, *Copy;
  
  if (!expression)
    return NULL;

  Copy = expression;
  ID = BP = 0;

  if (setjmp (RECOVER))
    return NULL;

  _strip_annotation (Copy, NULL, 0);

  buffer[BP] = '\0';
  result = (char *) space (BP + 1);
  strcpy (result, buffer);

  return result;
}

/*---------------------------------------------------------------------*/      
/*---------------------------------------------------------------------*/      

PRIVATE char *
_rename (char *expr, int *v, int idents, char format)
{
  /*
      format:  suppose the input term is \a.(\a.\b.(\c.(a)b)\d.c)a
       then 

       'I' .... "name-independent" turns it into
      
       \@.(\@.\@.(\@.(3)2)\@.c)1
	
       'D' .... "depth binding" turns it into
      
       \x1.(\x2.\x3.(\x4.(3)2)\x5.c)1
       
       'O' .... "order binding" turns it into
       
       \x1.(\x2.\x3.(\x4.(x2)x3)\x5.c)x1
  */
  
  register int l, i;
  int bound[MAX];
  char name[NAMELEN], work[10];
  
  if (!v) 
    l = 0;
  else
    for (l = 1; l <= idents; l++) bound[l] = v[l];
  
  l = idents;
  
  while (*expr)
    {
      switch (*expr)
	{
	case '\\':
	  expr++;
	  expr = identifier (expr, name);
	  strcpy (IDENTIFIER[++ID].name, name);
	  bound[++l] = ID;
	  
	  /* rename */
	  
	  switch (format)
	    {
	    case 'O': case 'D':
	      IDENTIFIER[ID].new_name[0] = ROOT;
	      sprintf (IDENTIFIER[ID].new_name + 1, "%d", ID);
	      break;
	      
	    case 'I':
	      strcpy (IDENTIFIER[ID].new_name, "@");
	      break;
	    }
	    
	  write_buffer ("\\");
	  write_buffer (IDENTIFIER[ID].new_name);
	  
	  break;
	  
	case '.':
	  write_buffer (".");
	  expr++;
	  break;
	  
	case '(':
	  write_buffer ("(");
	  expr++;
	  expr = _rename (expr, bound, l, format);
	  break;
	  
	case ')':
	  write_buffer (")");
	  expr++;
	  return expr;

	case '[':
	  expr = write_type (expr);
	  break;
	  
	default:
	  if (isalnum (*expr) || *expr == '$')
	    {
	      expr = identifier (expr, name);
	      for (i = l; i; i--)
		{
		  if (strcmp (name, IDENTIFIER[bound[i]].name) == 0)
		    {
		      switch (format)        /* bound identifier */
			{
			case 'D': case 'I':
			
			  /* binding depth or name independent */
			  
			  sprintf (work, "%d", l-i+1);
			  write_buffer (work);
			  goto quit;
			  break;
			  
			case 'O': 
			
			  /* binding order */
			  
			  write_buffer (IDENTIFIER[bound[i]].new_name);
			  goto quit;
			  break;
			}
		    }
		}
	      quit:
	      if (i == 0)
		{
		  write_buffer (name);
		}
	    }
	  else
	    {
	      printf ("%s\n", expr);
	      nrerror ("parse error in _rename ()");
	    }
	}
    }
  return expr;
}
      
/*---------------------------------------------------------------------*/      

PRIVATE char *
_annotate (char *expr, int *v, int idents, basis *Basis)
{
  /*
      Annotate variables in a term, if they have a basis type.
      For the type checker it is sufficient to annotate
      the abstractions and the free variables. This is
      done when the ESSENTIAL flag is set, otherwise all variables
      with a type in the basis are annotated.
  */
  
  register int l, i;
  int bound[MAX];
  char name[NAMELEN], *type;
  
  if (!v) 
    l = 0;
  else
    for (l = 1; l <= idents; l++) bound[l] = v[l];
  
  l = idents;
  
  while (*expr)
    {
      switch (*expr)
	{
	case '\\':
	  expr++;
	  expr = identifier (expr, name);
	  strcpy (IDENTIFIER[++ID].name, name);
	  bound[++l] = ID;

	  write_buffer ("\\");
	  write_buffer (IDENTIFIER[ID].name);
	  
	  if (*expr == '[')  /* do not annotate twice */
	    break;

	  if (type = basis_type (Basis, name))
	    {
	      write_buffer ("[");
	      write_buffer (type);
	      write_buffer ("]");
	    }
	  break;
	  
	case '.':
	  write_buffer (".");
	  expr++;
	  break;
	  
	case '(':
	  write_buffer ("(");
	  expr++;
	  expr = _annotate (expr, bound, l, Basis);
	  break;
	  
	case ')':
	  write_buffer (")");
	  expr++;
	  return expr;
	
	case '[':
	  expr = write_type (expr);
	  break;
	  
	default:
	  if (isalnum (*expr) || *expr == '$')
	    {
	      expr = identifier (expr, name);
	      write_buffer (name);
	      
	      if (*expr == '[')  /* do not annotate twice */
		break;
	      
	      if (ESSENTIAL)
		{
		  for (i = l; i; i--)
		    if (strcmp (name, IDENTIFIER[bound[i]].name) == 0)
		      goto quit;
		}
		
	      if (type = basis_type (Basis, name))
		{
		  write_buffer ("[");
		  write_buffer ( type);
		  write_buffer ("]");
		}
	    }
	  else
	    {
	      printf ("%s\n", expr);
	      nrerror ("parse error in annotate");
	    }
	  quit:
	  break;
	}
    }
  return expr;
}
      
/*---------------------------------------------------------------------*/      

PRIVATE char *
_expand_annotate (char *expr, int *v, int idents)
{
  /*
      Annotate variables in a term, following the
      annotations in the abstractions and the free variables.
  */
  
  register int l, i;
  int bound[MAX];
  char name[NAMELEN], *type;
  
  if (!v) 
    l = 0;
  else
    for (l = 1; l <= idents; l++) bound[l] = v[l];
  
  l = idents;
  
  while (*expr)
    {
      switch (*expr)
	{
	case '\\':
	  expr++;
	  expr = identifier (expr, name);
	  strcpy (IDENTIFIER[++ID].name, name);
	  bound[++l] = ID;

	  write_buffer ("\\");
	  write_buffer (IDENTIFIER[ID].name);
	  
	  if (*expr == '[')
	    IDENTIFIER[ID].type = expr; /* set a mark */
	  else
	    IDENTIFIER[ID].type = NULL;
	  
	  break;
	  
	case '.':
	  write_buffer (".");
	  expr++;
	  break;
	  
	case '(':
	  write_buffer ("(");
	  expr++;
	  expr = _expand_annotate (expr, bound, l);
	  break;
	  
	case ')':
	  write_buffer (")");
	  expr++;
	  return expr;
	
	case '[':
	  expr = write_type (expr);
	  break;
	  
	default:
	  if (isalnum (*expr) || *expr == '$')
	    {
	      expr = identifier (expr, name);
	      write_buffer (name);
	      
	      if (*expr == '[')  /* do not annotate twice */
		break;
	      
	      for (i = l; i; i--)
		{
		  if (strcmp (name, IDENTIFIER[bound[i]].name) == 0)
		    {
		      type = IDENTIFIER[bound[i]].type;
		      if (!type) 
			break;
		      write_type (type);
		      break;
		    }
		}
	    }
	  else
	    {
	      printf ("%s\n", expr);
	      nrerror ("parse error in expand_annotate");
	    }
	  break;
	}
    }
  return expr;
}
      
/*---------------------------------------------------------------------*/      

PRIVATE char *
_minimize_annotation (char *expr, int *v, int idents)
{
  /*
      Leave only annotations of abstractions and free variables.
  */
  
  register int l, i;
  int bound[MAX];
  char name[NAMELEN];
  
  if (!v) 
    l = 0;
  else
    for (l = 1; l <= idents; l++) bound[l] = v[l];
  
  l = idents;
  
  while (*expr)
    {
      switch (*expr)
	{
	case '\\':
	  expr++;
	  expr = identifier (expr, name);
	  strcpy (IDENTIFIER[++ID].name, name);
	  bound[++l] = ID;

	  write_buffer ("\\");
	  write_buffer (IDENTIFIER[ID].name);
	  break;
	  
	case '.':
	  write_buffer (".");
	  expr++;
	  break;
	  
	case '(':
	  write_buffer ("(");
	  expr++;
	  expr = _minimize_annotation (expr, bound, l);
	  break;
	  
	case ')':
	  write_buffer (")");
	  expr++;
	  return expr;
	
	case '[':
	  expr = write_type (expr);
	  break;
	  
	default:
	  if (isalnum (*expr) || *expr == '$')
	    {
	      expr = identifier (expr, name);
	      write_buffer (name);
	      
	      for (i = l; i; i--)
		{
		  if (strcmp (name, IDENTIFIER[bound[i]].name) == 0)
		    {
		      if (*expr == '[')  /* remove annotation */
			{
			  while (*expr != ']') expr++;
			  expr++;
			}
		      break;
		    }
		}
	    }
	  else
	    {
	      printf ("%s\n", expr);
	      nrerror ("parse error in minimize_annotation");
	    }
	}
    }
  return expr;
}
      
/*---------------------------------------------------------------------*/      

PRIVATE char *
_strip_annotation (char *expr, int *v, int idents)
{
  register int l;
  int bound[MAX];
  char name[NAMELEN];
  
  if (!v) 
    l = 0;
  else
    for (l = 1; l <= idents; l++) bound[l] = v[l];
  
  l = idents;
  
  while (*expr)
    {
      switch (*expr)
	{
	case '\\':
	  expr++;
	  expr = identifier (expr, name);
	  strcpy (IDENTIFIER[++ID].name, name);
	  bound[++l] = ID;

	  write_buffer ("\\");
	  write_buffer (IDENTIFIER[ID].name);
	  break;
	  
	case '.':
	  write_buffer (".");
	  expr++;
	  break;
	  
	case '(':
	  write_buffer ("(");
	  expr++;
	  expr = _strip_annotation (expr, bound, l);
	  break;
	  
	case ')':
	  write_buffer (")");
	  expr++;
	  return expr;
	
	case '[':
	  while (*expr != ']') expr++;
	  expr++;
	  break;
	  
	default:
	  if (isalnum (*expr) || *expr == '$')
	    {
	      expr = identifier (expr, name);
	      write_buffer (name);
	    }
	  else
	    {
	      printf ("%s\n", expr);
	      nrerror ("parse error in strip_annotation");
	    }
	}
    }
  return expr;
}
      
/*---------------------------------------------------------------------*/      

PRIVATE char *
identifier (char *expr, char *name)  /* scan alphanumeric identifier */
{
  register int l;
  
  l = 0;
  while (isalnum (*expr) || *expr == '$') name[l++] = *expr++;
  name[l] = '\0';
  return expr;
}

/*---------------------------------------------------------------------*/      
      
PRIVATE char *
basis_type (basis *Basis, char *name)
{
  register int l;
  
  l = -1;
  while (Basis[++l].name)
    {
      if (strcmp (Basis[l].name, name) == 0)
	return Basis[l].type;
    }
  return NULL;
}

/*---------------------------------------------------------------------*/      

PRIVATE void
write_buffer (char *text)
{
  register int i;
  
  for (i = 0; text[i]; i++) 
    buffer[BP++] = text[i];
  
  if (BP >= (BUFSIZE-10))
    longjmp (RECOVER, 1);
}

/*---------------------------------------------------------------------*/      

PRIVATE char *
write_type (char *expr)
{
  while (*expr != ']')
    buffer[BP++] = *expr++;
  buffer[BP++] = *expr++;
  
  return (expr);
}

/*---------------------------------------------------------------------*/      
