(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoSQ3 () Real)
(declare-fun pi () Real)
(declare-fun skoX () Real)
(assert (and (>  skoSQ3 0) (> (* skoSQ3 skoSQ3 skoSQ3 skoSQ3 skoSQ3 skoSQ3 skoSQ3 skoSQ3) 0) (=  (+ (- 3) (* skoSQ3 skoSQ3)) 0) (> (* (- 1) (+ (+ 1 (* pi (- 5000000))) (* skoX 10000000))) 0) (>  (+ (- 15707963) (* pi 5000000)) 0) (> (* (- 1) (+ (- 31415927) (* pi 10000000))) 0) (>  skoX 0)))
(check-sat)