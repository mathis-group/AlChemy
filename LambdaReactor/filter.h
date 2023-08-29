/* 
    filter.h

    c 1994 Walter Fontana  
 */

#ifndef	__FILTER_H
#define	__FILTER_H

#define INITIAL	  0
#define OPERATOR  1
#define ARGUMENT  2
#define RESULT	  3

typedef struct Regex
  {
    char **text;		/* regular expression; text */
    regex_t **compiled;		/* regular expression; compiled */
    int *match;			/* 0: must not match, 1: must match */
  }
Regex;

typedef struct Fun
  {
    float copy_prob;		/* acceptance frequency of copy actions */
  }
Fun;

typedef struct filter
  {
    Regex regexp[4];		/* init = 0, op = 1, arg = 2 res = 3 */
    Fun fun;
  }
filter;

extern int syntax_sieve (char *object, Regex Filter, int strip);

#endif  /* __FILTER_H */
