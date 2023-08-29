/*
    io.h
    
    c 1994 Walter Fontana
 */
 
#ifndef	__IO_H
#define	__IO_H

extern void set_parameters (char *fn, simulation ** S_,
			    parmsLambda ** Lambda_parameters_,
			    parmsRandExpr ** RandExpr_parameters_);
extern void change_parameters (char *fn, simulation ** S_,
			       parmsLambda ** Lambda_parameters_,
			       parmsRandExpr ** RandExpr_parameters_);
extern void print_parameters (FILE *fp, simulation * S,
			      parmsLambda * Lambda_parameters);
extern void get_expressions (simulation * S, parmsRandExpr * RandExpr, 
			     interpreter * Lambda);
extern void print_reactor (char *fname, simulation * S);
extern void read_basis (char *fname, simulation * S);

#endif  /* __IO_H */

