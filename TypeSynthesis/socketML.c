/* 
   interface stubs between socket.o (in C) and CaML-light
*/

#include <stdio.h>
#include "include.h"
#include "socket.h"
#include "mlvalues.h"

PRIVATE int connection = 0;

/*--------------------------------------------------------------------------*/

value ML_initialize_socket (value port)
{
  return (Val_int (initialize_socket (Int_val (port))));
}

/*--------------------------------------------------------------------------*/

value ML_GetString (value port)
{
  return (copy_string (GetData (&connection, (u_short) Int_val (port))));
}

/*--------------------------------------------------------------------------*/

value ML_SendString (value text)
{
  SendData (connection, String_val (text));
  return Val_unit;
}

/*--------------------------------------------------------------------------*/
