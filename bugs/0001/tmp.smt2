(set-info :smt-lib-version 2.6)
(set-logic QF_NRA)
(declare-fun x () Real)
(declare-fun y () Real)
(declare-fun z () Real)
(assert (let ((A (+ x (/ (- 1) 6)))) (< A (+ y (* x y))))) 
(check-sat)
(exit)
