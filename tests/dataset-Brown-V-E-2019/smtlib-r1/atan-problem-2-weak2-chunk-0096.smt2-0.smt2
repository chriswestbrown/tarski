(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoB () Real)
(declare-fun skoA () Real)
(declare-fun skoT () Real)
(assert (and (> (* (- 1) skoB) 0) (>  (+ (* skoA (* skoB (* skoB (* skoB 100)))) (* skoT (+ (* skoA (* skoB (* skoB 157))) (* skoT (+ (+ (* skoB (* skoB 100)) (* skoA (+ (* skoB (* skoB 30)) (* skoA 100)))) (* skoT (+ (* skoA 157) (* skoT (+ 100 (* skoA 30)))))))))) 0) (>  skoT 0) (>  (+ skoB skoA) 0) (>  (+ (- 1) skoT) 0) (> (* (- 1) (+ (* skoB (- 1)) skoA)) 0)))
(check-sat)