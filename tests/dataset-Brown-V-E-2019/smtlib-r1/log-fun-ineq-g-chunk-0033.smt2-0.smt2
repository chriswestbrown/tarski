(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoB () Real)
(declare-fun skoA () Real)
(declare-fun skoX () Real)
(assert (and (>  skoB 0) (> (* (- 1) skoB skoB) 0) (>  skoA 0) (>  skoX 0)))
(check-sat)