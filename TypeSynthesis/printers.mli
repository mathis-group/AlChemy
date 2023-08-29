(* 
    File: printers.mli
    
    Walter Fontana, Vienna 1995 
*)

#open "lambdatype";;
#open "basis";;

value tvar_name : int -> string;;
value list_to_string: ('a -> string) -> 'a list -> string;;
value quick_print_type_scheme : lambda_type_scheme -> unit;;
value quick_write_type_scheme : lambda_type_scheme -> string;;
value basis_print : (string * lambda_type_scheme * int) list -> unit;;
value typing_env_print : lambda_type_scheme list -> unit;;
value numeric_name : int -> string;;
value alpha_name : int -> string;;

(* value print_type_lexer_stream : type_token stream -> unit;; *)
