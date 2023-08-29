/*
    type_client.h
    
    c 1994 Walter Fontana
 */

#ifndef	__TYPE_CLIENT_H
#define	__TYPE_CLIENT_H

extern void initialize_type_synthesis (char *machine, int port);
extern void ctl_type_synthesis (char *ctl);
extern char *type_synthesis (char *object);

#endif /* __TYPE_CLIENT_H */
