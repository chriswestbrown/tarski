(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoC () Real)
(declare-fun skoS () Real)
(declare-fun skoX () Real)
(assert (and (<  (+ (* skoC (- 76)) (* skoS 15)) 0) (not (=  (+ (- 6000000000) (* skoX (+ 42000000 (* skoX (+ (- 147000) (* skoX 343)))))) 0)) (>  skoX 0) (=  (+ (+ (- 1) (* skoC skoC)) (* skoS skoS)) 0) (<  (+ (- 75) skoX) 0)))
(check-sat)