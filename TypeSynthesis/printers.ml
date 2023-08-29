(* 
    File: printers.ml
    
    Walter Fontana, Vienna 1995 
*)

#open "lambdatype";;
#open "type";;
#open "basis";;

(*--------------------------------------------------------------------*)
(*--- name for type variable -----------------------------------------*)
(*--------------------------------------------------------------------*)

(*--- compute an numeric name for a type variable with id n ----------*)

let numeric_name = function n -> string_of_int n
;;

(*--- compute an alphabetic name for a type variable with id n -------*)
(*
    Computes a name "'a", ... for type variables, given an integer n 
    representing the position of the type variable in the list of generic
    type variables
*)

let alpha_name n =
  let rec name_of n =
    let q, r = (n / 26), (n mod 26) 
    in
    let s = make_string 1 (char_of_int (96 + r)) 
    in
    if q = 0 then s else (name_of q)^s
  in "'"^(name_of n)
;;

(*--- name choice ----------------------------------------------------*)

let tvar_name n = numeric_name n
;;

(*--------------------------------------------------------------------*)
(*--- print a list to string -----------------------------------------*)
(*--------------------------------------------------------------------*)

let rec list_to_string conversion = function
    []      -> ""
  | n :: r  -> if r = [] then (conversion n)^""
	       else (conversion n)^","^(list_to_string conversion r)
;;

(* e.g conversion could be string_of_int *)

(*--------------------------------------------------------------------*)
(*--- print a type scheme --------------------------------------------*)
(*--------------------------------------------------------------------*)
(*
    print generic variables with numeric names and unknowns
    with alpha names
*)

let quick_print_type_scheme (Forall(gv,t)) =
  let names = (names_of (1,gv)
    where rec names_of = function
	(n,[])         -> []
      | (n,(v1 :: Lv)) -> (tvar_name n) :: (names_of (n+1, Lv))) 
  in
  let tvar_names = combine (rev gv,names) 
  in
  let rec print_rec = function
      TypeVar{Index = n; Value = Unknown} 
		      -> let name = try assoc n tvar_names
				    with Not_found -> alpha_name n
			 in print_string name
    | TypeVar{Index = _; Value = t} -> print_rec t
    | Arrow(t1,t2)    -> print_string "("; print_rec t1;
			 print_string "->"; print_rec t2;
			 print_string ")"
    | Atom str        -> print_string ("atom "^str)
    | Unknown         -> raise (TypingBug "quick_print_type_scheme")
  in
  print_rec t
;;

(*--- write version --------------------------------------------------*)

let quick_write_type_scheme (Forall(gv,t)) =
  let names = (names_of (1,gv)
    where rec names_of = function
	(n,[])         -> []
      | (n,(v1 :: Lv)) -> (tvar_name n) :: (names_of (n+1, Lv))) 
  in
  let tvar_names = combine (rev gv,names) 
  in
  let rec write_rec = function
      TypeVar{Index = n; Value = Unknown} 
		      -> (try assoc n tvar_names
			 with Not_found -> alpha_name n)
    | TypeVar{Index = _; Value = t} -> write_rec t
    | Arrow(t1,t2)    -> "("^(write_rec t1)^"->"^(write_rec t2)^")"
    | Atom str        -> "atom "^str
    | Unknown         -> raise (TypingBug "quick_write_type_scheme")
  in
  write_rec t
;;

(*--------------------------------------------------------------------*)
(*--- print basis ----------------------------------------------------*)
(*--------------------------------------------------------------------*)

let basis_print gamma =
  print_newline();
  map (
    function (str, x, i) -> print_string (str^" : ");
                            if x = Forall([],Unknown) then 
			      print_string "Unknown"
			    else
			      quick_print_type_scheme x;
		            print_newline()
      ) 
      gamma;
 print_newline()
;;

(*--------------------------------------------------------------------*)
(*--- print typing environment ---------------------------------------*)
(*--------------------------------------------------------------------*)

let typing_env_print gamma =
  print_string "TYPING ENVIRONMENT:"; print_newline(); print_newline();
  map (function x -> quick_print_type_scheme x; print_newline()) gamma;
  print_newline()
;;

(*---------------------------------------------------------------------*)
(*--- print type-lexer output -----------------------------------------*)
(*---------------------------------------------------------------------*)
(*

type type_token = LPAR | RPAR | ARR | VAR of int | ATOM of string;;

let rec print_type_lexer_stream str =
  match 
    str 
  with
      [< 'LPAR >]    -> prerr_string "LPAR "; print_type_lexer_stream str
    | [< 'RPAR >]    -> prerr_string "RPAR "; print_type_lexer_stream str
    | [< 'ARR >]     -> prerr_string "ARR "; print_type_lexer_stream str
    | [< 'VAR id >]  -> prerr_string "VAR "; prerr_int id; prerr_char ` `; 
                        print_type_lexer_stream str
    | [< 'ATOM s >]  -> prerr_string ("ATOM "^s); print_type_lexer_stream str
    | [< >]	     -> prerr_endline ""
;;

*)
