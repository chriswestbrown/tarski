(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoC () Real)
(declare-fun skoS () Real)
(assert (and (>  (+ 2446800000000 (* skoX (+ (- 32591820000) (* skoX 156450431)))) 0) (> (* (+ 1200000000 (* skoX (+ (- 16620000) (* skoX 76729)))) (+ (* skoC (- 86400000)) (* skoS 2025130727))) 0) (<= (* (- 1) (+ (* skoC (- 86400000)) (* skoS 2025130727))) 0) (>  (+ (* skoC (- 86400000)) (* skoS 2025130727)) 0) (<= (* (- 1) skoX) 0) (<=  skoX 0) (=  (+ (+ (- 1) (* skoC skoC)) (* skoS skoS)) 0) (<=  (+ (- 75) skoX) 0)))
(check-sat)