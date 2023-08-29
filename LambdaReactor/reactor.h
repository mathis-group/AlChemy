/*
    reactor.h
    
    c 1994 Walter Fontana
 */
 
#ifndef	__REACTOR_H
#define	__REACTOR_H

extern REC *add_to_system (int id, char *object, simulation * S, int type);
extern void pair_interactions (simulation * S, interpreter * L, int pure);
extern int which_systems (char *item, int *systems, simulation * S);
extern REC *choose_from_system (int sys, simulation * S);
extern int choose_system (simulation * S);

#endif  /* __REACTOR_H */
