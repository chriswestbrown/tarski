(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun pi () Real)
(declare-fun skoY () Real)
(assert (and (<= (* (+ 1000000000000000 (* skoX skoX)) skoX skoX) 0) (<= (* (- 1) (+ (- 100) skoX)) 0) (<=  (+ (- 120) skoX) 0) (<= (* (- 1) (+ (* pi (- 1)) (* skoY 4))) 0) (<=  (+ (* pi (- 1)) (* skoY 3)) 0) (> (* (- 1) (+ (- 31415927) (* pi 10000000))) 0) (>  (+ (- 15707963) (* pi 5000000)) 0)))
(check-sat)
