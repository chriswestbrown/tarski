(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoB () Real)
(declare-fun skoT () Real)
(declare-fun skoA () Real)
(assert (and (> (* (- 1) (+ (* skoB skoB) (* skoT skoT)) skoB) 0) (>  (+ (* skoT (+ (* skoB (* skoB (* skoB 50))) (* skoT (* skoT (* skoB 50))))) (* skoA (+ (+ (* skoB (* skoB (* skoB 157))) (* skoT (+ (* skoB (* skoB (+ (- 100) (* skoB 15)))) (* skoT (+ (* skoB 157) (* skoT (+ (- 50) (* skoB 15)))))))) (* skoA (* skoT (* skoB 50)))))) 0) (>  skoA 0) (>  (+ skoB skoA) 0) (>  (+ (- 1) skoT) 0) (> (* (- 1) (+ (* skoB (- 1)) skoA)) 0)))
(check-sat)
