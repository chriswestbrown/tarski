(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoZ () Real)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(assert (and (>  (+ 1680 (* skoZ (+ (- 840) (* skoZ (+ 180 (* skoZ (+ (- 20) skoZ))))))) 0) (=  (+ (+ (+ (- 2) skoZ) skoX) skoY) 0) (=  (+ (- 1) skoZ) 0) (<  (+ (- 1) skoY) 0) (=  (+ (- 1) skoX) 0) (=  skoZ 0) (>  skoY 0) (>  skoX 0)))
(check-sat)
