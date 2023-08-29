(* File: ipcShm.ml -- arguments: *)

#open "sys";;
#open "basis";;
#open "type";;
#open "parse";;
#open "printers";;
#open "shmemML";;

(* ------------------------ MAIN ------------------------------- *)

let main()=

  create_shared_memory ();
  attach_shared_memory ();

  while true do 
      let line = GetString ()
      in
      SendString (Type_Inference line)
  done
;;

main ();;
