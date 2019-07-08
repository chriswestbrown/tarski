(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(declare-fun skoZ () Real)
(assert (and (= (* (+ (* skoY (+ (- 15) (* skoX (* skoX (- 8))))) (* skoZ 3)) skoX) 0) (>  skoX 0) (=  (+ (+ (- 75) (* skoX (* skoX (- 80)))) (* skoZ skoZ)) 0) (=  (+ (- 3) (* skoY skoY)) 0) (=  (+ (- 1) skoX) 0) (=  skoZ 0) (=  skoY 0)))
(check-sat)