(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(declare-fun skoZ () Real)
(assert (and (>  (+ (+ (* skoX (+ (- 10080) (* skoX (* skoX 1680)))) (* skoY (+ (+ (- 10080) (* skoX (* skoX 2520))) (* skoY (+ (* skoX 2520) (* skoY (+ 1680 (* skoY (* skoY (+ (- 42) (* skoY skoY))))))))))) (* skoZ (+ (+ (+ (- 10080) (* skoX (* skoX 2520))) (* skoY (+ (* skoX 5040) (* skoY 2520)))) (* skoZ (+ (+ (* skoX 2520) (* skoY 2520)) (* skoZ 1680)))))) 0) (> (* (- 1) (+ (- 3) skoX)) 0) (> (* (- 1) (+ (- 3) skoY)) 0) (> (* (- 1) (+ (- 3) skoZ)) 0) (>  (+ (- 1) (* skoX 10)) 0) (>  (+ (- 1) (* skoY 10)) 0) (>  (+ (- 1) (* skoZ 10)) 0)))
(check-sat)
