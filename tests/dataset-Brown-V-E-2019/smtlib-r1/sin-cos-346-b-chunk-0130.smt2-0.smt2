(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoSQ3 () Real)
(declare-fun pi () Real)
(assert (and (> (* (+ 19958400 (* skoX (* skoX (+ (- 665280) (* skoX (* skoX (+ 11880 (* skoX (* skoX (+ (- 132) (* skoX skoX))))))))))) skoX skoX skoX skoX) 0) (>  skoX 0) (>  skoSQ3 0) (> (* (- 1) (+ (- 31415927) (* pi 10000000))) 0) (>  (+ (- 15707963) (* pi 5000000)) 0) (>  (+ (+ (- 1) (* skoX (- 10000000))) (* pi 5000000)) 0) (=  (+ (- 3) (* skoSQ3 skoSQ3)) 0)))
(check-sat)