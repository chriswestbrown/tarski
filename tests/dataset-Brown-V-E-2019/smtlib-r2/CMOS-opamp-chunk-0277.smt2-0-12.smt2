(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(declare-fun pi () Real)
(assert (and (= (* (+ 1000000000000000 (* skoX skoX)) (+ (- 2) (* skoY skoY)) skoX) 0) (=  (+ (+ 55289999999999882000000000000057 (* skoX (* skoX (+ 205193160000000000000000000 (* skoX (* skoX (- 6839943))))))) (* skoY (* skoY (* skoX (* skoX (+ 3420000000000000000000 (* skoX (* skoX 3420000)))))))) 0) (>  (+ (- 100) skoX) 0) (=  (+ (- 120) skoX) 0) (<  (+ (* skoY (- 4)) pi) 0) (=  (+ (* skoY (- 3)) pi) 0) (<  (+ (- 31415927) (* pi 10000000)) 0) (>  (+ (- 15707963) (* pi 5000000)) 0)))
(check-sat)
