(* File: type_int.ml *)

#open "sys";;
#open "lex";;
#open "parse";;
#open "printers";;
#open "basis";;
#open "lambdatype";;
#open "type";;
#open "utils";;

let Verbose_Type = function () ->
  try 
    print_string "Enter Lambda: ";
    let l1 = read_line ()^"" 
    in   
    print_newline ();
    if (compare_strings l1 "quit" = 0) then exit (0)
    else 
      begin
	print_endline l1;
	
	(* parse *)

        let env = make_set (make_environment l1)
        in
        let pexpr = Parse env (lexer (stream_of_string l1))
	in
	print_lambda pexpr; print_newline(); print_newline();
	print_string "GLOBAL ENVIRONMENT:"; print_newline();
	basis_print env;
	print_newline(); print_newline();
	
	(* initialize typing environment *)

        reset_vartypes();

	let typing_env = generate_typing_env env
        in
	typing_env_print typing_env;
	
	(* type (and generalise) *)
	
	let result = Type typing_env pexpr
	in
	print_string "TYPE:"; print_newline();
	print_type_scheme (generalise_type (typing_env, result));
	print_newline(); print_newline();

	typing_env_print typing_env;
	print_newline();

	print_endline "********************"; print_newline ()
      end
  with 
      IllegalChar c	->  print_string "IllegalChar: "; print_char c;
			    print_newline()
    | TypingBug str	->  print_string "TypingBug: "; print_endline str
    | ParseErr		->  print_endline "ParseErr"; print_newline()
    | TypeClash (t1,t2) ->  let vars = vars_of_type(t1) @ vars_of_type(t2) 
			    in
                            print_string "*** Type clash between ";
                            print_type_scheme (Forall(vars, t1));
                            print_string " and ";
                            print_type_scheme (Forall(vars, t2));
			    print_newline()
;;

let Terse_Type = function () ->
    print_string "Enter Lambda: ";
    let l1 = read_line ()^"" 
    in   
    print_newline ();
    if (compare_strings l1 "quit" = 0) then exit (0)
    else print_string (Type_Inference l1)
;;

let Cycle_Type = function () ->
    let l1 = "((\f.\g.(f)g)(\x.x))\x.\x.x"
    in   
    print_string (Type_Inference l1)
;;

while true do
  Verbose_Type ();
  print_newline()
done
;;
