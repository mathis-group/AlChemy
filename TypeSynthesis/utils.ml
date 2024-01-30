(* 
    File: utils.ml
    
    Walter Fontana, Vienna 1995 
*)

let MAXLEN = 10;;
let STRLEN = 100;;
let buffer = make_string MAXLEN ` `;;
let string = make_string STRLEN ` `;;

(*---------------------------------------------------------------------*)
(*--- STREAM UTILITIES ------------------------------------------------*)
(*---------------------------------------------------------------------*)

(*---------------------------------------------------------------------*)
(*--- Remove blanks, tabs and newlines from stream --------------------*)
(*---------------------------------------------------------------------*)

let rec spaces = function
    [< '` `|`\t`|`\n`; spaces _ >] -> ()
  | [< >]			   -> ()
;;

(*---------------------------------------------------------------------*)
(*--- recognize an identifier -----------------------------------------*)
(*---------------------------------------------------------------------*)

let identifier first len = 
  set_nth_char buffer 0 first;
  let rec id l = function
      [< '`a`..`z` | `A`..`Z` | `0`..`9` as c; (
	  if l >= MAXLEN then 
	    id l
	  else 
	    begin
	      set_nth_char buffer l c;
	      id (succ l)
	    end
	) s >]  -> s
    | [< >]     -> (
		    match  (* remove trailing blanks *)
		      sub_string buffer 0 l 
		    with s -> s
		   )
  in id len
;;

(*---------------------------------------------------------------------*)
(*--- recognize an integer --------------------------------------------*)
(*---------------------------------------------------------------------*)

let int_of_digit = function
   `0`..`9` as c -> (int_of_char c) - (int_of_char `0`)
  | _ -> raise (Failure "not a digit")
;;

let rec integer n = function
    [< '`0`..`9` as c; (integer (10 * n + int_of_digit c)) r >] -> r
  | [< >] -> n
;;


(*---------------------------------------------------------------------*)
(*--- recognize a type token ------------------------------------------*)
(*---------------------------------------------------------------------*)

let type_string = 
  let rec sos l = function
      [< '`.`| `a`..`z` | `A`..`Z` | `0`..`9` |
	  `-` | `>` | `(` | `)` | `,` as c; (
	  if l >= STRLEN then 
	    sos l
	  else 
	    begin
	      set_nth_char string l c;
	      sos (succ l)
	    end
	) s >]  -> s
    | [< >]     -> (
		    match  (* remove trailing blanks *)
		      sub_string string 0 l 
		    with s -> s
		   )
  in sos 0
;;

(*---------------------------------------------------------------------*)
(*--- LIST UTILS ------------------------------------------------------*)
(*---------------------------------------------------------------------*)

(*---------------------------------------------------------------------*)
(*--- return the nth element of a list --------------------------------*)
(*---------------------------------------------------------------------*)

let rec nth n = function
    []     -> raise (Failure "nth")
  | x :: l -> if n = 1 then x else nth (n-1) l
;;

(*---------------------------------------------------------------------*)
(*--- remove duplicates in list ---------------------------------------*)
(*---------------------------------------------------------------------*)

let rec make_set = function
    []     -> []
  | x :: l -> if mem x l then make_set l else x :: make_set l
;;

(*---------------------------------------------------------------------*)
(*--- flatten a list of lists -----------------------------------------*)
(*---------------------------------------------------------------------*)
(*
let flat = it_list (prefix @) []
;;
*)
