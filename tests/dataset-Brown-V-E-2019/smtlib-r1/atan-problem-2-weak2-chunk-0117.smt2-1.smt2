(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoT () Real)
(declare-fun skoA () Real)
(declare-fun skoB () Real)
(assert (and (<= (* (- 1) skoT) 0) (>  skoA 0) (> (* (- 1) (+ (* skoT (- 1)) skoB)) 0) (<=  (+ (* skoT (- 1)) skoB) 0) (>  (+ skoA skoB) 0) (>  (+ (- 1) skoT) 0) (>  (+ (* skoA (- 1)) skoB) 0)))
(check-sat)
