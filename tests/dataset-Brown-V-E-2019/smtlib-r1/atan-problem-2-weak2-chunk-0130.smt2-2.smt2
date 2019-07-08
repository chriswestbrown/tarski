(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoA () Real)
(declare-fun skoT () Real)
(declare-fun skoB () Real)
(assert (and (<=  skoA 0) (> (* (- 1) (+ skoA skoT)) 0) (>  (+ (* skoT (- 1)) skoB) 0) (>  skoT 0) (> (* (- 1) (+ (* skoT (* skoT (* skoT (* skoT (- 100))))) (* skoB (+ (* skoT (* skoT (* skoT (+ 157 (* skoT 30))))) (* skoB (+ (* skoT (* skoT (- 200))) (* skoB (+ (* skoA (- 100)) (* skoT (+ 157 (* skoT 30))))))))))) 0) (>  (+ skoA skoB) 0) (>  (+ (- 1) skoT) 0) (>  (+ (* skoA (- 1)) skoB) 0)))
(check-sat)