(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoY () Real)
(declare-fun skoZ () Real)
(declare-fun skoX () Real)
(assert (and (=  (+ (* skoY 3) skoZ) 0) (=  skoY 0) (> (* skoY skoX) 0) (>  skoX 0) (=  (+ (+ 75 (* skoZ (* skoZ (- 1)))) (* skoX (* skoX 80))) 0) (=  (+ (- 3) (* skoY skoY)) 0) (=  (+ (- 1) skoX) 0) (=  skoZ 0)))
(check-sat)
