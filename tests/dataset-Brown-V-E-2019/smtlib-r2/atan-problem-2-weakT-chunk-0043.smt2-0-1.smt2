(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoA () Real)
(declare-fun skoB () Real)
(declare-fun skoS () Real)
(assert (and (<  (+ (* skoB (+ (* skoA (- 100)) (* skoB 100))) (* skoS (+ 100 (* skoB (+ (- 207) (* skoA 100)))))) 0) (=  (+ (- 1) skoB) 0) (=  (+ (+ (+ (- 1) (* skoA (* skoA (- 1)))) (* skoB (* skoB (+ (- 1) (* skoA (* skoA (- 1))))))) (* skoS skoS)) 0) (>  skoS 0) (>  skoA 0) (<  (+ (- 2) skoB) 0) (>  (+ (* skoA (- 1)) skoB) 0)))
(check-sat)
