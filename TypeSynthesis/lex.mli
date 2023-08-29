(* 
    File: lex.mli
    
    Walter Fontana, Vienna 1995 
*)

type lambda_token =   LAMBDA
		    | DOT
		    | LPAR
		    | RPAR
		    | IDENT of string
		    | TYPE of string
;;

exception IllegalChar of char;;

value lexer : char stream -> lambda_token stream;;
value print_lexer_stream : lambda_token stream -> unit;;

