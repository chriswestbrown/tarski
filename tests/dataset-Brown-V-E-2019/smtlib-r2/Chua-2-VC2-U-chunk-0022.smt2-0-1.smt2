(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(assert (not (=  (+ (- 750000000) (* skoX (+ 10500000 (* skoX (+ (- 73500) (* skoX 343)))))) 0)))
(check-sat)