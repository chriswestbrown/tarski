(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoS3 () Real)
(declare-fun skoSX () Real)
(declare-fun skoX () Real)
(assert (<  (+ (+ (* skoS3 15) (* skoSX (- 3))) (* skoX (* skoX (* skoS3 8)))) 0))
(check-sat)