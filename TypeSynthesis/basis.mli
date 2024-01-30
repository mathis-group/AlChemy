(* 
    File: basis.mli
    
    Walter Fontana, Vienna 1995 
*)

#open "lambdatype";;

exception BadChar of char;;
exception Junk;;

value basis : (string * lambda_type_scheme) list ref;;

value scan_type_scheme : char stream -> lambda_type_scheme;;
