(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoZ () Real)
(declare-fun skoY () Real)
(declare-fun skoX () Real)
(assert (and (>  skoZ 0) (>  (+ (- 1) skoY) 0) (=  (+ (- 1) skoZ) 0) (>  (+ (- 1) skoX) 0) (<  (+ (- 2) skoZ) 0) (=  (+ (- 2) skoY) 0) (<  (+ (- 2) skoX) 0) (>  (+ (+ 1 skoY) (* skoX (+ (+ 1 (* skoY (+ (+ 1 (* skoZ (- 14))) (* skoY (* skoZ 2))))) (* skoX (* skoY (+ (* skoZ 2) (* skoY (* skoZ 2)))))))) 0)))
(check-sat)