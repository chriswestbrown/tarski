(set-info :smt-lib-version 2.6)
(set-logic QF_NRA)
(declare-fun x () Real)
(assert (let ((c (/ (- 1) 4))) (< x c)))
(check-sat)
(exit)
