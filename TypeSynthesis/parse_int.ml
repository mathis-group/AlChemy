(* File: parse_int.ml *)

#open "sys";;
#open "lex";;
#open "parse";;
#open "basis";;

#open "utils";;
#open "printers";;

while true do
  print_newline();
  print_endline "=============================================";
  print_string "Input Expression: ";
  try
    let s = (read_line())^"" 
    in
    if ((compare_strings s "quit") == 0) then exit(0);
    
    basis_print (make_set (make_environment s));
    
    print_lambda (parse s)
  with 
    ParseErr -> prerr_string "ParseErr"; prerr_endline ""
done
;;
