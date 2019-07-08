(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoSXY () Real)
(declare-fun skoY () Real)
(declare-fun skoX () Real)
(assert (and (>  skoSXY 0) (not (=  skoSXY 0)) (not (= (* skoSXY skoSXY) 0)) (>  (+ (- 1) skoY) 0) (>  (+ (- 3) (* skoX 2)) 0) (> (* (- 1) (+ (- 2) skoX)) 0) (> (* (- 1) (+ (- 33) (* skoY 32))) 0)))
(check-sat)