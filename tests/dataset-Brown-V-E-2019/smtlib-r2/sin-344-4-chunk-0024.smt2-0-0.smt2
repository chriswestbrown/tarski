(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoW () Real)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(declare-fun skoZ () Real)
(assert (and (<  (+ (+ (+ (+ (- 6) (* skoW skoW)) (* skoX (+ (* skoW 2) skoX))) (* skoY (+ (+ (* skoW 2) (* skoX 2)) skoY))) (* skoZ (+ (+ (+ (* skoW 2) (* skoX 2)) (* skoY 2)) skoZ))) 0) (<  (+ (- 3) skoW) 0) (<  (+ (- 3) skoX) 0) (<  (+ (- 3) skoY) 0) (<  (+ (- 3) skoZ) 0) (>  (+ (- 1) (* skoW 10)) 0) (>  (+ (- 1) (* skoX 10)) 0) (>  (+ (- 1) (* skoY 10)) 0) (>  (+ (- 1) (* skoZ 10)) 0)))
(check-sat)
