(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoC () Real)
(declare-fun skoS () Real)
(assert (and (<= (* (+ 3072000000000 (* skoX (+ 8832000000 (* skoX (+ 12696000 (* skoX 12167)))))) (+ 3072000000000 (* skoX (+ 8832000000 (* skoX (+ 12696000 (* skoX 12167))))))) 0) (<= (* (- 1) skoX) 0) (>  skoX 0) (=  (+ (+ (- 1) (* skoC skoC)) (* skoS skoS)) 0) (<=  (+ (- 75) skoX) 0)))
(check-sat)