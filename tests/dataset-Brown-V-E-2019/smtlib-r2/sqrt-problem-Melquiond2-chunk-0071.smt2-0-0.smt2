(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoSXY () Real)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(assert (and (<  (+ (* skoSXY 728405) (* skoX (+ (- 2048000) (* skoSXY 589824)))) 0) (=  (+ (+ (* skoSXY (* skoSXY (- 1))) skoX) skoY) 0) (>  (+ (- 1) skoY) 0) (>  (+ (- 3) (* skoX 2)) 0) (>  skoSXY 0) (<  (+ (- 2) skoX) 0) (<  (+ (- 33) (* skoY 32)) 0)))
(check-sat)