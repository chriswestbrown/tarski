(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoZ () Real)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(assert (and (>  skoZ 0) (>  (+ (+ (- 11) (* skoX 5)) (* skoY (+ (+ 5 (* skoX (+ (+ 5 (* skoZ (- 15))) (* skoX skoZ)))) (* skoY (* skoX (+ skoZ (* skoX skoZ))))))) 0) (>  (+ (+ (+ (- 11) (* skoZ 5)) (* skoX (+ 5 (* skoZ 5)))) (* skoY (* skoX (+ (* skoZ (+ (- 15) skoZ)) (* skoX (* skoZ (+ 1 skoZ))))))) 0) (>  (+ (- 1) skoZ) 0) (>  (+ (- 1) skoY) 0) (>  (+ (- 1) skoX) 0) (<  (+ (- 2) skoZ) 0) (<  (+ (- 2) skoY) 0) (=  (+ (- 2) skoX) 0)))
(check-sat)
