(* File: ipc.ml -- arguments: port *)

#open "sys";;
#open "basis";;
#open "type";;
#open "parse";;
#open "printers";;
#open "socketML";;

let PORT = int_of_string command_line.(1);;

(* ------------------------ SOCKET ----------------------------- *)

let SOCKET = initialize_socket PORT;;

(* ------------------------ MAIN ------------------------------- *)

let main()=
  while true do 
      let line = GetString PORT
      in
      SendString (Type_Inference line)
  done
;;

main ();;
