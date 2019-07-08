(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoS2 () Real)
(declare-fun pi () Real)
(declare-fun skoSP () Real)
(declare-fun skoSM () Real)
(declare-fun skoX () Real)
(assert (and (not (=  (+ (- 2) (* skoS2 skoS2)) 0)) (>  (+ (- 15707963) (* pi 5000000)) 0) (<  (+ (- 31415927) (* pi 10000000)) 0) (=  skoSP 0) (>  skoSM 0) (=  skoS2 0) (>  skoX 0) (<  (+ (- 1) skoX) 0)))
(check-sat)