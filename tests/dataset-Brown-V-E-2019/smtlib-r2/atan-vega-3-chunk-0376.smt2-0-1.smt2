(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(declare-fun skoZ () Real)
(assert (< (* (+ (+ 3 (* skoY (+ (* skoX (- 4)) (* skoY (- 1))))) (* skoZ (* skoY (+ (- 1) (* skoY skoX))))) (+ (+ (* skoX (- 1)) (* skoY (* skoY (* skoX (- 1))))) (* skoZ (+ (- 1) (* skoY skoX))))) 0))
(check-sat)