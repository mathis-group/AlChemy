(* 
    File: lex.ml
    
    Walter Fontana, Vienna 1995 
*)

#open "utils";;

(*---------------------------------------------------------------------*)
(*--- recognize an identifier -----------------------------------------*)
(*---------------------------------------------------------------------*)

let ident c = function
    [< (identifier c 1) s >] -> IDENT s
;;

(*---------------------------------------------------------------------*)
(*--- recognize a type annotation -------------------------------------*)
(*---------------------------------------------------------------------*)

let type_token = function
    [< type_string s >] -> TYPE s
;;

(*---------------------------------------------------------------------*)
(*--- lambda lexer ----------------------------------------------------*)
(* 
    variables have alpha numeric names 
*)

let rec lexer str = 
  spaces str;
  match 
    str 
  with
      [< '`(`; spaces _ >]  -> [< 'LPAR; lexer str >]
    | [< '`)`; spaces _ >]  -> [< 'RPAR; lexer str >]
    | [< '`\\`; spaces _ >] -> [< 'LAMBDA; lexer str >]
    | [< '`a`..`z`|`A`..`Z`|`0`..`9` as c; (ident c) tok; spaces _ >]
                            -> [< 'tok; lexer str >]
    | [< '`[`; type_token tok; '`]`; spaces _ >]  
                            -> [< 'tok; lexer str >]
    | [< '`.`; spaces _ >]  -> [< 'DOT; lexer str >]
    | [< 'c >]		    -> raise (IllegalChar c)
;;     

(*---------------------------------------------------------------------*)
(*--- print -----------------------------------------------------------*)
(*---------------------------------------------------------------------*)

let print_token = function
    LPAR      -> print_string "LPAR "
  | RPAR      -> print_string "RPAR "
  | LAMBDA    -> print_string "LAMBDA "
  | DOT	      -> print_string "DOT "
  | IDENT str -> print_string "IDENT "; print_string str; print_string " "
  | TYPE str  -> print_string "TYPE "; print_string str; print_string " "
;;

let rec print_lexer_stream = function
    [< 'c; s >]	-> print_token c; print_lexer_stream s
  | [< >]	-> print_newline ()
;;
