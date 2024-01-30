(* File: int_lex.ml *)

#open "lex";;

while true do
  print_newline();
  print_endline "=============================================";
  print_string "Input Expression: ";
  try
    let gumbo = (read_line())^"" 
    in
    if ((compare_strings gumbo "quit") == 0) then exit(0);
    print_lexer_stream (lexer (stream_of_string gumbo))
  with 
    IllegalChar c -> prerr_string "IllegalChar"; prerr_char c; prerr_endline ""
done
;;
