(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoS () Real)
(declare-fun skoT () Real)
(declare-fun skoA () Real)
(declare-fun skoB () Real)
(assert (and (<= (* (- 1) skoS) 0) (> (* (- 1) skoT) 0) (not (=  skoT 0)) (= (* (- 1) (+ (+ (+ (* skoS (* skoS (- 1))) (* skoT (* skoT (* skoT skoT)))) (* skoA (* skoA (* skoT skoT)))) (* skoB (* skoB (+ (* skoT skoT) (* skoA skoA)))))) 0) (>  skoA 0) (> (* (- 1) (+ (- 2) skoB)) 0) (>  (+ (* skoA (- 1)) skoB) 0)))
(check-sat)
