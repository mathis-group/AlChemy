#open "sys";;
#open "unix";;

value initialize_socket: int -> int = 1 "ML_initialize_socket"
  and GetString: int -> string = 1 "ML_GetString"
  and SendString: string -> unit = 1 "ML_SendString"
;;


(*

compile:

camlc -c socketML.c socketML.mli
camlc -o PROG -custom stuff socketML.o

*)
