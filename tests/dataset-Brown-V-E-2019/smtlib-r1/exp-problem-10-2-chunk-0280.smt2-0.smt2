(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoSM1 () Real)
(declare-fun skoSP1 () Real)
(declare-fun skoX () Real)
(declare-fun skoS () Real)
(assert (and (> (* (- 1) (+ (* skoSM1 (- 1)) (* skoSP1 (+ (+ 2 (* skoSM1 (* skoSM1 24))) (* skoSP1 (+ (* skoSM1 (- 60)) (* skoSP1 (+ 20 (* skoSM1 (* skoSM1 240)))))))))) 0) (>  (+ (- 1) skoX) 0) (>  skoSP1 0) (>  skoSM1 0) (>  skoS 0) (> (* (- 1) (+ (- 5) skoX)) 0)))
(check-sat)