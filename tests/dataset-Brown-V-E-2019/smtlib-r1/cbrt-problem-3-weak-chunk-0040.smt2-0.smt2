(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoY () Real)
(declare-fun skoZ () Real)
(declare-fun skoX () Real)
(assert (and (<= (* (+ 1 (* skoY 2)) (+ 1 (* skoZ 2))) 0) (<=  (+ (+ (+ 1 (* skoY (+ (- 2) skoY))) (* skoZ (+ (+ (- 2) (* skoY (+ (- 12) (* skoY 2)))) (* skoZ (+ 1 (* skoY 2)))))) (* skoX (+ (+ (+ (- 2) (* skoY (+ (- 12) (* skoY 2)))) (* skoZ (+ (+ (- 12) (* skoY (+ (- 40) (* skoY 4)))) (* skoZ (+ 2 (* skoY 4)))))) (* skoX (+ (+ 1 (* skoY 2)) (* skoZ (+ 2 (* skoY 4)))))))) 0) (>  (+ (- 1) (* skoZ 20)) 0) (>  (+ (- 1) (* skoY 20)) 0) (>  (+ (- 1) (* skoX 20)) 0) (> (* (- 1) (+ (- 15) skoZ)) 0) (> (* (- 1) (+ (- 15) skoY)) 0) (> (* (- 1) (+ (- 15) skoX)) 0)))
(check-sat)