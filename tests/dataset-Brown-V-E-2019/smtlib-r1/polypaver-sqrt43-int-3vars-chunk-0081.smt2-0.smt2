(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoEC1 () Real)
(declare-fun skoRC1 () Real)
(declare-fun skoXC1 () Real)
(assert (> (* (- 1) (+ (* skoRC1 (* skoRC1 (+ (- 1) (* skoEC1 (+ 2 skoEC1))))) (* skoXC1 (+ 1 (* skoEC1 (+ 3 (* skoEC1 (+ 3 skoEC1)))))))) 0))
(check-sat)
