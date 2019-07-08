(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoZ () Real)
(declare-fun skoY () Real)
(declare-fun skoX () Real)
(assert (and (<= (* (- 1) (+ (- 1) skoZ)) 0) (<=  (+ (- 1) skoZ) 0) (>  (+ (- 1) skoZ) 0) (> (* skoZ skoZ skoY skoY) 0) (>  (+ (+ 1 (* skoY (+ 2 skoY))) (* skoX (+ (* skoY (+ (* skoZ 4) (* skoY (+ (* skoZ (+ 8 (* skoZ (- 64)))) (* skoY (* skoZ 4)))))) (* skoX (* skoY (* skoY (+ (* skoZ (* skoZ 4)) (* skoY (+ (* skoZ (* skoZ 8)) (* skoY (* skoZ (* skoZ 4)))))))))))) 0) (<= (* (- 1) (+ (- 1) skoX)) 0) (>  (+ (- 1) skoX) 0) (<= (* (- 1) (+ (- 1) skoY)) 0) (<=  (+ (- 2) skoX) 0) (<=  (+ (- 2) skoY) 0) (<=  (+ (- 2) skoZ) 0)))
(check-sat)