(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoEC1 () Real)
(declare-fun skoRC1 () Real)
(declare-fun skoXC1 () Real)
(assert (and (=  (+ (* skoRC1 (* skoRC1 (+ (- 1) (* skoEC1 (+ 2 skoEC1))))) (* skoXC1 (+ 1 (* skoEC1 (+ 3 (* skoEC1 (+ 3 skoEC1))))))) 0) (=  (+ (+ 4 (* skoRC1 (- 4))) (* skoXC1 skoXC1)) 0) (>  (+ (* skoRC1 4) (* skoXC1 (+ (- 4) skoXC1))) 0) (=  (+ (- 1) (* skoEC1 32)) 0) (=  (+ 1 (* skoEC1 32)) 0) (=  (+ (- 2) skoXC1) 0) (=  (+ (- 3) skoRC1) 0) (>  (+ (- 1) (* skoXC1 2)) 0) (>  skoRC1 0)))
(check-sat)