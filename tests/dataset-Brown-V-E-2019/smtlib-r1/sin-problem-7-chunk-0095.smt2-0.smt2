(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(declare-fun pi () Real)
(assert (and (> (* (+ (* skoX (* skoX 20)) (* skoY (* skoY (+ (- 20) (* skoY skoY))))) skoX) 0) (> (* (+ (* skoX (* skoX 20)) (* skoY (* skoY (+ (- 20) (* skoY skoY))))) skoY) 0) (>  skoX 0) (>  (+ (- 15707963) (* pi 5000000)) 0) (> (* (- 1) (+ (- 31415927) (* pi 10000000))) 0) (>  (+ (* skoX (- 1)) skoY) 0)))
(check-sat)
