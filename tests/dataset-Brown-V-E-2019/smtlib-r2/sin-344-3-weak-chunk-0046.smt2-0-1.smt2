(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(declare-fun skoZ () Real)
(assert (>  (+ (+ (+ (- 3) (* skoX (* skoX (* skoX 5)))) (* skoY (* skoY (* skoY 5)))) (* skoZ (* skoZ (* skoZ 5)))) 0))
(check-sat)
