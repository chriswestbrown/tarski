(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoA () Real)
(declare-fun skoB () Real)
(declare-fun skoT () Real)
(assert (and (>  (+ (+ (- 1) (* skoB (* skoB (* skoA skoA)))) (* skoT (* skoT (+ (+ (* skoA skoA) (* skoB skoB)) (* skoT skoT))))) 0) (>  (+ (* skoA (- 1)) skoB) 0) (> (* (- 1) (+ (- 2) skoB)) 0) (>  skoA 0) (<= (* (- 1) skoT) 0) (not (=  skoT 0))))
(check-sat)
