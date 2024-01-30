(* 
    File: parse.mli
    
    Walter Fontana, Vienna 1995 
*)

#open "lex";;
#open "lambdatype";;

type lambda =   Var of int
              | App of lambda * lambda
              | Abs of string * lambda
;;

(*
  e.g. \x.(x)\y.y  is
       Abs(x, App(Var 1, Abs(y, Var 1)))
*)

exception ParseErr;;

value Parse : (string * lambda_type_scheme * int) list -> lambda_token stream -> lambda;;
value parse : string -> lambda;;
value print_lambda : lambda -> unit;;
value make_environment : string -> (string * lambda_type_scheme * int) list;;
