(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoY () Real)
(declare-fun pi () Real)
(declare-fun skoX () Real)
(assert (and (>  skoY 0) (>  (+ (- 15707963) (* pi 5000000)) 0) (> (* (- 1) (+ (- 31415927) (* pi 10000000))) 0) (>  (+ (* skoY (- 2)) pi) 0) (>  skoX 0) (> (* (- 1) (+ (* skoY (- 1)) skoX)) 0) (<= (* (- 1) (+ (* skoY (- 1)) (* skoX 2000))) 0) (> (* (- 1) (+ (* skoY (- 1)) (* skoX 2000))) 0) (<=  (+ (* skoY (- 1)) (* skoX 2000)) 0)))
(check-sat)
