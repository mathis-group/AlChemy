(* 
    File: type.mli
    
    Walter Fontana, Vienna 1995 
*)

#open "parse";;
#open "lambdatype";;

exception TypeClash of lambda_type * lambda_type;;
exception TypingBug of string;;

value Type_Inference : string -> string;;
value Type : lambda_type_scheme list -> lambda -> lambda_type;;
value vars_of_type : lambda_type -> int list;;
value reset_vartypes : unit -> unit;;
value generalise_type : lambda_type_scheme list * lambda_type -> lambda_type_scheme;;
value free_list : ('b * lambda_type_scheme * int) list -> (int * lambda_type) list;;
value generate_typing_env : ('a * lambda_type_scheme * int) list -> lambda_type_scheme list;;
value unknowns_of_type : (int list * lambda_type) -> int list;;
value print_type_scheme : lambda_type_scheme -> unit;;
value write_type_scheme : lambda_type_scheme -> string;;

