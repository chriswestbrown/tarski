(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(assert (not (=  (+ 6000000000000 (* skoX (+ 17400000000 (* skoX (+ 25230000 (* skoX 24389)))))) 0)))
(check-sat)