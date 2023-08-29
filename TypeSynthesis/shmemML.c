/* 
   interface stubs between shmem.o (in C) and CaML-light
*/

#include <stdio.h>
#include "shmem.h"
#include "mlvalues.h"

/*--------------------------------------------------------------------------*/

value ML_create_shared_memory (value unit)
{
  create_shared_memory();
  return Val_unit;
}

/*--------------------------------------------------------------------------*/

value ML_attach_shared_memory (value unit)
{
  return ((value) attach_shared_memory());
}

/*--------------------------------------------------------------------------*/

value ML_GetString (value unit)
{
  return (copy_string (shmem_GetData ()));
}

/*--------------------------------------------------------------------------*/

value ML_SendString (value text)
{
  shmem_SendData (String_val (text), '!');
  return Val_unit;
}

/*--------------------------------------------------------------------------*/
