(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(declare-fun skoZ () Real)
(assert (>  (+ (+ (+ 1 (* skoX (* skoX 3))) (* skoY (+ (* skoX 4) (* skoY (+ 3 (* skoX skoX)))))) (* skoZ (+ (+ (* skoX 6) (* skoY (+ (+ 6 (* skoX (* skoX (- 6)))) (* skoY (* skoX (- 6)))))) (* skoZ (+ 3 (* skoY (+ (* skoX (- 6)) (* skoY (* skoX (* skoX 3)))))))))) 0))
(check-sat)