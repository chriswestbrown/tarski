(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(declare-fun pi () Real)
(assert (and (<= (* (- 1) (+ (* skoX (- 2000)) skoY)) 0) (> (* (- 1) (+ (* skoX (- 2000)) skoY)) 0) (<=  (+ (* skoX (- 2000)) skoY) 0) (>  (+ (- 15707963) (* pi 5000000)) 0) (> (* (- 1) (+ (- 31415927) (* pi 10000000))) 0) (>  (+ (* skoY (- 2)) pi) 0) (>  skoX 0) (>  (+ (* skoX (- 1)) skoY) 0) (> (* (- 1) (+ (* skoX (* skoX (* skoX (- 1000)))) (* skoY 3))) 0)))
(check-sat)
