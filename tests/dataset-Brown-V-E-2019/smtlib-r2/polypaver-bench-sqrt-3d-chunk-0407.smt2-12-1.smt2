(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(declare-fun skoZ () Real)
(assert (> (* (+ (- 3) (* skoZ (+ (+ 5 (* skoY (* skoX (- 7)))) (* skoZ (* skoY skoX))))) skoX) 0))
(check-sat)
