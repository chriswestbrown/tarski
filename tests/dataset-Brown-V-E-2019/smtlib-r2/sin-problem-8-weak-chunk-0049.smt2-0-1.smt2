(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun pi () Real)
(declare-fun skoY () Real)
(assert (> (* (+ (+ 12 (* pi (- 6))) (* skoY (* skoY pi))) skoY) 0))
(check-sat)
