(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoC () Real)
(declare-fun skoS () Real)
(declare-fun skoX () Real)
(assert (and (>  (+ (* skoC (- 400)) (* skoS 441)) 0) (=  skoX 0) (=  (+ (+ (- 1) (* skoC skoC)) (* skoS skoS)) 0) (>  skoS 0) (=  skoC 0)))
(check-sat)