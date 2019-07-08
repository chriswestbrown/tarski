(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(declare-fun skoZ () Real)
(assert (and (<  skoX 0) (<  skoY 0) (>  (+ (+ (* skoX (- 1)) (* skoY (- 1))) (* skoZ (+ (- 1) (* skoY skoX)))) 0) (>  (+ (+ (+ 1 (* skoX (- 1))) (* skoY (+ (- 1) (* skoX (- 1))))) (* skoZ (+ (- 1) (* skoY skoX)))) 0) (=  (+ (+ (* skoX (* skoX (* skoX (- 1)))) (* skoY (+ (* skoX (* skoX (- 3))) (* skoY (* skoX (+ (- 3) (* skoX (* skoX (- 1))))))))) (* skoZ (+ (+ (- 3) (* skoX (* skoX (- 1)))) (* skoY (* skoX (+ 3 (* skoX skoX))))))) 0) (>  skoZ 0) (>  (+ 1 skoX) 0) (>  (+ (* skoX (- 1)) skoY) 0)))
(check-sat)