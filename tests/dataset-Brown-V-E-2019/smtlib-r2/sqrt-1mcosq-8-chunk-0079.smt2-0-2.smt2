(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun pi () Real)
(assert (<  (+ (- 31415927) (* pi 10000000)) 0))
(check-sat)