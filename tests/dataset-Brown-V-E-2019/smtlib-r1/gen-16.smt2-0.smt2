(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun a () Real)
(declare-fun b () Real)
(assert (and (=  (+ (- 2) (* a a)) 0) (=  (+ (- 3) (* b b)) 0) (<  (+ (* a (- 1)) b) 0)))
(check-sat)
