(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoZ () Real)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(assert (and (<=  (+ (- 10) (* skoZ 9)) 0) (<= (* (- 1) (+ (+ (* skoZ (- 17200)) (* skoX 2540)) (* skoY 493))) 0) (> (* (- 1) (+ (+ (* skoZ (- 17200)) (* skoX 2540)) (* skoY 493))) 0) (> (* (- 1) (+ (+ (+ (- 1000) (* skoZ (* skoZ 7200))) (* skoX (+ (* skoZ (- 2130)) (* skoX 26100)))) (* skoY (+ (+ (* skoZ (- 413)) (* skoX 10500)) (* skoY 14100))))) 0) (not (=  skoX 0))))
(check-sat)
