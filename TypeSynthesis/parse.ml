(* 
    File: parse.ml
    
    Walter Fontana, Vienna 1995 
*)

#open "lex";;
#open "lambdatype";;
#open "basis";;
#open "utils";;

exception Unbound of string;;

(*
    Oh well. This has gotten a bit out of control, 
    due to "annotational complexities"....
    
    \a[(1).1->2->4].a 
    must be the same as
    \a[(1).1->2->4].a[(1).1->2->4], 
    but 
    \a[(1).1->2->4].a[(1).1]
    should flag an inconsistent type annotation.
    
    Of course,
    (\a[(1).1->2->4].a)a[(1).1]
    is fine, since the last a is not bound by the typed abstraction.
*)

(*----------------------------------------------------------------------*)
(*--- binding depth ----------------------------------------------------*)
(*----------------------------------------------------------------------*)
(* 
  returns index if (s,tau) = (id,ts) or s = id and tau = unknown, see above
*)

let binding_depth_1 (s,tau) rho = 
  let rec search n = function
      []             -> raise (Unbound s)
    | (id,ts,x) :: l -> if x = 1 then
                           (
			    if s = id then 
			       (
			       if tau = Forall([],Unknown) then 
				 n
			       else 
				 (
				 if tau = ts then 
				   n
				 else    (* sanity check *)
				   raise (Failure "inconsistent type annotation")
				 )
			       )
			     else
			       search (succ n) l
			    )
			 else
			   search (succ n) l
  in
  search 1 rho
;;

(* 
  returns index only if (s,tau) = (id,ts) 
*)

let binding_depth_0 (s,tau) rho = 
  let rec search n = function
      []             -> raise (Unbound s)
    | (id,ts,x) :: l -> if x = 0 then
			    (
			      if (s,tau) = (id,ts) then n else search (succ n) l
			    )
			else
			  search (succ n) l
  in
  search 1 rho
;;

(*----------------------------------------------------------------------*)
(*--- make list of unbounds --------------------------------------------*)
(*----------------------------------------------------------------------*)

let make_free_vars_list (s,tau) bnd fv =
  try
    binding_depth_1 (s,tau) bnd; fv
  with Unbound _ -> 
	let rec search_fv = function
	    []             -> (s,tau,0) :: fv
	  | (id,ts,x) :: l -> if (s,tau) = (id,ts) then fv else search_fv l
	in
	search_fv fv
;;

(*----------------------------------------------------------------------*)
(*
   like parse, but returns a list of variables which are free or
   which have a type in the basis.
*)

(*--- scan type --------------------------------------------------------*)

let scan_type = function
    [< 'TYPE s >] -> scan_type_scheme (stream_of_string s) 
  | [< >]	  -> Forall([],Unknown)
;;

(*----------------------------------------------------------------------*)

let rec scan bnd unbnd =
  let rec rest f = function
      [< (scan bnd f) fv1; (rest fv1) fv >] -> fv
    | [< >]                                 -> f
  in function
      [< 'LPAR; (scan bnd unbnd) fv1; 'RPAR; (rest fv1) fv >] -> fv
    | [< 'LAMBDA; 'IDENT id; scan_type tau; 'DOT; 
         (scan ((id,tau,1) :: bnd) unbnd) fv>] -> 
	                      if tau = Forall([],Unknown) then fv
			      else (id,tau,0) :: fv
    | [< (atom bnd unbnd) fv1; (rest fv1) fv >]	-> fv
and atom bnd unbnd = function
      [< 'IDENT id; scan_type tau >]  -> make_free_vars_list (id,tau) bnd unbnd
;;

(*----------------------------------------------------------------------*)

let make_environment s =     (* wrapper *)
  try
    scan [] [] (lexer (stream_of_string s))
  with 
      IllegalChar c -> raise (IllegalChar c)
    | Failure s	    -> print_string s; print_newline (); exit (0)
    | _		    -> raise ParseErr
;;

(*----------------------------------------------------------------------*)
(*--- lambda parser ----------------------------------------------------*)
(*----------------------------------------------------------------------*)

let rec Parse rho =
  let rec rest e1 = function
      [< (Parse rho) e2; (rest (App(e1,e2))) e >] -> e
    | [< >]                                       -> e1
  in function
      [< 'LPAR; (Parse rho) e; 'RPAR; (rest e) e2 >] -> e2
    | [< 'LAMBDA; 'IDENT id; scan_type tau; 'DOT; 
         (Parse ((id,tau,1) :: rho)) e >] ->
			if tau = Forall([],Unknown) then
			  Abs(id,e)
			else  (* scan outer environment *)
			  Abs("@"^string_of_int (binding_depth_0 (id,tau) rho),e)
    | [< (atom rho) e1; (rest e1) e2 >]	-> e2
and atom rho = function
      [< 'IDENT id; scan_type tau >] ->  
                         try  (* scan inner for name, *)
			   Var (binding_depth_1 (id,tau) rho)
                         with Unbound s -> try (* then outer for name and type *)
			                     Var (binding_depth_0 (id,tau) rho)
			                   with Unbound s -> 
			                     print_string "Unbound identifier: ";
                                             print_string s; print_newline();
                                             raise (Failure "lambda parsing")
			    | Failure s -> print_string s;
			                   print_string s; print_newline();
					   raise (Failure "lambda parsing")
;;

(*----------------------------------------------------------------------*)
(* 
     parse wrapper.

     Generates a parse tree where a variable's name is its binding depth.
     In an abstraction vars with a basis type are prefixed with "@" followed
     by an index into the environment.
*)

let parse s =
  try
    Parse (make_set (make_environment s)) (lexer (stream_of_string s))
  with 
      IllegalChar c -> raise (IllegalChar c)
    | _		    -> raise ParseErr
;;

(*----------------------------------------------------------------------*)
(*--- print ------------------------------------------------------------*)
(*----------------------------------------------------------------------*)

let rec print_lambda = function
    Var idx       -> print_string "Var "; print_int idx
  | App(t1,t2)	  -> print_string "App("; print_lambda t1; print_string ",";
		     print_lambda t2; print_string ")"
  | Abs(str,t)	  -> print_string "Abs("; print_string str; print_string ",";
		     print_lambda t; print_string ")"
;;
