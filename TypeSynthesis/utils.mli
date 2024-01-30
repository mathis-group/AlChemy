(* 
    File: utils.mli
    
    Walter Fontana, Vienna 1995 
*)

value spaces : char stream -> unit;;
value identifier : char -> int -> char stream -> string;;
value type_string : char stream -> string;;
value int_of_digit : char -> int;;
value integer : int -> char stream -> int;;
value nth : int -> 'a list -> 'a;;
value make_set : 'a list -> 'a list;;
(* value flat : 'a list list -> 'a list;; *)
