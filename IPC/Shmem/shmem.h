/*  
    shmem.h
    
    c 1995 Walter Fontana, Vienna
 */
 
#ifndef	__SHMEM_H
#define	__SHMEM_H

extern int create_shared_memory ();
extern char * attach_shared_memory ();
extern void detach_shared_memory ();
extern void remove_shared_memory ();
extern char * shmem_GetService (char * text);
extern char * shmem_GetData ();
extern char * shmem_SendData (char * text, char sign);

#endif /* __SHMEM_H */
