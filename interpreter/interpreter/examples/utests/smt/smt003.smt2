(set-info :status unsat)
(set-logic QF_NRA)
(set-info :source | Simple test example.|)
(set-info :smt-lib-version 2.0)
(assert (exists ((a Real) (b Real)) (forall ((x Real)) 
	(> 
	(+ (+ (* x x) (* a x)) b)
	0))))
(checksat)
