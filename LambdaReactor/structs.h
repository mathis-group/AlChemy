/*
    structs.h
    
    c 1994 Walter Fontana
 */
 
#ifndef	__STRUCTS_H
#define	__STRUCTS_H

#include "include.h"
#include "avl.h"
#include "lambda.h"
#include "regex.h"
#include "filter.h"

typedef struct law
  {
    char *expr;			/* interaction law (expressed in lambda) */
    float prob;			/* probability of application of law */
  }
law;

typedef struct basis
  {
    char *name;
    char *type;
  }
basis;

typedef struct state
  {
    int num_objects;
  }
state;

typedef struct parmsOrg
  {
    int num_laws;		/* number of laws to apply */
    char input_objects[100];	/* file of input objects */
  }
parmsOrg;

typedef struct record
  {
    char *object;		/* function */
    char *type;		        /* type */
    int instances;		/* copies */
    int sysid;			/* system id */
  }
REC;

typedef struct organization
  {
    AVL_TREE *Objects;		/* data structure containing the objects */
    parmsOrg Params;		/* organization specific parameters */
    state State;		/* various counters and variables */
    filter Filter;		/* interaction filters */
    law *Law;			/* interaction laws */
  }
organization;

typedef struct simulation
  {
    int systems;		/* number of systems */
    int max_objects;		/* max number of objects in reactor */
    int num_objects;		/* actual number of objects */
    int diversity;		/* number of different objects in reactor */
    int new;			/* cumulative sum of innovative reactions */
    int collisions;		/* number of collisions to perform */
    int bangs;			/* number of collisions performed */
    int typeclashes;		/* number of type clashes which occurred */
    float new_rate;		/* rate of innovations (over WINDOW) */
    float clashes_rate;		/* rate of type clashes (over WINDOW) */

    int snapshot;		/* interval between reactor dumps */
    int Reaction;		/* specifies reaction type */ 
    long seed;	                /* random seed */

    int type_check;		/* type synthesis flag */
    int type_save;		/* save type of normal form */
        
    char name[200];		/* prefix for all output files */
    FILE *logfp;		/* log file pointer */
    
    basis *TypeBasis;		/* typing basis */

    organization **Org;         /* component organizations */
  }
simulation;

#endif /* __STRUCTS_H */
