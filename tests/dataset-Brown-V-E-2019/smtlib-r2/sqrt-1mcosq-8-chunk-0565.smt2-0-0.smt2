(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(assert (> (* (+ 360 (* skoX (* skoX (+ (- 30) (* skoX skoX))))) (+ (- 1440) (* skoX (* skoX (+ 360 (* skoX (* skoX (+ (- 30) (* skoX skoX))))))))) 0))
(check-sat)
