(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(declare-fun skoZ () Real)
(assert (>  (+ (+ (* skoX (+ (- 240) (* skoX (* skoX (+ 40 (* skoX (* skoX (- 1)))))))) (* skoY (+ (+ (- 240) (* skoX (* skoX 60))) (* skoY (+ (* skoX 60) (* skoY 40)))))) (* skoZ (+ (+ (+ (- 240) (* skoX (* skoX 60))) (* skoY (+ (* skoX 120) (* skoY 60)))) (* skoZ (+ (+ (* skoX 60) (* skoY 60)) (* skoZ 40)))))) 0))
(check-sat)