(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(declare-fun skoZ () Real)
(assert (and (<  (+ (+ (- 51) (* skoX (* skoX 50))) (* skoY (+ (* skoX (- 100)) (* skoY 50)))) 0) (>  (+ 3 (* skoZ 2)) 0) (>  (+ 3 (* skoY 2)) 0) (>  (+ 3 (* skoX 2)) 0) (<  (+ (- 3) (* skoZ 2)) 0) (<  (+ (- 3) (* skoY 2)) 0) (<  (+ (- 3) (* skoX 2)) 0)))
(check-sat)