(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(declare-fun pi () Real)
(assert (and (<  (+ 479001600 (* skoX (* skoX (+ (- 239500800) (* skoX (* skoX (+ 19958400 (* skoX (* skoX (+ (- 665280) (* skoX (* skoX (+ 11880 (* skoX (* skoX (+ (- 132) (* skoX skoX))))))))))))))))) 0) (>  (+ (* skoX (- 1)) skoY) 0) (>  (+ (- 1) (* skoX 10)) 0) (<  (+ (- 31415927) (* pi 10000000)) 0) (>  (+ (- 15707963) (* pi 5000000)) 0) (>  (+ (+ (- 2) (* skoY (- 10))) (* pi 5)) 0)))
(check-sat)
