(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun pi () Real)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(assert (>  (+ (+ (+ 120 (* pi (- 60))) (* skoX (* skoX (+ (- 20) (* skoX skoX))))) (* skoY (* skoY (* pi 10)))) 0))
(check-sat)