(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoY () Real)
(declare-fun pi () Real)
(declare-fun skoX () Real)
(assert (and (<  (+ (- 720) (* skoY (* skoY (+ 360 (* skoY (* skoY (+ (- 30) (* skoY skoY)))))))) 0) (=  (+ (- 2) (* skoY skoY)) 0) (>  (+ (- 15707963) (* pi 5000000)) 0) (<  (+ (- 31415927) (* pi 10000000)) 0) (>  (+ (* skoY (- 2)) pi) 0) (=  (+ (- 1) (* skoX 20)) 0) (<  (+ (* skoY (- 1)) skoX) 0)))
(check-sat)
