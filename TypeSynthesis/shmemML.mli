#open "sys";;
#open "unix";;

type memory;;

value create_shared_memory: unit -> unit = 1 "ML_create_shared_memory"
  and attach_shared_memory: unit -> memory = 1 "ML_attach_shared_memory"
  and GetString: unit -> string = 1 "ML_GetString"
  and SendString: string -> unit = 1 "ML_SendString"
;;


(*

compile:

camlc -c shmemML.c shmemML.mli
camlc -o PROG -custom stuff shmemML.o

*)
