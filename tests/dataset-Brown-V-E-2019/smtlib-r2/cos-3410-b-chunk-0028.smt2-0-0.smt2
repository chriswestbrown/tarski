(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(declare-fun skoZ () Real)
(assert (>  (+ (+ (+ (- 251) (* skoX (* skoX 50))) (* skoY (* skoY 50))) (* skoZ (+ (+ (* skoX (- 100)) (* skoY (- 100))) (* skoZ 100)))) 0))
(check-sat)
