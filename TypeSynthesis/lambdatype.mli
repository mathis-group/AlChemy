(* 
    File: lambdatype.mli
    
    Walter Fontana, Vienna 1995 
*)

type lambda_type =   Unknown
                   | Atom of string
                   | TypeVar of vartype
                   | Arrow of lambda_type * lambda_type

and vartype = {Index:int; mutable Value:lambda_type}
and lambda_type_scheme = Forall of int list * lambda_type;;
