(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoC () Real)
(declare-fun skoS () Real)
(assert (and (>  (+ 12000000 (* skoX (+ 114000 (* skoX 361)))) 0) (>  skoX 0) (>  (+ (* skoC 235) (* skoS 42)) 0)))
(check-sat)