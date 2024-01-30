(* 
    File: basis.ml
    
    Walter Fontana, Vienna 1995 
*)

#open "sys";;
#open "lambdatype";;
#open "printers";;
#open "utils";;

type type_token = LPAR | RPAR | ARR | VAR of int | ATOM of string;;

let Basis = [("", Forall([],Unknown))];;
let basis = ref Basis;;
basis := tl !basis;;   (* get rid of "". Any better way of doing this? *) 

(*---------------------------------------------------------------------*)
(*--- Open a file -----------------------------------------------------*)
(*---------------------------------------------------------------------*)

let open_infile filename =
  try 
    open_in filename 
  with
    Sys_error _ -> prerr_endline ("Can't Open: "^filename); exit(0)
;;

(*---------------------------------------------------------------------*)
(*--- recognize an identifier -----------------------------------------*)
(*
    Type variables are represented as numbers (up to 8 digits).
    The function is called with len = 1, since the first character 
    is scanned in lex_type_scheme. Type variables are integers,
    and atoms are alphanumeric.
*)   
    
let rec ident c = function
    [< (identifier c 1) s >] -> try let number = int_of_string s 
				    in VAR number
			        with Failure _ -> ATOM s
;;

(*---------------------------------------------------------------------*)
(*--- type-scheme lexer -----------------------------------------------*)
(*---------------------------------------------------------------------*)

let rec lex_type_scheme str =
  spaces str;
  match 
    str 
  with
    [< '`(`; spaces _ >]      -> [< 'LPAR; lex_type_scheme str >]
  | [< '`)`; spaces _ >]      -> [< 'RPAR; lex_type_scheme str >]
  | [< '`-`;'`>`;spaces _ >]  -> [< 'ARR; lex_type_scheme str >]
  | [< '`a`..`z`|`A`..`Z`|`0`..`9` as c; (ident c) tok; spaces _ >]
     	                      -> [< 'tok; lex_type_scheme str >]
  | [< 'c >]		      -> raise (BadChar c)
;;

(*---------------------------------------------------------------------*)
(*--- type-scheme parser ----------------------------------------------*)
(*---------------------------------------------------------------------*)

let rec parse_type_scheme =
  let rec rest e1 = function
      [< 'ARR; symbol e2; (rest (Arrow(e1,e2))) e >] -> e
    | [< >]					     -> e1
  in function
      [< symbol e1; (rest e1) e2 >] -> e2
and symbol = function
    [< 'VAR id >]   -> TypeVar{Index = id; Value = Unknown}
  | [< 'ATOM str >] -> Atom str
  | [< 'LPAR; parse_type_scheme e; 'RPAR >] -> e
;;

(*---------------------------------------------------------------------*)
(*--- obtain list of generic variables  -------------------------------*)
(*---------------------------------------------------------------------*)

let rec generic_vars str = 
  match 
    str
  with
     [< '`0`..`9` as c; (integer (int_of_digit c)) n; s >] -> n :: generic_vars s
   | [< '`,`; s >] -> generic_vars s
   | [< >] -> []
;;

(*---------------------------------------------------------------------*)
(*--- scan a type scheme  ---------------------------------------------*)
(*---------------------------------------------------------------------*)

let scan_type_scheme = function
    [< '`(`; generic_vars bv; '`)`; spaces _; '`.`; spaces _; s2 >] 
	      ->  let tau = parse_type_scheme (lex_type_scheme s2) 
		  in
		  let gv = try rev bv with _ -> bv
		  in
		  Forall(gv,tau)    
  | [< 'c >]  ->  raise (BadChar c)
;;

(*--------------------------------------------------------------------*)
(*--- print typing basis ---------------------------------------------*)
(*--------------------------------------------------------------------*)

let basis_print2 gamma =
  print_newline();
  map (
    function (str, x) -> print_string (str^" : ");
                         if x = Forall([],Unknown) then 
			    print_string "Unknown"
			 else
			    quick_print_type_scheme x;
		         print_newline()
      ) 
      gamma;
 print_newline()
;;

(*---------------------------------------------------------------------*)
(*--- read and generate typing basis ----------------------------------*)
(*---------------------------------------------------------------------*)

let assign_basis_types filename =
  let inputfile = open_infile filename 
  in 
  try
    while true do
      let str = input_line inputfile 
      in
      match 
	stream_of_string str 
      with
        [< '`a`..`z`|`A`..`Z`|`0`..`9` as c; (identifier c 1) id; 
           spaces _; '`:`; spaces _; scan_type_scheme ts >] 
	          -> basis := (id,ts) :: !basis
	| [< c >] -> raise Junk
    done
  with 
      End_of_file -> close_in inputfile; basis_print2 !basis
    | BadChar c	  -> print_string "BadChar "; print_char c; 
                     print_newline(); exit(2)
    | Junk        -> close_in inputfile; basis_print2 !basis
    | e		  -> raise e

;;

(*---------------------------------------------------------------------*)
(*--- search typing basis ---------------------------------------------*)
(*---------------------------------------------------------------------*)

let search_basis str = 
  try assoc str !basis with Not_found -> Forall([],Unknown)
;;

(*---------------------------------------------------------------------*)

(*
assign_basis_types command_line.(1);;
*)
